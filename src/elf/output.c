
#include "elf/output.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "elf/def.h"
#include "symbols.h"
#include "xmalloc.h"

enum sectiontypes_e {
	SHT_NULL = 0x0,
	SHT_PROGBITS = 0x1,
	SHT_SYMTAB = 0x2,
	SHT_STRTAB = 0x3,
};

enum sections outputsection = SECTION_TEXT;
static struct section outputsections[SECTION_COUNT] = { { .size = 0,
							  .contents = NULL } };

static struct {
	uint64_t flags;
	uint32_t link;
	uint32_t info;
	uint64_t align;
	uint64_t entrysize;
	uint32_t type;
} sectiondata[SECTION_COUNT] = {
	{ 0x00, 0x0, 0x0, 0x1, 0x0, SHT_NULL },
	{ 0x00, 0x0, 0x0, 0x1, 0x0, SHT_STRTAB }, // .strtab
	{ 0x06, 0x0, 0x0, 0x4, 0x0, SHT_PROGBITS }, // .text
	{ 0x03, 0x0, 0x0, 0x1, 0x0, SHT_PROGBITS }, // .data
	{ 0x00, 0x1, 0x0, 0x8, 0x18, SHT_SYMTAB }, // .symtab
};

static const char *sectionnames[SECTION_COUNT] = {
	"", ".strtab", ".text", ".data", ".symtab",
};

void change_output(enum sections section)
{
	if (section >= SECTION_COUNT || section < 0)
		return;
	outputsection = section;
}

struct sectionpos get_outputpos(void)
{
	return (struct sectionpos){
		.section = outputsection,
		.offset = outputsections[outputsection].size
	};
}

void inc_outputsize(enum sections section, size_t amount)
{
	outputsections[section].size += amount;
}

void set_section(enum sections section)
{
	outputsection = section;
}

size_t calc_fileoffset(struct sectionpos a)
{
	return outputsections[a.section].offset + a.offset;
}

static inline size_t align_offset(size_t offset, size_t align)
{
	offset--;
	offset /= align;
	offset++;
	offset *= align;
	return offset;
}

void calc_strtab(void)
{
	outputsections[SECTION_STRTAB].size = 0;
	for (int i = 0; i < SECTION_COUNT; i++)
		outputsections[SECTION_STRTAB].size +=
			strlen(sectionnames[i]) + 1;
	outputsections[SECTION_STRTAB].size += calc_symtab_str_buf_size();
}

int fill_strtab(void)
{
	const size_t symtab_strings_sz = calc_symtab_str_buf_size();
	char *symtab_strings = create_symtab_str_buf(symtab_strings_sz);
	const size_t count =
		write_sectiondata(symtab_strings, symtab_strings_sz,
				  (struct sectionpos){
					  .section = SECTION_STRTAB,
					  .offset = 0,
				  });
	free(symtab_strings);
	if (count != symtab_strings_sz) {
		logger(ERROR, error_internal,
		       "Unable to write data to memory for section .strtab");
		return 1;
	}
	size_t offset = count;
	for (int i = 0; i < SECTION_COUNT; i++) {
		const size_t sz = strlen(sectionnames[i]) + 1;
		const size_t written =
			write_sectiondata(sectionnames[i], sz,
					  (struct sectionpos){
						  .section = SECTION_STRTAB,
						  .offset = offset,
					  });
		if (sz != written) {
			logger(ERROR, error_internal,
			       "Unable to write data to memory for section .strtab");
			return 1;
		}
		outputsections[i].nameoffset = (uint32_t)offset;
		offset += sz;
	}
	return 0;
}

void calc_symtab(void)
{
	size_t sz = 0;
	for (size_t hash = 0; hash < SYMBOLMAP_ENTRIES; hash++)
		sz += symbols[hash].count;
	outputsections[SECTION_SYMTAB].size =
		(sz + 1) * sizeof(struct elf64sym);
	sectiondata[SECTION_SYMTAB].info = (uint32_t)sz;
}

