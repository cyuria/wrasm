#pragma once

#include "elf/def.h"
#include "elf/output.h"

struct symbol_t {
  size_t name_sz;
  char *name;
  enum sections_e section;
  long value;
  unsigned char binding;
  enum symbol_types_e {
    symbol_unknown,
    symbol_label,
    symbol_number,
    symbol_other,
  } type;
};

#define SYMBOLMAP_ENTRIES 256
struct symbolmap_t {
  int count;
  struct symbol_t *data;
};
extern struct symbolmap_t symbols[SYMBOLMAP_ENTRIES];

struct symbol_t *get_symbol(const char *);
struct symbol_t *create_symbol(const char *, enum symbol_types_e);

struct elf64sym_t create_symtab_entry(const char *);

size_t calc_symtab_str_buf_size(void);
char *create_symtab_str_buf(size_t);

void free_labels(void);
