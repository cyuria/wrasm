
#include "stringutil.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "xmalloc.h"

int is_terminating(char c)
{
	return !c || c == ';' || c == '\n';
}

char *trim_whitespace(const char *str)
{
	const char *start = str;
	while (isspace(*start) && !is_terminating(*start))
		start++;

	const char *end = start;
	while (!is_terminating(*end))
		end++;

	while (isspace(*end) && end > start)
		end--;

	char *newstr = xmalloc((size_t)(end - start + 1));
	if (newstr == NULL) {
		logger(ERROR, error_internal,
		       "Unable to allocate memory for stripped string");
		return NULL;
	}
	memcpy(newstr, start, (size_t)(end - start));
	newstr[end - start] = '\0';

	return newstr;
}
