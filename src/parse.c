
#include "parse.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "bytecode.h"
#include "debug.h"
#include "elf/output.h"
#include "form/instructions.h"
#include "registers.h"
#include "stringutil.h"
#include "symbols.h"
#include "xmalloc.h"

const struct args_t empty_args = { .rd = 0,
				   .rs1 = 0,
				   .rs2 = 0,
				   .imm = 0,
				   .sym = NULL };

static char *trim_arg(char *s)
{
	const char *tok = strtok(s, ",");
	if (!tok)
		return NULL;
	return trim_whitespace(tok);
}

static uint8_t expect_reg(char *arg)
{
	size_t reg = get_register_id(arg);
	if (reg == (size_t)-1)
		logger(ERROR, error_instruction_other,
		       "Expected register but got %s", arg);
	return (uint8_t)reg;
}

static void expect_offreg(char *arg, int32_t *offset, uint8_t *reg)
{
	*offset = (int32_t)strtol(arg, &arg, 0);

	while (isspace(*arg))
		arg++;

	if (*arg != '(')
		logger(ERROR, error_instruction_other,
		       "Expected '(' but got '%c'", *arg);
	arg++;
	char *closing = arg;
	while (*closing != ')') {
		if (!*closing) {
			logger(ERROR, error_instruction_other,
			       "Expected closing parenthesis");
			return;
		}
		closing++;
	}
	*closing = '\0';

	size_t r = get_register_id(arg);

	if (r == (size_t)-1)
		logger(ERROR, error_instruction_other,
		       "Expected register but got %s", arg);

	*reg = (uint8_t)r;

	closing++;
	if (*closing)
		logger(ERROR, error_instruction_other,
		       "Received unexpected expression \"%s\"", closing);
}

static uint32_t expect_imm(char *arg)
{
	size_t imm;
	if (get_immediate(arg, &imm))
		logger(ERROR, error_instruction_other,
		       "Expected immediate but got %s", arg);
	return (uint32_t)imm;
}

static struct symbol_t *expect_sym(char *arg)
{
	struct symbol_t *sym = get_symbol(arg);
	if (!sym)
		logger(ERROR, error_instruction_other,
		       "Expected symbol but got %s", arg);
	return sym;
}

static int expect_one_arg(char *first)
{
	if (strtok(NULL, ",")) {
		logger(ERROR, error_instruction_other,
		       "Instruction has more than one argument");
		return 0;
	}

	if (!first) {
		logger(ERROR, error_instruction_other,
		       "Expected one argument, not none");
		return 1;
	}
	return 0;
}

static int expect_two_args(char *first, char *second)
{
	if (strtok(NULL, ",")) {
		logger(ERROR, error_instruction_other,
		       "Instruction has more than two arguments");
		return 0;
	}

	if (!first) {
		logger(ERROR, error_instruction_other,
		       "Expected two arguments, not none");
		return 1;
	}
	if (!second) {
		logger(ERROR, error_instruction_other,
		       "Expected two arguments, not one");
		free(first);
		return 1;
	}
	return 0;
}

static int expect_three_args(char *first, char *second, char *third)
{
	if (strtok(NULL, ",")) {
		logger(ERROR, error_instruction_other,
		       "Instruction has more than three arguments");
		return 0;
	}

	if (!first) {
		logger(ERROR, error_instruction_other,
		       "Expected three arguments, not none");
		return 1;
	}
	if (!second) {
		logger(ERROR, error_instruction_other,
		       "Expected three arguments, not one");
		free(first);
		return 1;
	}
	if (!third) {
		logger(ERROR, error_instruction_other,
		       "Expected three arguments, not two");
		free(first);
		free(second);
		return 1;
	}
	return 0;
}

int parse_asm(const char *linestr, struct sectionpos_t position)
{
	logger(DEBUG, no_error, "Parsing assembly %s", linestr);

	char *line = xmalloc(strlen(linestr) + 1);
	strcpy(line, linestr);

	char *instruction = strtok(line, " \t");
	char *argstr = strtok(NULL, "");

	struct formation_t formation = parse_form(instruction);
	struct args_t args = formation.arg_handler(argstr);

	free(line);

	add_instruction((struct instruction_t){
		.formation = formation,
		.args = args,
		.line = linenumber,
		.position = position,
	});
	inc_outputsize(position.section, formation.idata.sz);
	logger(DEBUG, no_error, "Updated position to offset (%zu)",
	       position.offset);

	return 0;
}

