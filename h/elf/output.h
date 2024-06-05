#pragma once

#include <stdint.h>
#include <stdio.h>

enum sections {
	SECTION_NULL,
	SECTION_STRTAB,
	SECTION_TEXT,
	SECTION_DATA,
	SECTION_SYMTAB,
	SECTION_COUNT
};

struct sectionpos {
	enum sections section;
	size_t offset;
};

struct section {
	size_t offset;
	size_t size;
	char *contents;
	uint32_t nameoffset;
};

extern enum sections outputsection;

void change_output(enum sections);

struct sectionpos get_outputpos(void);
void inc_outputsize(enum sections, size_t);
void set_section(enum sections);

size_t calc_fileoffset(struct sectionpos);

void calc_strtab(void);
int fill_strtab(void);

void calc_symtab(void);
int fill_symtab(void);

int alloc_output(void);

size_t write_sectiondata(const void *, size_t, struct sectionpos);

int flush_output(FILE *);
void free_output(void);
