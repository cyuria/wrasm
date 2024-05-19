
#include <stdint.h>
#include "asm.h"
#include "debug.h"
#include "macros.h"
#include "parsers.h"

struct case_t {
	const char *instruction;
	struct parser_t parser;
	uint32_t bytecode;
};
struct case_t cases_math[] = {
	{ .instruction = "mv", .bytecode = 0x00010093 }, // mv x1, x2
	{ .instruction = "not", .bytecode = 0xfff1c093 }, // not x1, x3
	{ .instruction = "neg", .bytecode = 0x404000b3 }, // neg x1, x4
	{ .instruction = "negw", .bytecode = 0x405000bb }, // negw x1, x5
	{ .instruction = "sext.w", .bytecode = 0x0003009b }, // sext.w x1, x6
};
struct case_t cases_setif[] = {
	{ .instruction = "seqz", .bytecode = 0x00113093 }, // seqz x1, x2
	{ .instruction = "snez", .bytecode = 0x003030b3 }, // snez x1, x3
	{ .instruction = "sltz", .bytecode = 0x000220b3 }, // sltz x1, x4
	{ .instruction = "sgtz", .bytecode = 0x005020b3 }, // sgtz x1, x5
};

int test_case(struct case_t c, int i)
{
	if (parse_parser(c.instruction, &c.parser)) {
		logger(ERROR, error_internal,
		       "Unable to find parser for instruction %s",
		       c.instruction);
		return 1;
	}
	const struct args_t args = {
		.type = { arg_register, arg_register, arg_none },
		.arg = { 1, i + 2, 0 },
	};
	struct bytecode_t result = c.parser.handler(c.parser, args, 0);
	if (result.size != sizeof(c.bytecode)) {
		logger(ERROR, error_internal, "invalid size generated for %s",
		       c.instruction);
		return 1;
	}
	if (*(uint32_t *)result.data != c.bytecode) {
		logger(ERROR, error_internal,
		       "Expected %.08x but got %.08x while generating %s instruction",
		       c.bytecode, *(uint32_t *)result.data, c.instruction);
		return 1;
	}
	return 0;
}

int main(void)
{
	set_exit_loglevel(NODEBUG);
	int errors = 0;
	for (size_t i = 0; i < ARRAY_LENGTH(cases_math); i++)
		errors += test_case(cases_math[i], i);
	for (size_t i = 0; i < ARRAY_LENGTH(cases_setif); i++)
		errors += test_case(cases_setif[i], i);
	return errors != 0;
}
