
#include "form/generic.h"

#include <string.h>

#include "debug.h"
#include "form/rv64i.h"
#include "macros.h"
#include "symbols.h"

const struct bytecode_t error_bytecode = { .size = (size_t)-1, .data = NULL };

int32_t calc_relative_address(struct symbol_t *sym, size_t position)
{
	const size_t labelpos = calc_fileoffset((struct sectionpos_t){
		.section = sym->section, .offset = (size_t)sym->value });
	return (int32_t)(labelpos - position);
}

struct bytecode_t form_empty_bytecode(void)
{
	logger(DEBUG, no_error, "Generating empty bytecode");
	return (struct bytecode_t){ .size = 0, .data = NULL };
}
