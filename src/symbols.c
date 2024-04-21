
#include "symbols.h"

#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "elf/def.h"

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

static struct symbolmap_t {
  int count;
  struct symbol_t *data;
} symbols[256] = {{.count = 0, .data = NULL}};

static size_t hash_str(const char *str) {
  size_t hash = 5381;
  char c;

  while ((c = *str++))
    hash = hash * 33 ^ c;

  return hash % (sizeof(symbols) / sizeof(*symbols));
}

struct symbol_t *get_symbol(const char *name) {
  const size_t hash = hash_str(name);
  for (int i = 0; i < symbols[hash].count; i++)
    if (!strcmp(name, symbols[hash].data[i].name))
      return &symbols[hash].data[i];
  return NULL;
}

struct symbol_t *create_symbol(const char *name, enum symbol_types_e type) {
  const size_t hash = hash_str(name);
  const int index = symbols[hash].count;

  symbols[hash].count++;
  symbols[hash].data = realloc(
      symbols[hash].data, symbols[hash].count * sizeof(*symbols[hash].data));

  char *n = malloc(strlen(name) + 1);
  strcpy(n, name);
  symbols[hash].data[index].name = n;

  symbols[hash].data[index].type = type;

  logger(DEBUG, no_error, "Created symbol named \"%s\"", n);

  return &symbols[hash].data[index];
}

struct elf64sym_t create_symtab_entry(const char *name) {
  // TODO: implement symtab entry
  (void)name;
  return (struct elf64sym_t){.name = 0};
}

void free_labels(void) {
  for (size_t hash = 0; hash < sizeof(symbols) / sizeof(*symbols); hash++) {
    for (int index = 0; index < symbols[hash].count; index++)
      free(symbols[hash].data[index].name);
    free(symbols[hash].data);
  }
}
