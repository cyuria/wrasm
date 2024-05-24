
#include "parse.h"

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

int parse_asm(const char *linestr, struct sectionpos_t position)
{
	logger(DEBUG, no_error, "Parsing assembly %s", linestr);

	char *line = xmalloc(strlen(linestr) + 1);
	strcpy(line, linestr);

	char *instruction = strtok(line, " \t");

	struct formation_t formation = parse_form(instruction);
	struct args_t args = formation.arg_handler(strtok(NULL, ""));

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

/*
int parse_args(char *argstr, struct args_t *args)
{
	// TODO: reimplement
	logger(DEBUG, no_error, "Parsing arguments for instruction (%s)",
	       argstr);

	args->a[0].type = arg_none;
	args->a[1].type = arg_none;
	args->a[2].type = arg_none;

	char **arg_strs = NULL;
	size_t arg_count = 0;

	char *raw = NULL;
	raw = strtok(argstr, ",");
	while (raw) {
		const size_t i = arg_count;
		arg_count++;
		if (arg_count > 3) {
			logger(ERROR, error_instruction_other,
			       "Instruction has too many arguments");
			return 1;
		}
		arg_strs = xrealloc(arg_strs, arg_count * sizeof(*arg_strs));
		arg_strs[i] = trim_whitespace(raw);
		raw = strtok(NULL, ",");
	}

	for (size_t i = 0; i < arg_count; i++)
		parse_arg(arg_strs[i], &args->a[i].type, &args->a[i].arg);

	logger(DEBUG, no_error,
	       "Arguments parsed, (%d, %d), (%d, %d), (%d, %d)",
	       args->a[0].type, args->a[0].arg, args->a[1].type, args->a[1].arg,
	       args->a[2].type, args->a[2].arg);

	return 0;
}

static int parse_arg(const char *str, enum argtype_e *type, size_t *arg)
{
	size_t reg = get_register_id(str);
	if (reg != (size_t)-1) {
		*type = arg_register;
		*arg = reg;
		return 0;
	}
	struct symbol_t *sym = get_symbol(str);
	if (sym) {
		*type = arg_symbol;
		*arg = (size_t)sym;
		return 0;
	}
	const int status = get_immediate(str, arg);
	if (!status) {
		*type = arg_immediate;
		return 0;
	}
	logger(ERROR, error_instruction_other,
	       "Uknown argument (%s) encountered", str);
	return 0;
}
*/

/*
	size_t reg = get_register_id(str);
	if (reg != (size_t)-1) {
		*type = arg_register;
		*arg = reg;
		return 0;
	}
	struct symbol_t *sym = get_symbol(str);
	if (sym) {
		*type = arg_symbol;
		*arg = (size_t)sym;
		return 0;
	}
	const int status = get_immediate(str, arg);
	if (!status) {
		*type = arg_immediate;
		return 0;
	}
	logger(ERROR, error_instruction_other,
	       "Uknown argument (%s) encountered", str);
	return 0;
*/

struct args_t parse_none(char *argstr)
{
	logger(DEBUG, no_error,
	       "Parsing arguments for no argument instruction");

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

	if (strtok(NULL, ","))
		logger(ERROR, error_instruction_other,
		       "Instruction has more than three arguments");

	if (!first) {
		logger(ERROR, error_instruction_other,
		       "Expected three arguments, not none");
		return empty_args;
	}
	if (!second) {
		logger(ERROR, error_instruction_other,
		       "Expected three arguments, not one");
		return empty_args;
	}
	if (!third) {
		logger(ERROR, error_instruction_other,
		       "Expected three arguments, not two");
		return empty_args;
	}

	struct args_t args = {
		.rd = expect_reg(first),
		.rs1 = expect_reg(second),
		.rs2 = expect_reg(third),
	};

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

	if (strtok(NULL, ","))
		logger(ERROR, error_instruction_other,
		       "Instruction has more than three arguments");

	if (!first) {
		logger(ERROR, error_instruction_other,
		       "Expected three arguments, not none");
		return empty_args;
	}
	if (!second) {
		logger(ERROR, error_instruction_other,
		       "Expected three arguments, not one");
		return empty_args;
	}
	if (!third) {
		logger(ERROR, error_instruction_other,
		       "Expected three arguments, not two");
		return empty_args;
	}

	struct args_t args = {
		.rd = expect_reg(first),
		.rs1 = expect_reg(second),
		.imm = expect_imm(third),
	};

	logger(DEBUG, no_error, "Registers parsed x%d, x%d, %d", args.rd,
	       args.rs1, args.imm);

	return args;
}

struct args_t parse_stype(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments for jtype instruction %s",
	       argstr);

	char *first = trim_arg(argstr);
	char *second = trim_arg(NULL);
	char *third = trim_arg(NULL);

	if (strtok(NULL, ","))
		logger(ERROR, error_instruction_other,
		       "Instruction has more than three arguments");

	if (!first) {
		logger(ERROR, error_instruction_other,
		       "Expected three arguments, not none");
		return empty_args;
	}
	if (!second) {
		logger(ERROR, error_instruction_other,
		       "Expected three arguments, not one");
		return empty_args;
	}
	if (!third) {
		logger(ERROR, error_instruction_other,
		       "Expected three arguments, not two");
		return empty_args;
	}

	struct args_t args = {
		.rs1 = expect_reg(first),
		.rs2 = expect_reg(second),
		.imm = expect_imm(third),
	};

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

	if (strtok(NULL, ","))
		logger(ERROR, error_instruction_other,
		       "Instruction has more than three arguments");

	if (!first) {
		logger(ERROR, error_instruction_other,
		       "Expected two arguments but got none");
		return empty_args;
	}
	if (!second) {
		logger(ERROR, error_instruction_other,
		       "Expected two arguments but got one");
		return empty_args;
	}

	struct args_t args = {
		.rd = expect_reg(first),
		.imm = expect_imm(second),
	};

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

	if (strtok(NULL, ","))
		logger(ERROR, error_instruction_other,
		       "Instruction has more than three arguments");

	if (!first) {
		logger(ERROR, error_instruction_other,
		       "Expected three arguments, not none");
		return empty_args;
	}
	if (!second) {
		logger(ERROR, error_instruction_other,
		       "Expected three arguments, not one");
		return empty_args;
	}
	if (!third) {
		logger(ERROR, error_instruction_other,
		       "Expected three arguments, not two");
		return empty_args;
	}

	struct args_t args = {
		.rs1 = expect_reg(first),
		.rs2 = expect_reg(second),
		.sym = expect_sym(third),
	};

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

	if (strtok(NULL, ","))
		logger(ERROR, error_instruction_other,
		       "Instruction has more than three arguments");

	if (!first) {
		logger(ERROR, error_instruction_other,
		       "Expected two arguments but got none");
		return empty_args;
	}
	if (!second) {
		logger(ERROR, error_instruction_other,
		       "Expected two arguments but got one");
		return empty_args;
	}

	struct args_t args = {
		.rs1 = expect_reg(first),
		.sym = expect_sym(second),
	};

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

	if (strtok(NULL, ","))
		logger(ERROR, error_instruction_other,
		       "Instruction has more than three arguments");

	if (!first) {
		logger(ERROR, error_instruction_other,
		       "Expected two arguments but got none");
		return empty_args;
	}
	if (!second) {
		logger(ERROR, error_instruction_other,
		       "Expected two arguments but got one");
		return empty_args;
	}

	struct args_t args = {
		.rd = expect_reg(first),
		.rs1 = expect_reg(second),
	};

	logger(DEBUG, no_error, "Registers parsed x%d, x%d", args.rd, args.rs1);

	return args;
}

