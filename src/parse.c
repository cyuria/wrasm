
#include "parse.h"

#include <stdlib.h>
#include <string.h>

#include "bytecode.h"
#include "debug.h"
#include "elf/output.h"
#include "form/instructions.h"
#include "macros.h"
#include "registers.h"
#include "stringutil.h"
#include "symbols.h"
#include "xmalloc.h"

const struct args_t empty_args;

int parse_asm(const char *line, struct sectionpos_t position)
{
	logger(DEBUG, no_error, "Parsing assembly %s", line);

	char *linestr = xmalloc(strlen(line) + 1);
	strcpy(linestr, line);

	char *argstart = linestr;
	while (!is_whitespace(*argstart) && *argstart)
		argstart++;
	if (*argstart) {
		*argstart = '\0';
		argstart++;
	}
	char *argstr = trim_whitespace(argstart);

	struct formation_t formation = parse_form(linestr);
	struct args_t args = formation.arg_handler(argstr);

	args = formation.arg_handler(argstr);
	free(argstr);
	free(linestr);

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

	struct args_t args;

	uint8_t *registers[] = {
		&args.rd,
		&args.rs1,
		&args.rs2,
	};

	char *raw = NULL;
	for (size_t i = 0; i < ARRAY_LENGTH(registers); i++) {
		raw = strtok(argstr, ",");
		if (!raw) {
			logger(ERROR, error_instruction_other,
			       "Expected 3 arguments, got %d", i);
			break;
		}

		char *arg = trim_whitespace(raw);
		*registers[i] = get_register_id(arg);
		free(arg);
	}
	if (strtok(argstr, ","))
		logger(ERROR, error_instruction_other,
		       "Instruction has more than three arguments");

	logger(DEBUG, no_error, "Registers parsed, x%d, x%d, x%d", args.rd,
	       args.rs1, args.rs2);

	return args;
}
struct args_t parse_jal(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments for rtype instruction %s",
	       argstr);

	struct args_t args;

	uint8_t *registers[] = {
		&args.rd,
		&args.rs1,
		&args.rs2,
	};

	char *raw = NULL;
	for (size_t i = 0; i < ARRAY_LENGTH(registers); i++) {
		raw = strtok(argstr, ",");
		if (!raw) {
			logger(ERROR, error_instruction_other,
			       "Expected 3 arguments, got %d", i);
			break;
		}

		char *arg = trim_whitespace(raw);
		*registers[i] = get_register_id(arg);
		free(arg);
	}
	if (strtok(argstr, ","))
		logger(ERROR, error_instruction_other,
		       "Instruction has more than three arguments");

	logger(DEBUG, no_error, "Registers parsed, x%d, x%d, x%d", args.rd,
	       args.rs1, args.rs2);

	return args;
}
struct args_t parse_jalr(char *argstr)
{
	logger(DEBUG, no_error, "Parsing arguments for rtype instruction %s",
	       argstr);

	struct args_t args;

	uint8_t *registers[] = {
		&args.rd,
		&args.rs1,
		&args.rs2,
	};

	char *raw = NULL;
	for (size_t i = 0; i < ARRAY_LENGTH(registers); i++) {
		raw = strtok(argstr, ",");
		if (!raw) {
			logger(ERROR, error_instruction_other,
			       "Expected 3 arguments, got %d", i);
			break;
		}

		char *arg = trim_whitespace(raw);
		*registers[i] = get_register_id(arg);
		free(arg);
	}
	if (strtok(argstr, ","))
		logger(ERROR, error_instruction_other,
		       "Instruction has more than three arguments");

	logger(DEBUG, no_error, "Registers parsed, x%d, x%d, x%d", args.rd,
	       args.rs1, args.rs2);

	return args;
}
