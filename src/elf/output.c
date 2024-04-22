
#include "elf/output.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "elf/def.h"
#include "symbols.h"
#include "xmalloc.h"

enum sectiontypes_e {
  sht_null = 0x0,
  sht_progbits = 0x1,
  sht_symtab = 0x2,
  sht_strtab = 0x3,
};

enum sections_e outputsection = section_text;
static struct section_t outputsections[SECTION_COUNT] = {
    {.size = 0, .contents = NULL}};

static struct {
  uint64_t flags;
  uint32_t link;
  uint32_t info;
  uint64_t align;
  uint64_t entrysize;
  uint32_t type;
} sectiondata[SECTION_COUNT] = {
    {0x00, 0x0, 0x0, 0x1, 0x0, sht_null},
    {0x00, 0x0, 0x0, 0x1, 0x0, sht_strtab},   // .strtab
    {0x06, 0x0, 0x0, 0x4, 0x0, sht_progbits}, // .text
    {0x03, 0x0, 0x0, 0x1, 0x0, sht_progbits}, // .data
    {0x00, 0x1, 0x3, 0x8, 0x18, sht_symtab},  // .symtab
};

static const char *sectionnames[SECTION_COUNT] = {
    "", ".strtab", ".text", ".data", ".symtab",
};

void change_output(enum sections_e section) {
  if (section >= SECTION_COUNT || section < 0)
    return;
  outputsection = section;
}

struct sectionpos_t get_outputpos(void) {
  return (struct sectionpos_t){.section = outputsection,
                               .offset = outputsections[outputsection].size};
}

void inc_outputsize(enum sections_e section, size_t amount) {
  outputsections[section].size += amount;
}

void set_section(enum sections_e section) { outputsection = section; }

size_t calc_fileoffset(struct sectionpos_t a) {
  return outputsections[a.section].offset + a.offset;
}

static inline size_t align_offset(size_t offset, size_t align) {
  offset--;
  offset /= align;
  offset++;
  offset *= align;
  return offset;
}

void calc_strtab(void) {
  outputsections[section_strtab].size = 0;
  for (int i = 0; i < SECTION_COUNT; i++)
    outputsections[section_strtab].size += strlen(sectionnames[i]) + 1;
  outputsections[section_strtab].size += calc_symtab_str_buf_size();
}

int fill_strtab(void) {
  const size_t symtab_strings_sz = calc_symtab_str_buf_size();
  char *symtab_strings = create_symtab_str_buf(symtab_strings_sz);
  const size_t count = write_sectiondata(symtab_strings, symtab_strings_sz,
                                         (struct sectionpos_t){
                                             .section = section_strtab,
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
    const size_t count = write_sectiondata(sectionnames[i], sz,
                                           (struct sectionpos_t){
                                               .section = section_strtab,
                                               .offset = offset,
                                           });
    if (sz != count) {
      logger(ERROR, error_internal,
             "Unable to write data to memory for section .strtab");
      return 1;
    }
    outputsections[i].nameoffset = (uint32_t)offset;
    offset += sz;
  }
  return 0;
}

void calc_symtab(void) {
  size_t sz = 1;
  for (size_t hash = 0; hash < SYMBOLMAP_ENTRIES; hash++)
    sz += symbols[hash].count;
  outputsections[section_symtab].size = sz * sizeof(struct elf64sym_t);
}

/* TODO: implement symbol table */
int fill_symtab(void) {
  const struct elf64sym_t blank = (struct elf64sym_t){0, 0, 0, 0, 0, 0};
  write_sectiondata(
      &blank, sizeof(blank),
      (struct sectionpos_t){.section = section_symtab, .offset = 0});
  uint32_t strtab_addr = 1;
  size_t count = 1;
  for (size_t hash = 0; hash < SYMBOLMAP_ENTRIES; hash++) {
    for (int index = 0; index < symbols[hash].count; index++) {
      const struct symbol_t *sym = &symbols[hash].data[index];
      struct elf64sym_t entry = (struct elf64sym_t){
          .name = strtab_addr,
          .info = sym->binding,
          .other = 0, /* TODO: add other visibility attributes */
          .shndx = sym->section,
          .value = sym->value,
          .size = 0, /* TODO: support for symbol sizes? */
      };
      write_sectiondata(
          &entry, sizeof(entry),
          (struct sectionpos_t){.section = section_symtab,
                                .offset = count * sizeof(struct elf64sym_t)});
      strtab_addr += sym->name_sz;
      count++;
    }
  }
  return 0;
}

int alloc_output(void) {
  size_t offset = sizeof(struct elf64header_t);
  for (int i = 0; i < SECTION_COUNT; i++) {
    outputsections[i].contents = xmalloc(outputsections[i].size);
    logger(DEBUG, no_error, "%d bytes allocated to section (%p)",
           outputsections[i].size, outputsections[i].contents);
    offset = align_offset(offset, sectiondata[i].align);
    outputsections[i].offset = offset;
    offset += outputsections[i].size;
  }
  outputsections[section_null].offset = 0x0;
  return 0;
}

size_t write_sectiondata(const void *bytes, size_t count,
                         struct sectionpos_t position) {
  if (position.offset + count > outputsections[position.section].size) {
    logger(CRITICAL, error_internal,
           "Too many bytes for allowed size (requested end: %d, allocated: %d)",
           position.offset + count, outputsections[position.section].size);
    return 0;
  }
  char *dest = outputsections[position.section].contents + position.offset;
  memcpy(dest, bytes, count);
  return count;
}

int flush_output(FILE *elf) {
  logger(DEBUG, no_error, "Writing ELF output to temporary file");
  /* Generate headers */
  struct elf64header_t elfheader = new_elf64header();
  elfheader.phoffset = 0;
  elfheader.phentrysize = 0;
  elfheader.phcount = 0;

  elfheader.shcount = SECTION_COUNT;
  elfheader.shentrysize = sizeof(struct elf64sectionheader_t);
  elfheader.shstrindex = section_strtab;

  struct elf64sectionheader_t sectionheaders[SECTION_COUNT];
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
           sectionnames[i], sectionheaders[i].size, sectionheaders[i].offset);
  }

  /* Fix offset and alignment stuff */
  sectionheaders[section_null].addralign = 0x0;
  elfheader.shoffset = align_offset(outputsections[SECTION_COUNT - 1].offset +
                                        outputsections[SECTION_COUNT - 1].size,
                                    8);

  logger(DEBUG, no_error, "Section Header offset at 0x%.08x",
         elfheader.shoffset);

  /* Write data to output */
  logger(DEBUG, no_error, "Writing ELF header");
  fwrite(&elfheader, sizeof(elfheader), 1, elf);
  for (int i = 0; i < SECTION_COUNT; i++) {
    logger(DEBUG, no_error, "Writing Section (%s)", sectionnames[i]);
    fseek(elf, (long)outputsections[i].offset, SEEK_SET);
    fwrite(outputsections[i].contents, 1, outputsections[i].size, elf);
  }
  logger(DEBUG, no_error, "Writing section headers");
  fseek(elf, (long)elfheader.shoffset, SEEK_SET);
  fwrite(sectionheaders, sizeof(sectionheaders), 1, elf);

  return 0;
}