struct args_t parse_fence(char *argstr)
{
	(void)argstr;
	logger(ERROR, error_not_implemented,
	       "The FENCE instruction has not been implemented yet");
	return empty_args;
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

	if (!first)
		logger(ERROR, error_invalid_instruction,
		       "Expected at least one argument");

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

	if (!first) {
		logger(DEBUG, error_instruction_other,
		       "Expected at least one argument");
		return empty_args;
	}

	uint8_t rd = expect_reg(first);

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
		return empty_args;
	}

	if (strtok(NULL, ","))
		logger(ERROR, error_instruction_other,
		       "Instruction has more than three arguments");

	const uint8_t rs1 = expect_reg(second);
	const uint32_t imm = expect_imm(third);

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

	if (strtok(NULL, ","))
		logger(ERROR, error_instruction_other,
		       "Instruction has more than three arguments");

	if (!first) {
		logger(ERROR, error_instruction_other,
		       "Expected two arguments but got none");
		return empty_args;
	}
	if (!second) {
		logger(ERROR, error_instruction_other,
		       "Expected two arguments but got one");
		return empty_args;
	}

	struct args_t args = {
		.rd = expect_reg(first),
		.sym = expect_sym(second),
	};

	logger(DEBUG, no_error, "Registers parsed x%d, x%d", args.rd, args.rs1);

	return args;
}
