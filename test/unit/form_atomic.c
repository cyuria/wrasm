
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
	{ .asm = "lr.w t0, (a0)", .bytecode = 0x100522af },
	{ .asm = "sc.w t0, a2, (a0)", .bytecode = 0x18c522af },
	{ .asm = "lr.w.aq t0, (a0)", .bytecode = 0x140522af },
	{ .asm = "sc.w.aq t0, a2, (a0)", .bytecode = 0x1cc522af },
	{ .asm = "lr.w.rl t0, (a0)", .bytecode = 0x120522af },
	{ .asm = "sc.w.rl t0, a2, (a0)", .bytecode = 0x1ac522af },
	{ .asm = "lr.w.aqrl t0, (a0)", .bytecode = 0x160522af },
	{ .asm = "sc.w.aqrl t0, a2, (a0)", .bytecode = 0x1ec522af },
	{ .asm = "lr.d t0, (a0)", .bytecode = 0x100532af },
	{ .asm = "sc.d t0, a2, (a0)", .bytecode = 0x18c532af },
	{ .asm = "lr.d.aq t0, (a0)", .bytecode = 0x140532af },
	{ .asm = "sc.d.aq t0, a2, (a0)", .bytecode = 0x1cc532af },
	{ .asm = "lr.d.rl t0, (a0)", .bytecode = 0x120532af },
	{ .asm = "sc.d.rl t0, a2, (a0)", .bytecode = 0x1ac532af },
	{ .asm = "lr.d.aqrl t0, (a0)", .bytecode = 0x160532af },
	{ .asm = "sc.d.aqrl t0, a2, (a0)", .bytecode = 0x1ec532af },
};

int test_case(struct case_t c)
{
	const size_t line_sz = strlen(c.asm) + 1;
	char *line = xmalloc(line_sz);
	memcpy(line, c.asm, line_sz);

	char *instruction = strtok(line, " \t");
	char *argstr = strtok(NULL, "");

	const struct formation_t formation = parse_form(instruction);
	if (!formation.name) {
		logger(ERROR, error_internal,
		       "Unable to find formation for instruction %s in %s",
		       instruction, c.asm);
		return 1;
	}

	struct args_t args = formation.arg_handler(argstr);

	struct bytecode_t result = formation.form_handler(
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

	struct symbol_t *start = create_symbol("_start", symbol_label);
	start->section = section_null;
	start->value = 0;

	int errors = 0;

	for (size_t i = 0; i < ARRAY_LENGTH(cases); i++)
		errors += test_case(cases[i]);

	return errors != 0 || get_clean_exit(ERROR);
}
