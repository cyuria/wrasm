
#include "form/generic.h"

#include "debug.h"
#include "symbols.h"

const struct bytecode_t error_bytecode = { .size = (size_t)-1, .data = NULL };

int check_args(const char *name, struct args_t args, struct args_t expected)
{
	const int a = args.a[0].type == expected.a[0].type;
	const int b = args.a[1].type == expected.a[1].type;
	const int c = args.a[2].type == expected.a[2].type;
	if (a && b && c)
		return 0;
	logger(ERROR, error_invalid_syntax,
	       "Invalid arguments for instruction %s", name);
	return 1;
}

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
