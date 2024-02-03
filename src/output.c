
#include "output.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "elfdef.h"

enum sectiontypes_e {
  sht_null = 0x0,
  sht_progbits = 0x1,
  sht_symtab = 0x2,
  sht_strtab = 0x3,
  sht_rela = 0x4,
  sht_nobits = 0x8,
  sht_riscv_attribute = 0x70000003,
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
} sectiondata[] = {{0x00, 0x0, 0x0, 0x1, 0x0, sht_null},
                   {0x06, 0x0, 0x0, 0x4, 0x0, sht_progbits},
                   {0x40, 0x7, 0x1, 0x8, 0x18, sht_rela},
                   {0x03, 0x0, 0x0, 0x1, 0x0, sht_progbits},
                   {0x03, 0x0, 0x0, 0x1, 0x0, sht_nobits},
                   {0x02, 0x0, 0x0, 0x1, 0x0, sht_progbits},
                   {0x00, 0x0, 0x0, 0x1, 0x0, sht_riscv_attribute},
                   {0x00, 0x8, 0x11, 0x8, 0x18, sht_symtab},
                   {0x00, 0x0, 0x0, 0x1, 0x0, sht_strtab},
                   {0x00, 0x0, 0x0, 0x1, 0x0, sht_strtab}};


void change_output(enum sections_e section) { outputsection = section; }

struct sectionpos_t get_outputpos(void) {
  return (struct sectionpos_t){.section = outputsection,
                               .offset = outputsections[outputsection].size};
}

void inc_outputsize(enum sections_e section, long amount) {
  outputsections[section].size += amount;
}

void set_outputpos(struct sectionpos_t pos) {
  change_output(pos.section);
}

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

int alloc_output(void) {
  size_t offset = sizeof(struct elf64header_t);
  for (int i = 0; i < SECTION_COUNT; i++) {
    outputsections[i].contents = malloc(outputsections[i].size);
    if (outputsections[i].contents == NULL) {
      logger(ERROR, error_internal, "Unable to malloc space for elf section");
      return 1;
    }
    offset = align_offset(offset, sectiondata[i].align);
    outputsections[i].offset = offset;
    offset += outputsections[i].size;
  }
  outputsections[section_null].offset = 0x0;
  return 0;
}

size_t write_sectiondata(const char *bytes, size_t count,
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
  /* Generate headers */
  struct elf64header_t elfheader = new_elf64header();
  elfheader.phoffset = 0;
  elfheader.phentrysize = 0;
  elfheader.phcount = 0;

  elfheader.shcount = SECTION_COUNT;

  struct elf64sectionheader_t sectionheaders[SECTION_COUNT];
  for (int i = 0; i < SECTION_COUNT; i++) {
    sectionheaders[i] = new_elf64sectionheader();
    sectionheaders[i].flags = sectiondata[i].flags;
    sectionheaders[i].link = sectiondata[i].link;
    sectionheaders[i].info = sectiondata[i].info;
    sectionheaders[i].addralign = sectiondata[i].align;
    sectionheaders[i].entrysize = sectiondata[i].entrysize;
    sectionheaders[i].type = sectiondata[i].type;
    sectionheaders[i].offset = outputsections[i].offset;
    sectionheaders[i].size = outputsections[i].size;
  }

  /* Fix offset and alignment stuff */
  sectionheaders[section_null].addralign = 0x0;
  elfheader.shoffset = align_offset(outputsections[SECTION_COUNT - 1].offset +
                                        outputsections[SECTION_COUNT - 1].size,
                                    8);

  /* Write data to output */
  fwrite(&elfheader, sizeof(elfheader), 1, elf);
  for (int i = 0; i < SECTION_COUNT; i++) {
    fseek(elf, outputsections[i].offset, SEEK_SET);
    fwrite(outputsections[i].contents, 1, outputsections[i].size, elf);
  }
  fwrite(sectionheaders, sizeof(sectionheaders), 1, elf);

  return 0;
}
