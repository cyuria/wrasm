
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
	{ .asm = "mv x1, x2", .bytecode = 0x00010093 },
	{ .asm = "not x1, x3", .bytecode = 0xfff1c093 },
	{ .asm = "neg x1, x4", .bytecode = 0x404000b3 },
	{ .asm = "negw x1, x5", .bytecode = 0x405000bb },
	{ .asm = "sext.w x1, x6", .bytecode = 0x0003009b },

	{ .asm = "seqz x1, x2", .bytecode = 0x00113093 },
	{ .asm = "snez x1, x3", .bytecode = 0x003030b3 },
	{ .asm = "sltz x1, x4", .bytecode = 0x000220b3 },
	{ .asm = "sgtz x1, x5", .bytecode = 0x005020b3 },

	{ .asm = "beqz x1, _start", .p = 0x00, .bytecode = 0x00008063 },
	{ .asm = "bnez x2, _start", .p = 0x04, .bytecode = 0xfe011ee3 },
	{ .asm = "blez x3, _start", .p = 0x08, .bytecode = 0xfe305ce3 },
	{ .asm = "bgez x4, _start", .p = 0x0C, .bytecode = 0xfe025ae3 },
	{ .asm = "bltz x5, _start", .p = 0x10, .bytecode = 0xfe02c8e3 },
	{ .asm = "bgtz x6, _start", .p = 0x14, .bytecode = 0xfe6046e3 },

	{ .asm = "bgt x1, x5, _start", .p = 0x0, .bytecode = 0x0012c063 },
	{ .asm = "ble x2, x6, _start", .p = 0x4, .bytecode = 0xfe235ee3 },
	{ .asm = "bgtu x3, x7, _start", .p = 0x8, .bytecode = 0xfe33ece3 },
	{ .asm = "bleu x4, x8, _start", .p = 0xC, .bytecode = 0xfe447ae3 },

	{ .asm = "j _start", .p = 0x0, .bytecode = 0x0000006f },
	{ .asm = "jal x1, _start", .p = 0x4, .bytecode = 0xffdff0ef },

	{ .asm = "jr x2", .bytecode = 0x00010067 },
	{ .asm = "jalr x1, x3, 0", .bytecode = 0x000180e7 },

	{ .asm = "ret", .bytecode = 0x00008067 },

	{ .asm = "fence", .bytecode = 0x0ff0000f },
	{ .asm = "fence ir, ow", .bytecode = 0x0a50000f },
	{ .asm = "fence.tso", .bytecode = 0x8330000f },

	{ .asm = "lb x6, 12(sp)", .bytecode = 0x00c10303 },
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
