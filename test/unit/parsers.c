
#include <stdint.h>
#include "asm.h"
#include "debug.h"
#include "macros.h"
#include "parsers.h"
#include "symbols.h"

struct case_t {
	const char *instruction;
	struct parser_t parser;
	uint32_t bytecode;
};
struct case_t cases_math[] = {
	{ .instruction = "mv", .bytecode = 0x00010093 },
	{ .instruction = "not", .bytecode = 0xfff1c093 },
	{ .instruction = "neg", .bytecode = 0x404000b3 },
	{ .instruction = "negw", .bytecode = 0x405000bb },
	{ .instruction = "sext.w", .bytecode = 0x0003009b },
};
struct case_t cases_setif[] = {
	{ .instruction = "seqz", .bytecode = 0x00113093 },
	{ .instruction = "snez", .bytecode = 0x003030b3 },
	{ .instruction = "sltz", .bytecode = 0x000220b3 },
	{ .instruction = "sgtz", .bytecode = 0x005020b3 },
};
struct case_t cases_branchifz[] = {
	{ .instruction = "beqz", .bytecode = 0x00008063 },
	{ .instruction = "bnez", .bytecode = 0xfe011ee3 },
	{ .instruction = "blez", .bytecode = 0xfe305ce3 },
	{ .instruction = "bgez", .bytecode = 0xfe025ae3 },
	{ .instruction = "bltz", .bytecode = 0xfe02c8e3 },
	{ .instruction = "bgtz", .bytecode = 0xfe6046e3 },
};
struct case_t cases_branchifr[] = {
	{ .instruction = "bgt", .bytecode = 0x0012c063 },
	{ .instruction = "ble", .bytecode = 0xfe235ee3 },
	{ .instruction = "bgtu", .bytecode = 0xfe33ece3 },
	{ .instruction = "bleu", .bytecode = 0xfe447ae3 },
};

int test_case(struct case_t c, struct args_t args, size_t position)
{
	if (parse_parser(c.instruction, &c.parser)) {
		logger(ERROR, error_internal,
		       "Unable to find parser for instruction %s",
		       c.instruction);
		return 1;
	}
	struct bytecode_t result = c.parser.handler(c.parser, args, position);
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
	struct symbol_t start = {
		.section = section_null,
		.value = 0,
		.type = symbol_label,
	};
	for (size_t i = 0; i < ARRAY_LENGTH(cases_math); i++)
		errors += test_case(cases_math[i],
				    (struct args_t){
					    .type = { arg_register,
						      arg_register, arg_none },
					    .arg = { 1, i + 2, 0 },
				    },
				    0);
	for (size_t i = 0; i < ARRAY_LENGTH(cases_setif); i++)
		errors += test_case(cases_setif[i],
				    (struct args_t){
					    .type = { arg_register,
						      arg_register, arg_none },
					    .arg = { 1, i + 2, 0 },
				    },
				    0);
	for (size_t i = 0; i < ARRAY_LENGTH(cases_branchifz); i++)
		errors += test_case(
			cases_branchifz[i],
			(struct args_t){
				.type = { arg_register, arg_symbol, arg_none },
				.arg = { i + 1, (size_t)&start, 0 },
			},
			i * RV64I_SIZE);
	for (size_t i = 0; i < ARRAY_LENGTH(cases_branchifr); i++)
		errors += test_case(
			cases_branchifr[i],
			(struct args_t){
				.type = { arg_register, arg_register,
					  arg_symbol },
				.arg = { i + 1, i + 5, (size_t)&start },
			},
			i * RV64I_SIZE);
	return errors != 0;
}
