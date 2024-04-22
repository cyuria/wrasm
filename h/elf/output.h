#pragma once

#include <stdint.h>
#include <stdio.h>

enum sections_e {
  section_null,
  section_strtab,
  section_text,
  section_data,
  section_symtab,
  SECTION_COUNT
};

struct sectionpos_t {
  enum sections_e section;
  size_t offset;
};

struct section_t {
  size_t offset;
  size_t size;
  char *contents;
  uint32_t nameoffset;
};

extern enum sections_e outputsection;

void change_output(enum sections_e);

struct sectionpos_t get_outputpos(void);
void inc_outputsize(enum sections_e, size_t);
void set_section(enum sections_e);

size_t calc_fileoffset(struct sectionpos_t);

void calc_strtab(void);
int fill_strtab(void);

void calc_symtab(void);
int fill_symtab(void);

int alloc_output(void);

size_t write_sectiondata(const void *, size_t, struct sectionpos_t);

int flush_output(FILE *);
