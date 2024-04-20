
#include "symbols.h"

#include <stdlib.h>
#include <string.h>

#include "debug.h"

static struct symbolmap_t {
  int count;
  struct symbol_t *data;
} symbols[256] = {{.count = 0, .data = NULL}};

static unsigned char hash_str(const char *str) {
  unsigned char hash = 5;
  char c;

  while ((c = *str++))
    hash = hash * 33 ^ c;

  return hash;
}

struct symbol_t *get_symbol(const char *name) {
  const unsigned char hash = hash_str(name);
  for (int i = 0; i < symbols[hash].count; i++)
    if (!strcmp(name, symbols[hash].data[i].name))
      return &symbols[hash].data[i];
  return NULL;
}

struct symbol_t *create_symbol(const char *name) {
  const unsigned char hash = hash_str(name);
  const int index = symbols[hash].count;

  symbols[hash].count++;
  symbols[hash].data = realloc(
      symbols[hash].data, symbols[hash].count * sizeof(*symbols[hash].data));

  char *n = malloc(strlen(name) + 1);
  strcpy(n, name);
  symbols[hash].data[index].name = n;

  logger(DEBUG, no_error, "Created symbol named \"%s\"", n);

  return &symbols[hash].data[index];
}

void free_labels(void) {
  for (unsigned char hash = 0;
       hash < (unsigned char)(sizeof(symbols) / sizeof(*symbols)); hash++) {
    for (int index = 0; index < symbols[hash].count; index++)
      free(symbols[hash].data[index].name);
    free(symbols[hash].data);
  }
}
