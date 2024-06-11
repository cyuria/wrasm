
#include "symbols.h"

#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "xmalloc.h"

struct symbolmap symbols[] = { { .count = 0, .data = NULL } };

static size_t hash_str(const char *str)
{
	size_t hash = 5381;

	while (*str) {
		hash = hash * 33 ^ (size_t)*str;
		str++;
	}

	return hash % SYMBOLMAP_ENTRIES;
}

struct symbol *get_symbol(const char *name)
{
	const size_t hash = hash_str(name);
	for (size_t i = 0; i < symbols[hash].count; i++) {
		if (!strcmp(name, symbols[hash].data[i].name)) {
			return &symbols[hash].data[i];
		}
	}
	return NULL;
}

struct symbol *get_or_create_symbol(const char *name, enum symbol_types type)
{
	struct symbol *sym = get_symbol(name);
	if (sym)
		return sym;
	return create_symbol(name, type);
}

struct symbol *create_symbol(const char *name, enum symbol_types type)
{
	const size_t hash = hash_str(name);

	const size_t index = symbols[hash].count;

	for (size_t i = 0; i < symbols[hash].count; i++) {
		if (!strcmp(name, symbols[hash].data[i].name)) {
			logger(ERROR, error_invalid_syntax,
			       "Duplicate symbol %s encountered");
			return NULL;
		}
	}

	symbols[hash].count++;
	symbols[hash].data =
		xrealloc(symbols[hash].data,
			 symbols[hash].count * sizeof(*symbols[hash].data));

	const size_t n_sz = strlen(name) + 1;
	char *n = xmalloc(n_sz);
	memcpy(n, name, n_sz);
	symbols[hash].data[index].name = n;
	symbols[hash].data[index].name_sz = n_sz;
	symbols[hash].data[index].type = type;
	symbols[hash].data[index].binding = 0;

	logger(DEBUG, no_error, "Created symbol named \"%s\"", n);

	return &symbols[hash].data[index];
}

size_t calc_symtab_str_buf_size(void)
{
	size_t sz = 1;
	for (size_t hash = 0; hash < SYMBOLMAP_ENTRIES; hash++)
		for (size_t index = 0; index < symbols[hash].count; index++)
			sz += symbols[hash].data[index].name_sz;
	return sz;
}

char *create_symtab_str_buf(size_t sz)
{
	char *buf = malloc(sz);
	buf[0] = '\0';
	size_t offset = 1;
	for (size_t hash = 0; hash < SYMBOLMAP_ENTRIES; hash++) {
		for (size_t index = 0; index < symbols[hash].count; index++) {
			const size_t name_sz =
				symbols[hash].data[index].name_sz;
			memcpy(buf + offset, symbols[hash].data[index].name,
			       name_sz);
			offset += name_sz;
		}
	}
	return buf;
}

void free_symbols(void)
{
	for (size_t hash = 0; hash < SYMBOLMAP_ENTRIES; hash++) {
		for (size_t index = 0; index < symbols[hash].count; index++)
			free(symbols[hash].data[index].name);
		free(symbols[hash].data);
	}
}
