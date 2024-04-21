
#include "stringutil.h"

#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "xmalloc.h"

int is_terminating(char c) { return c == '\0' || c == ';' || c == '\n'; }

int is_whitespace(char c) { return c == ' ' || c == '\t' || c == '\n'; }

char *trim_whitespace(const char *str) {
  const char *start = str;
  while (is_whitespace(*start) && !is_terminating(*start))
    start++;

  const char *end = start;
  while (!is_terminating(*end))
    end++;

  while (is_whitespace(*end) && end > start)
    end--;

  char *newstr = xmalloc(end - start + 1);
  if (newstr == NULL) {
    logger(ERROR, error_internal,
           "Unable to allocate memory for stripped string");
    return NULL;
  }
  memcpy(newstr, start, end - start);
  newstr[end - start] = '\0';

  return newstr;
}