struct args_t parse_none(char *argstr)
{
	logger(DEBUG, no_error,
	       "Parsing arguments for no argument instruction");

	if (!argstr)
		return empty_args;

	for (char *c = argstr; *c; c++)
		if (*c == ',')
			logger(ERROR, error_instruction_other,
			       "Expected zero arguments, but got at least one");

	return empty_args;
}

struct args_t parse_rtype(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments for rtype instruction %s",
	       argstr);

	char *first = trim_arg(argstr);
	char *second = trim_arg(NULL);
	char *third = trim_arg(NULL);

	logger(DEBUG, no_error, "Parsed %s, %s, %s", first, second, third);

	if (expect_three_args(first, second, third))
		return empty_args;

	struct args_t args = {
		.rd = expect_reg(first),
		.rs1 = expect_reg(second),
		.rs2 = expect_reg(third),
	};

	free(first);
	free(second);
	free(third);

	logger(DEBUG, no_error, "Registers parsed x%d, x%d, x%d", args.rd,
	       args.rs1, args.rs2);

	return args;
}

struct args_t parse_itype(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments for itype instruction %s",
	       argstr);

	char *first = trim_arg(argstr);
	char *second = trim_arg(NULL);
	char *third = trim_arg(NULL);

	if (expect_three_args(first, second, third))
		return empty_args;

	struct args_t args = {
		.rd = expect_reg(first),
		.rs1 = expect_reg(second),
		.imm = expect_imm(third),
	};

	free(first);
	free(second);
	free(third);

	logger(DEBUG, no_error, "Registers parsed x%d, x%d, %d", args.rd,
	       args.rs1, args.imm);

	return args;
}

struct args_t parse_ltype(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments for itype instruction %s",
	       argstr);

	char *first = trim_arg(argstr);
	char *second = trim_arg(NULL);

	if (expect_two_args(first, second))
		return empty_args;

	struct args_t args = {
		.rd = expect_reg(first),
	};

	expect_offreg(second, &args.imm, &args.rs1);

	free(first);
	free(second);

	logger(DEBUG, no_error, "Registers parsed x%d, %d(x%d)", args.rd,
	       args.imm, args.rs1);

	return args;
}

struct args_t parse_stype(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments for itype instruction %s",
	       argstr);

	char *first = trim_arg(argstr);
	char *second = trim_arg(NULL);

	if (expect_two_args(first, second))
		return empty_args;

	struct args_t args = {
		.rs2 = expect_reg(second),
	};

	expect_offreg(first, &args.imm, &args.rs1);

	free(first);
	free(second);

	logger(DEBUG, no_error, "Registers parsed %d(x%d), x%d", args.imm,
	       args.rs1, args.rs2);

	free(first);
	free(second);

	logger(DEBUG, no_error, "Registers parsed x%d, x%d, %d", args.rs1,
	       args.rs2, args.imm);

	free(first);
	free(second);

	logger(DEBUG, no_error, "Registers parsed x%d, x%d, %d", args.rs1,
	       args.rs2, args.imm);

	return args;
}

struct args_t parse_utype(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments for utype instruction %s",
	       argstr);

	char *first = trim_arg(argstr);
	char *second = trim_arg(NULL);

	if (expect_two_args(first, second))
		return empty_args;

	struct args_t args = {
		.rd = expect_reg(first),
		.imm = expect_imm(second),
	};

	free(first);
	free(second);

	logger(DEBUG, no_error, "Registers parsed x%d, %d", args.rd, args.imm);

	return args;
}

