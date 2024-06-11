#pragma once

#include "elf/def.h"
#include "elf/output.h"

struct symbol {
	size_t name_sz;
	char *name;
	enum sections section;
	long value;
	unsigned char binding;
	enum symbol_types {
		SYMBOL_UNKNOWN,
		SYMBOL_LABEL,
		SYMBOL_VALUE,
		SYMBOL_OTHER,
	} type;
};

#define SYMBOLMAP_ENTRIES 256
struct symbolmap {
	size_t count;
	struct symbol *data;
};
extern struct symbolmap symbols[SYMBOLMAP_ENTRIES];

struct symbol *get_symbol(const char *);
struct symbol *create_symbol(const char *, enum symbol_types);
struct symbol *get_or_create_symbol(const char *, enum symbol_types);

struct elf64sym create_symtab_entry(const char *);

size_t calc_symtab_str_buf_size(void);
char *create_symtab_str_buf(size_t);

void free_symbols(void);
