#pragma once

#include "elf/def.h"
#include "elf/output.h"

struct symbol_t {
  char *name;
  enum sections_e section;
  long value;
  enum symbol_types_e {
    symbol_unknown,
    symbol_label,
    symbol_number,
    symbol_other,
  } type;
};

struct symbol_t *get_symbol(const char *);
struct symbol_t *create_symbol(const char *);

struct elf64sym_t create_symtab_entry(const char *);

void free_labels(void);