struct args_t parse_btype(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments for jtype instruction %s",
	       argstr);

	char *first = trim_arg(argstr);
	char *second = trim_arg(NULL);
	char *third = trim_arg(NULL);

	if (expect_three_args(first, second, third))
		return empty_args;

	struct args_t args = {
		.rs1 = expect_reg(first),
		.rs2 = expect_reg(second),
		.sym = expect_sym(third),
	};

	free(first);
	free(second);
	free(third);

	logger(DEBUG, no_error, "Registers parsed x%d, x%d, %s", args.rs1,
	       args.rs2, args.sym->name);

	return args;
}

struct args_t parse_bztype(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments for bztype instruction %s",
	       argstr);

	char *first = trim_arg(argstr);
	char *second = trim_arg(NULL);

	if (expect_two_args(first, second))
		return empty_args;

	struct args_t args = {
		.rs1 = expect_reg(first),
		.sym = expect_sym(second),
	};

	free(first);
	free(second);

	logger(DEBUG, no_error, "Registers parsed x%d, %s", args.rs1,
	       args.sym->name);

	return args;
}

struct args_t parse_pseudo(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments %s for pseudo instruction",
	       argstr);

	char *first = trim_arg(argstr);
	char *second = trim_arg(NULL);

	if (expect_two_args(first, second))
		return empty_args;

	struct args_t args = {
		.rd = expect_reg(first),
		.rs1 = expect_reg(second),
	};

	free(first);
	free(second);

	logger(DEBUG, no_error, "Registers parsed x%d, x%d", args.rd, args.rs1);

	return args;
}

static int parse_fence_arg(const char *arg)
{
	const char *key = "iorw";
	int iorw = 0;
	for (const char *c = key; *c; c++) {
		iorw <<= 1;
		if (*c != *arg)
			continue;
		iorw |= 1;
		arg++;
	}
	if (*arg)
		logger(ERROR, error_instruction_other,
		       "expected combination of iorw but encountered '%c' character",
		       *arg);
	return iorw;
}

struct args_t parse_fence(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments %s for fence instruction",
	       argstr);

	char *first = trim_arg(argstr);
	char *second = trim_arg(NULL);

	if (!first)
		return (struct args_t){
			.rd = 0x0,
			.rs1 = 0x0,
			.imm = 0xFF,
		};

	if (strtok(NULL, ","))
		logger(ERROR, error_instruction_other,
		       "Instruction has more than two arguments");

	if (!second) {
		logger(ERROR, error_instruction_other,
		       "Expected two arguments but got one");
		return empty_args;
	}

	const int predecessor = parse_fence_arg(first);
	const int successor = parse_fence_arg(second);

	free(first);
	free(second);

	return (struct args_t){
		.rd = 0x0,
		.rs1 = 0x0,
		.imm = (predecessor << 4) | successor,
	};
}

struct args_t parse_jal(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments for jal instruction %s",
	       argstr);

	struct args_t args = { .rd = 1 };

	char *first = trim_arg(argstr);
	char *second = trim_arg(NULL);
	char *sym = first;

	if (strtok(NULL, ","))
		logger(ERROR, error_instruction_other,
		       "Expected at most two arguments");

	if (!first) {
		logger(ERROR, error_invalid_instruction,
		       "Expected at least one argument");
		return empty_args;
	}

	if (second) {
		args.rd = expect_reg(first);
		free(first);
		sym = second;
		if (!sym)
			logger(ERROR, error_invalid_instruction,
			       "Expected a second argument");
	}

	args.sym = expect_sym(sym);
	free(sym);

	logger(DEBUG, no_error, "Registers parsed, x%d, %s", args.rd,
	       args.sym->name);

	return args;
}

struct args_t parse_jalr(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments for jalr instruction %s",
	       argstr);

	char *first = trim_arg(argstr);
	char *second = trim_arg(NULL);
	char *third = trim_arg(NULL);

	if (strtok(NULL, ","))
		logger(ERROR, error_instruction_other,
		       "Instruction has more than three arguments");

	if (!first) {
		logger(DEBUG, error_instruction_other,
		       "Expected at least one argument");
		return empty_args;
	}

	const uint8_t rd = expect_reg(first);
	free(first);

	if (!second) {
		logger(DEBUG, no_error, "jalr (pseudo) arguments parsed -> x%d",
		       rd);
		return (struct args_t){
			.rd = 0x1,
			.rs1 = rd,
			.imm = 0,
		};
	}

	if (!third) {
		logger(ERROR, error_instruction_other,
		       "Expected one or three arguments but got two");
		free(second);
		return empty_args;
	}

	const uint8_t rs1 = expect_reg(second);
	const uint32_t imm = expect_imm(third);

	free(second);
	free(third);

	logger(DEBUG, no_error, "jalr arguments parsed -> x%d, x%d, %d", rd,
	       rs1, imm);

	return (struct args_t){ .rd = rd, .rs1 = rs1, .imm = imm };
}

