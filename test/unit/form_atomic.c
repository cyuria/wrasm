
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
	{ .asm = "lr.w t0, a0", .bytecode = 0x8330000f },
	{ .asm = "sc.w t0, a2, a0", .bytecode = 0x8330000f },
	{ .asm = "lr.w.aq t0, a0", .bytecode = 0x8330000f },
	{ .asm = "sc.w.aq t0, a2, a0", .bytecode = 0x8330000f },
	{ .asm = "lr.w.rl t0, a0", .bytecode = 0x8330000f },
	{ .asm = "sc.w.rl t0, a2, a0", .bytecode = 0x8330000f },
	{ .asm = "lr.w.aq.rl t0, a0", .bytecode = 0x8330000f },
	{ .asm = "sc.w.aq.rl t0, a2, a0", .bytecode = 0x8330000f },
	{ .asm = "lr.d t0, a0", .bytecode = 0x8330000f },
	{ .asm = "sc.d t0, a2, a0", .bytecode = 0x8330000f },
	{ .asm = "lr.d.aq t0, a0", .bytecode = 0x8330000f },
	{ .asm = "sc.d.aq t0, a2, a0", .bytecode = 0x8330000f },
	{ .asm = "lr.d.rl t0, a0", .bytecode = 0x8330000f },
	{ .asm = "sc.d.rl t0, a2, a0", .bytecode = 0x8330000f },
	{ .asm = "lr.d.aq.rl t0, a0", .bytecode = 0x8330000f },
	{ .asm = "sc.d.aq.rl t0, a2, a0", .bytecode = 0x8330000f },
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
