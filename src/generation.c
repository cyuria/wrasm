#include "generation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bytecode.h"
#include "debug.h"
#include "directives.h"
#include "elf/output.h"
#include "parse.h"
#include "stringutil.h"
#include "symbols.h"
#include "xmalloc.h"

/*
 * custom getline implementation for cross platform support removes newline at
 * end of line and therefore not compatible with GNU/POSIX getline
 */
size_t getl(char **lineptr, size_t *n, FILE *stream)
{
	char *bufptr = NULL;
	char *p = bufptr;
	size_t size;
	int c;

	if (lineptr == NULL)
		return (size_t)-1;
	if (stream == NULL)
		return (size_t)-1;
	if (n == NULL)
		return (size_t)-1;

	bufptr = *lineptr;
	size = *n;

	c = fgetc(stream);
	if (c == EOF)
		return (size_t)-1;

	if (!bufptr) {
		size = 128;
		bufptr = xmalloc(size);
	}
	p = bufptr;
	while (c != EOF) {
		if ((size_t)(p - bufptr) > (size - 1)) {
			size = size + 128;
			bufptr = xrealloc(bufptr, size);
		}
		if (c == '\n')
			break;
		*p++ = (char)c;
		c = fgetc(stream);
	}

	*(p++) = '\0';
	*lineptr = bufptr;
	*n = size;

	return (size_t)(p - bufptr - 1);
}

void parse_file(FILE *ifp, FILE *ofp)
{
	char *line = NULL;
	size_t linesize = 0;
	size_t nread;

	linenumber = 0;

	while ((nread = getl(&line, &linesize, ifp)) != (size_t)-1)
		symbol_forward_declare(line);
	fseek(ifp, 0L, SEEK_SET);

	while ((nread = getl(&line, &linesize, ifp)) != (size_t)-1) {
		linenumber++;
		logger(DEBUG, no_error, "Parsing line \"%s\"", line);
		if (parse_line(line, get_outputpos()))
			return;
		logger(DEBUG, no_error, " | Finished parsing line");
	}

	linenumber = 0;

	calc_strtab();
	calc_symtab();
	alloc_output();

	write_all();

	free(line);

	linenumber = 0;

	fill_strtab();
	fill_symtab();

	flush_output(ofp);

	free_instructions();
}

static int parse_line_trimmed(char *, struct sectionpos);
int parse_line(char *line, struct sectionpos position)
{
	char *trimmed_line = trim_whitespace(line);
	const int result = parse_line_trimmed(trimmed_line, position);
	free(trimmed_line);
	return result;
}

static int parse_line_trimmed(char *line, struct sectionpos position)
{
	logger(DEBUG, no_error, " |-> \"%s\"", line);

	if (*line == '\0')
		return 0;

	if (*line == '.' || *line == '[')
		return parse_directive(line);
	if (*line == ';')
		return 0;
	if (*line == '/' && *(line + 1) == '/')
		return 0;
	if (strchr(line, ':'))
		return parse_label(line, position);

	return parse_asm(line, position);
}

int symbol_forward_declare(char *line)
{
	char *colon = strchr(line, ':');
	if (colon == NULL)
		return 0;
	*colon = '\0';
	char *name = trim_whitespace(line);
	return !create_symbol(name, SYMBOL_LABEL);
}

int parse_label(char *line, struct sectionpos position)
{
	char *end = strchr(line, ':');
	/* check for invalid label definition */
	if (end == NULL)
		return 0;

	logger(DEBUG, no_error, "Setting position of label (%s)", line);

	*(end++) = '\0';
	char *name = trim_whitespace(line);

	struct symbol *label = get_symbol(name);
	if (!label) {
		logger(ERROR, error_internal, "Unknown label encountered (%s)",
		       name);
		free(name);
		return 1;
	}
	if (label->type != SYMBOL_LABEL) {
		logger(ERROR, error_internal,
		       "%s is defined but is not a label", name);
		free(name);
		return 1;
	}

	free(name);

	const struct sectionpos fpos = get_outputpos();
	if (fpos.offset == (size_t)-1) {
		logger(CRITICAL, error_system,
		       "Unable to determine section file position");
		return 1;
	}

	label->section = fpos.section;
	label->value = (long)fpos.offset;

	logger(DEBUG, no_error, "Moving on to line (%s %p)", end, end);
	return parse_line(end, position);
}

int parse_preprocessor(const char *line)
{
	/* TODO: implement preprocessor stuff */
	(void)line;
	logger(WARN, error_not_implemented, "Preprocessor not implemented");
	return 0;
}