int fill_symtab(void)
{
	const struct elf64sym blank = (struct elf64sym){ 0, 0, 0, 0, 0, 0 };
	write_sectiondata(&blank, sizeof(blank),
			  (struct sectionpos){ .section = SECTION_SYMTAB,
					       .offset = 0 });
	uint32_t strtab_addr = 1;
	size_t count = 1;
	for (size_t hash = 0; hash < SYMBOLMAP_ENTRIES; hash++) {
		for (size_t index = 0; index < symbols[hash].count; index++) {
			const struct symbol *sym = &symbols[hash].data[index];
			struct elf64sym entry = (struct elf64sym){
				.name = strtab_addr,
				.info = sym->binding,
				.other = 0, /* TODO: add other attributes */
				.shndx = (uint16_t)sym->section,
				.value = (uint64_t)sym->value,
				.size = 0, /* TODO: support for symbol sizes? */
			};
			write_sectiondata(
				&entry, sizeof(entry),
				(struct sectionpos){ .section = SECTION_SYMTAB,
						     .offset = count *
							       sizeof(entry) });
			strtab_addr += (uint32_t)sym->name_sz;
			count++;
		}
	}
	return 0;
}

int alloc_output(void)
{
	size_t offset = sizeof(struct elf64header);
	for (int i = 0; i < SECTION_COUNT; i++) {
		outputsections[i].contents = xmalloc(outputsections[i].size);
		logger(DEBUG, no_error, "%d bytes allocated to section (%p)",
		       outputsections[i].size, outputsections[i].contents);
		offset = align_offset(offset, sectiondata[i].align);
		outputsections[i].offset = offset;
		offset += outputsections[i].size;
	}
	outputsections[SECTION_NULL].offset = 0x0;
	return 0;
}

size_t write_sectiondata(const void *bytes, size_t count,
			 struct sectionpos position)
{
	logger(DEBUG, no_error, "writing %d bytes to section %s",
	       position.section, sectionnames[position.section]);
	if (position.offset + count > outputsections[position.section].size) {
		logger(CRITICAL, error_internal,
		       "Too many bytes for allowed size (requested end: %d, allocated: %d)",
		       position.offset + count,
		       outputsections[position.section].size);
		return 0;
	}
	char *dest =
		outputsections[position.section].contents + position.offset;
	memcpy(dest, bytes, count);
	return count;
}

int flush_output(FILE *elf)
{
	logger(DEBUG, no_error, "Writing ELF output to temporary file");
	/* Generate headers */
	struct elf64header elfheader = new_elf64header();
	elfheader.phoffset = 0;
	elfheader.phentrysize = 0;
	elfheader.phcount = 0;

	elfheader.shcount = SECTION_COUNT;
	elfheader.shentrysize = sizeof(struct elf64sectionheader);
	elfheader.shstrindex = SECTION_STRTAB;

	struct elf64sectionheader sectionheaders[SECTION_COUNT];
	for (int i = 0; i < SECTION_COUNT; i++) {
		sectionheaders[i] = new_elf64sectionheader();
		sectionheaders[i].flags = sectiondata[i].flags;
		sectionheaders[i].link = sectiondata[i].link;
		sectionheaders[i].info = sectiondata[i].info;
		sectionheaders[i].addralign = sectiondata[i].align;
		sectionheaders[i].entrysize = sectiondata[i].entrysize;
		sectionheaders[i].type = sectiondata[i].type;
		sectionheaders[i].name = outputsections[i].nameoffset;
		sectionheaders[i].offset = outputsections[i].offset;
		sectionheaders[i].size = outputsections[i].size;
		logger(DEBUG, no_error,
		       "Creating section (%s) of size (0x%.08x) and offset (0x%.08x)",
		       sectionnames[i], sectionheaders[i].size,
		       sectionheaders[i].offset);
	}

	/* Fix offset and alignment stuff */
	sectionheaders[SECTION_NULL].addralign = 0x0;
	elfheader.shoffset =
		align_offset(outputsections[SECTION_COUNT - 1].offset +
				     outputsections[SECTION_COUNT - 1].size,
			     8);

	logger(DEBUG, no_error, "Section Header offset at 0x%.08x",
	       elfheader.shoffset);

	/* Write data to output */
	logger(DEBUG, no_error, "Writing ELF header");
	fwrite(&elfheader, sizeof(elfheader), 1, elf);
	for (int i = 0; i < SECTION_COUNT; i++) {
		logger(DEBUG, no_error, "Writing Section (%s)",
		       sectionnames[i]);
		fseek(elf, (long)outputsections[i].offset, SEEK_SET);
		fwrite(outputsections[i].contents, 1, outputsections[i].size,
		       elf);
	}
	logger(DEBUG, no_error, "Writing section headers");
	fseek(elf, (long)elfheader.shoffset, SEEK_SET);
	fwrite(sectionheaders, sizeof(sectionheaders), 1, elf);

	return 0;
}

void free_output(void)
{
	for (int i = 0; i < SECTION_COUNT; i++)
		free(outputsections[i].contents);
}
