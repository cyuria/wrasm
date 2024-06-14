
#include <stdint.h>
#include <string.h>

#include "debug.h"
#include "elf/output.h"
#include "form/instructions.h"
#include "form/generic.h"
#include "macros.h"
#include "symbols.h"
#include "xmalloc.h"

struct case_t {
	const char *asm;
	uint32_t bytecode;
	size_t p;
};

struct case_t cases[] = {
	{ .asm = "csrrw gp, frm, t1", .bytecode = 0x002311f3 },
	{ .asm = "csrrs tp, menvcfg, t2", .bytecode = 0x30a3a273 },
	{ .asm = "csrrc t0, sstateen3, s0", .bytecode = 0x10f432f3 },
	{ .asm = "csrrwi s1, misa, 0x5", .bytecode = 0x3012d4f3 },
	{ .asm = "csrrsi a0, pmpaddr63, 0x2", .bytecode = 0x3ef16573 },
	{ .asm = "csrrci a1, tselect, 0x12", .bytecode = 0x7a0975f3 },
};

int test_case(struct case_t c)
{
	const size_t line_sz = strlen(c.asm) + 1;
	char *line = xmalloc(line_sz);
	memcpy(line, c.asm, line_sz);

	char *instruction = strtok(line, " \t");
	char *argstr = strtok(NULL, "");

	const struct formation formation = parse_form(instruction);
	if (!formation.name) {
		logger(ERROR, error_internal,
		       "Unable to find formation for instruction %s in %s",
		       instruction, c.asm);
		return 1;
	}

	struct args args = formation.arg_handler(argstr);

	struct bytecode result = formation.form_handler(
		formation.name, formation.idata, args, c.p);

	if (result.size != sizeof(c.bytecode)) {
		logger(ERROR, error_internal, "invalid size generated for %s",
		       c.asm);
		return 1;
	}
	if (*(uint32_t *)result.data != c.bytecode) {
		logger(ERROR, error_internal,
		       "Expected %.08x but got %.08x while generating %s",
		       c.bytecode, *(uint32_t *)result.data, c.asm);
		return 1;
	}

	return 0;
}

int main(void)
{
	set_exit_loglevel(NODEBUG);
	set_min_loglevel(DEBUG);

	struct symbol *start = create_symbol("_start", SYMBOL_LABEL);
	start->section = SECTION_NULL;
	start->value = 0;

	int errors = 0;

	for (size_t i = 0; i < ARRAY_LENGTH(cases); i++)
		errors += test_case(cases[i]);

	return errors != 0 || get_clean_exit(ERROR);
}
