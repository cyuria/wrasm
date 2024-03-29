#include "directives.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "instructions.h"
#include "output.h"
#include "stringutil.h"

struct directive_t {
  const char *name;
  int (*parser)(const char *);
};
struct directive_t directive_map[] = {
    {".string", parse_asciz},
    {".asciz", parse_asciz},
    {".ascii", parse_ascii},
    {".section", parse_section},
};
#define DIRECTIVE_COUNT (sizeof(directive_map) / sizeof(*directive_map))
struct {
  const char *name;
  enum sections_e section;
}
section_map[] = {
  {".text", section_text},
  {".data", section_data},
  {".rodata", section_rodata},
  {".bss", section_bss}
};
#define SELECTABLE_SECTION_COUNT (sizeof(section_map) / sizeof(*section_map))

static struct directive_t get_parser(const char *name) {
  for (int i = 0; i < DIRECTIVE_COUNT; i++)
    if (!strcmp(name, directive_map[i].name))
      return directive_map[i];
  logger(ERROR, error_invalid_instruction,
         "Unknown directive found with name: %s", name);
  return (struct directive_t){NULL, NULL};
}

int parse_directive(char *line) {
  /* TODO: implement directive parsing */
  char *directivename = line;
  while (!is_whitespace(*line) && *line)
    line++;
  if (*line)
    *(line++) = '\0';

  struct directive_t directive = get_parser(directivename);

  if (directive.parser == NULL)
    return 1;

  char *args = trim_whitespace(line);
  const int result = directive.parser(args);
  free(args);
  return result;
}

static char get_escapedchar(const char **str) {
  switch (**str) {
  case 'x':
  case 'u': {
    char *end;
    long val = strtol(*str + 1, &end, (**str == 'x') ? 16 : 8);
    if (val >= 256 || val < 0) {
      logger(ERROR, error_invalid_instruction, "Escape sequence out of range");
      return '\0';
    }
    *str = end - 1;
    return val;
  }
  case 'a':
    return '\a';
    break;
  case 'b':
    return '\b';
  case 'e':
    return '\e';
  case 'f':
    return '\f';
  case 'n':
    return '\n';
  case 'r':
    return '\r';
  case 't':
    return '\t';
  case 'v':
    return '\v';
  case '\\':
    return '\\';
  case '"':
    return '"';
  }
  return '\0';
}

static inline size_t parse_nullstr(char *dest, const char *str) {
  if (*str != '"')
    return -1;
  str++;
  int size = 0;
  while (*str != '"') {
    register char val = *str;
    if (!val) {
      logger(ERROR, error_invalid_syntax,
             "Expected '\"' (0x22 double quote) character at start or end of "
             "ascii string");
      return -1;
    }
    if (val == '\\') {
      str++;
      val = get_escapedchar(&str);
    }
    dest[size] = val;
    size++;
    str++;
  }
  dest[size] = '\0';
  size++;
  str++;
  return size;
}

static int parse_ascii_generic(const char *str, const bool nullterm) {
  char *parsed = malloc(strlen(str) - 1);
  const size_t size = parse_nullstr(parsed, str) - !nullterm;
  if (size < 0)
    return 1;
  char *data = malloc(size);
  memcpy(data, parsed, size);
  free(parsed);
  const struct sectionpos_t position = get_outputpos();
  const int res = add_data((struct rawdata_t){.data = data,
                                              .size = size,
                                              .position = position,
                                              .line = linenumber});
  inc_outputsize(position.section, size);
  return res;
}
int parse_asciz(const char *str) {
  return parse_ascii_generic(str, true);
}
int parse_ascii(const char *str) {
  return parse_ascii_generic(str, false);
}
int parse_section(const char *str) {
  logger(DEBUG, no_error, "Selecting Section \"%s\"", str);
  for (int i = 0; i < SELECTABLE_SECTION_COUNT; i++) {
    if (!strcmp(str, section_map[i].name)) {
      change_output(section_map[i].section);
      return 0;
    }
  }
  logger(WARN, error_invalid_instruction, "Unknown Section \"%s\"", str);
  return 1;
}
