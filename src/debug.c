
#include "debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "args.h"

size_t linenumber;

const char *level_names[6] = {
	"debug", "info", "warning", "error", "critical", "none",
};
const char *level_colours[6] = {
	"",
	"\033[0;36;1m",
	"\033[0;35;1m",
	"\033[0;31;1m",
	"\033[1;31;1m",
	"\033[1m",
};
size_t level_instances[6] = { 0 };

static enum loglvl_t minloglevel = WARN;
static enum loglvl_t exitloglevel = ERROR;

void set_min_loglevel(enum loglvl_t level)
{
	minloglevel = level;
	printf("setting min log level to %s\n", level_names[minloglevel]);
	logger(INFO, no_error, "Log level set to %s%s\033[0m",
	       level_colours[minloglevel], level_names[minloglevel]);
}

void logger(enum loglvl_t level, enum error_t id, const char *format, ...)
{
	level_instances[level]++;
	if (level < minloglevel)
		return;

	FILE *out = stderr;

	fprintf(out, "%s: %s0x%.02x\033[0m / %s%s\033[0m ", progname,
		level_colours[level], id, level_colours[level],
		level_names[level]);
	if (linenumber)
		fprintf(out, "- %sL%lu\033[0m ", level_colours[level],
			(unsigned long)linenumber);

	va_list format_params;
	va_start(format_params, format);
	vfprintf(out, format, format_params);
	va_end(format_params);

	putc('\n', out);

	if (level >= exitloglevel)
		exit(1);
}

int get_clean_exit(enum loglvl_t level)
{
	for (; level <= NODEBUG; level++)
		if (level_instances[level])
			return 1;
	return 0;
}