struct args_t parse_la(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments %s for la instruction",
	       argstr);

	char *first = trim_arg(argstr);
	char *second = trim_arg(NULL);

	if (expect_two_args(first, second))
		return empty_args;

	struct args_t args = {
		.rd = expect_reg(first),
		.sym = expect_sym(second),
	};

	free(first);
	free(second);

	logger(DEBUG, no_error, "Registers parsed x%d, x%d", args.rd, args.rs1);

	return args;
}

struct args_t parse_li(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments %s for li instruction",
	       argstr);

	char *first = trim_arg(argstr);
	char *second = trim_arg(NULL);

	if (expect_two_args(first, second))
		return empty_args;

	struct args_t args = {
		.rd = expect_reg(first),
		.imm = expect_imm(second),
	};

	free(first);
	free(second);

	logger(DEBUG, no_error, "Registers parsed x%d, x%d", args.rd, args.rs1);

	return args;
}

struct args_t parse_j(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments %s for li instruction",
	       argstr);

	char *first = trim_arg(argstr);

	if (expect_one_arg(first))
		return empty_args;

	struct args_t args = {
		.sym = expect_sym(first),
	};

	free(first);

	logger(DEBUG, no_error, "Symbol parsed %s", args.sym->name);

	return args;
}

struct args_t parse_jr(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments %s for li instruction",
	       argstr);

	char *first = trim_arg(argstr);

	if (expect_one_arg(first))
		return empty_args;

	struct args_t args = {
		.rs1 = expect_reg(first),
	};

	free(first);

	logger(DEBUG, no_error, "Register parsed x%d", args.rs1);

	return args;
}

struct args_t parse_ftso(char *argstr)
{
	logger(DEBUG, no_error,
	       "Parsing arguments for no argument instruction");

	const struct args_t args = {
		.rd = 0x0,
		.rs1 = 0x0,
		.imm = 0x833,
	};

	if (!argstr)
		return args;

	for (char *c = argstr; *c; c++)
		if (*c == ',')
			logger(ERROR, error_instruction_other,
			       "Expected zero arguments, but got at least one");

	return args;
}

struct args_t parse_al(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments for itype instruction %s",
	       argstr);

	char *first = trim_arg(argstr);
	char *second = trim_arg(NULL);

	if (expect_two_args(first, second))
		return empty_args;

	struct args_t args = {
		.rd = expect_reg(first),
		.rs2 = 0,
	};

	expect_offreg(second, &args.imm, &args.rs1);

	if (args.imm)
		logger(ERROR, error_invalid_instruction,
		       "Optional integer offset must be zero");

	free(first);
	free(second);

	logger(DEBUG, no_error, "Registers parsed x%d %d(x%d)", args.rd,
	       args.imm, args.rs1);

	return args;
}

struct args_t parse_as(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments for itype instruction %s",
	       argstr);

	char *first = trim_arg(argstr);
	char *second = trim_arg(NULL);
	char *third = trim_arg(NULL);

	if (expect_three_args(first, second, third))
		return empty_args;

	struct args_t args = {
		.rd = expect_reg(first),
		.rs2 = expect_reg(second),
	};

	expect_offreg(third, &args.imm, &args.rs1);

	if (args.imm)
		logger(ERROR, error_invalid_instruction,
		       "Optional integer offset must be zero");

	free(first);
	free(second);
	free(third);

	logger(DEBUG, no_error, "Registers parsed x%d x%d %d(x%d)", args.rd,
	       args.rs2, args.imm, args.rs1);

	return args;
}
