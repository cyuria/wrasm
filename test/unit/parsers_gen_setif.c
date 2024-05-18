
#include <stdint.h>
#include "asm.h"
#include "debug.h"
#include "macros.h"
#include "parsers.h"
/* TODO: add tests for math shortcut parsers */

struct {
	const char *instruction;
	struct parser_t parser;
	uint32_t bytecode;
} cases[] = {
	{ .instruction = "seqz", .bytecode = 0x00113093 }, // seqz x1, x2
	{ .instruction = "snez", .bytecode = 0x003030b3 }, // snez x1, x3
	{ .instruction = "sltz", .bytecode = 0x000220b3 }, // sltz x1, x4
	{ .instruction = "sgtz", .bytecode = 0x005020b3 }, // sgtz x1, x5
};

int main(void)
{
	set_exit_loglevel(NODEBUG);
	for (size_t i = 0; i < ARRAY_LENGTH(cases); i++) {
		if (parse_parser(cases[i].instruction, &cases[i].parser)) {
			logger(ERROR, error_internal,
			       "Unable to find parser for instruction %s",
			       cases[i].instruction);
			return 1;
		}
		const struct args_t args = {
			.type = { arg_register, arg_register, arg_none },
			.arg = { 1, i + 2, 0 },
		};
		struct bytecode_t result =
			cases[i].parser.handler(cases[i].parser, args, 0);
		if (result.size != sizeof(cases[i].bytecode)) {
			logger(ERROR, error_internal,
			       "invalid size generated for %s",
			       cases[i].instruction);
			return 1;
		}
		if (*(uint32_t *)result.data != cases[i].bytecode) {
			logger(ERROR, error_internal,
			       "Expected %.08x but got %.08x while generating %s instruction",
			       cases[i].bytecode, *(uint32_t *)result.data,
			       cases[i].instruction);
			return 1;
		}
	}
	return 0;
}
