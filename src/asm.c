
#include "asm.h"

#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "elf/output.h"
#include "instructions.h"
#include "macros.h"
#include "parsers.h"
#include "registers.h"
#include "stringutil.h"
#include "symbols.h"
#include "xmalloc.h"

static int parse_parser(char *, struct parser_t *);
static int parse_arg(const char *, enum argtype_e *, size_t *);
static int parse_args(char *, struct args_t *);

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

	struct args_t args;
	struct parser_t parser;

	if (parse_parser(linestr, &parser)) {
		free(argstr);
		free(linestr);
		return 1;
	}
	if (parse_args(argstr, &args)) {
		free(argstr);
		free(linestr);
		return 1;
	}

	free(argstr);
	free(linestr);

	add_instruction((struct instruction_t){ .args = args,
						.line = linenumber,
						.parser = parser,
						.position = position });
	inc_outputsize(position.section, parser.isize);
	logger(DEBUG, no_error, "Updated position to offset (%zu)",
	       position.offset);

	return 0;
}

static int parse_parser(char *parserstr, struct parser_t *parser)
{
	logger(DEBUG, no_error, "Getting parser for instruction (%s)",
	       parserstr);

	const struct parser_t *sets[] = {
		rv64s,
		rv64i,
	};
	for (size_t i = 0; i < ARRAY_LENGTH(sets); i++) {
		while (sets[i]->name) {
			if (!strcmp(parserstr, sets[i]->name)) {
				*parser = *sets[i];
				return 0;
			}
			sets[i]++;
		}
	}
	logger(ERROR, error_invalid_instruction,
	       "Unknown assembly instruction - %s\n", parserstr);
	return 1;
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
	const size_t status = get_immediate(str, arg);
	if (!status) {
		*type = arg_immediate;
		return 0;
	}
	logger(ERROR, error_instruction_other,
	       "Uknown argument (%s) encountered", str);
	return 0;
}

static int parse_args(char *argstr, struct args_t *args)
{
	// TODO: reimplement
	logger(DEBUG, no_error, "Parsing arguments for instruction (%s)",
	       argstr);

	args->type[0] = arg_none;
	args->type[1] = arg_none;
	args->type[2] = arg_none;

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
		parse_arg(arg_strs[i], &args->type[i], &args->arg[i]);

	logger(DEBUG, no_error,
	       "Arguments parsed, (%d, %d), (%d, %d), (%d, %d)", args->type[0],
	       args->arg[0], args->type[1], args->arg[1], args->type[2],
	       args->arg[2]);

	return 0;
}
