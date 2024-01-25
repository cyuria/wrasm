#include "generation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "instructions.h"
#include "labels.h"
#include "placeholders.h"
#include "registers.h"

/* custom getline implementation for cross platform support */
/* NOTE: removes newline at end of line and therefore not compatible with
 * GNU/POSIX getline */
size_t getl(char **lineptr, size_t *n, FILE *stream) {
  char *bufptr = NULL;
  char *p = bufptr;
  size_t size;
  int c;

  if (lineptr == NULL)
    return -1;
  if (stream == NULL)
    return -1;
  if (n == NULL)
    return -1;

  bufptr = *lineptr;
  size = *n;

  c = fgetc(stream);
  if (c == EOF)
    return -1;

  if (bufptr == NULL) {
    bufptr = malloc(128);
    if (bufptr == NULL)
      return -1;
    size = 128;
  }
  p = bufptr;
  while (c != EOF) {
    if ((p - bufptr) > (size - 1)) {
      size = size + 128;
      bufptr = realloc(bufptr, size);
      if (bufptr == NULL)
        return -1;
    }
    if (c == '\n')
      break;
    *p++ = c;
    c = fgetc(stream);
  }

  *p++ = '\0';
  *lineptr = bufptr;
  *n = size;

  return p - bufptr - 1;
}

static int linenumber;
static FILE *outputfile;

void parse_file(FILE *ifp, FILE *ofp) {
  char *line = NULL;
  size_t linesize = 0;
  size_t nread;

  linenumber = 0;
  outputfile = ofp;

  while ((nread = getl(&line, &linesize, ifp)) != -1) {
    linenumber++;
    logger(DEBUG, NO_ERROR, linenumber, "Parsing line \"%s\"", line);
    int res = parse_line(line);
    if (res)
      return;
  }

  free(line);
  linenumber = 0;
  outputfile = NULL;
}

int parse_line(const char *line) {
  switch (*line) {
  case '#':
  case '%':
    return parse_preprocessor(line);
  case '.':
  case '[':
    return parse_directive(line);
  case ' ':
  case '\t':
    return parse_asm(line);
  case ';':
    /* comment */
    return 0;
  case '/':
    if (*(line + 1) == '/')
      return 0;
  default:
    logger(INFO, NO_ERROR, linenumber, "Attempting to create label (%s)", line);
    return parse_label(line);
  }
}

static inline int valid_labelchar(const char c) {
  if (c == '_')
    return 1;
  if (c >= '0' && c <= '9')
    return 1;
  if (c >= 'A' && c <= 'Z')
    return 1;
  if (c >= 'a' && c <= 'z')
    return 1;

  return 0;
}

int parse_label(const char *line) {
  if (!valid_labelchar(*line))
    return 0;

  const char *end = strchr(line, ':');
  /* check for invalid label definition */
  if (end == NULL)
    return 0;
  char *label = malloc(end - line + 1);
  memcpy(label, line, end - line);
  label[end - line] = '\0';

  if (get_label_by_name(label) >= 0) {
    logger(ERROR, ERROR_INSTRUCTION_OTHER, linenumber, "Label already defined (%s)", label);
    return 1;
  }

  // TODO: create label
  const int fpos = ftell(outputfile);
  if (unlikely(fpos == -1L)) {
    logger(CRITICAL, ERROR_SYSTEM, linenumber,
           "Unable to determine file position");
    return 1;
  }

  if (create_label(label, fpos) < 0) {
    logger(CRITICAL, ERROR_INTERNAL, linenumber, "Unable to create label");
    return 1;
  }

  parse_asm(end + 1);

  return 0;
}

static inline int is_whitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n';
}

int parse_preprocessor(const char *line) {
  /* TODO: implement preprocessor stuff */
  logger(ERROR, ERROR_NOT_IMPLEMENTED, linenumber,
         "Preprocessor not implemented");
  return 1;
}
int parse_directive(const char *line) {
  /* TODO: implement directive stuff */
  logger(ERROR, ERROR_NOT_IMPLEMENTED, linenumber,
         "Assembler directives not implemented");
  return 1;
}

int parse_asm(const char *line) {
  /* remove inital whitespace */
  while (is_whitespace(*line))
    line++;

  logger(DEBUG, NO_ERROR, linenumber, "Parsing assembly %s", line);

  // if line is empty
  if (!*line)
    return 0;

  const char *end = line;

  while (*end && !is_whitespace(*end))
    end++;

  const size_t instidsize = end - line + 1;
  char *const instid = malloc(instidsize);
  if (unlikely(instid == NULL)) {
    logger(CRITICAL, ERROR_INTERNAL, linenumber, "Unable to allocate memory");
    return 1;
  }
  memcpy(instid, line, instidsize - 1);
  instid[instidsize - 1] = '\0';

  logger(DEBUG, NO_ERROR, linenumber, "Assembly instruction name (%s)", instid);

  const struct instruction_t *instruction = rv64i;
  while (strcmp(instid, instruction->name)) {
    instruction++;
    if (unlikely(!*instruction->name)) {
      logger(ERROR, ERROR_INVALID_INSTRUCTION, linenumber,
             "Unknown assembly instruction - %s\n", instid);
      free(instid);
      return 1;
    }
  }

  free(instid);

  while (is_whitespace(*end))
    end++;

  if (!*end)
    return add_bytecode(*instruction, (struct args_t){NOARGS_TYPE});

  char *const argstr = malloc(strlen(end) + 1);
  strcpy(argstr, end);

  char *args[3] = {argstr, NULL, NULL};
  for (int i = 1; i < 3; ++i) {
    char *chr = args[i - 1];
    while (*chr && *chr != ',')
      chr++;
    if (!*chr)
      break;
    *chr = '\0';
    chr++;
    args[i] = chr;
  }

  /* check for error */
  if (unlikely(args[1] == NULL)) {
    logger(ERROR, ERROR_INVALID_INSTRUCTION, linenumber,
           "Invalid instruction arguments");
    free(argstr);

    return 1;
  }

  while (is_whitespace(*(args[1])) && *(args[1]))
    args[1]++;

  int r0 = get_register_id(args[0]);

  /* U and J type arg formats */
  if (args[2] == NULL) {
    logger(DEBUG, NO_ERROR, linenumber, "Found U/J type argument format");
    if (unlikely(r0 < 0)) {
      logger(ERROR, ERROR_INVALID_INSTRUCTION, linenumber,
             "Invalid instruction arguments");
      return 1;
    }

    int arg1 = get_label_by_name(args[1]);
    if (arg1 != -1) {
      free(argstr);
      struct instruction_t inst = *instruction;
      inst.handler = &placeholder_handler;
      return add_bytecode(inst, (struct args_t){UJ_TYPE, .uj = {r0, arg1}});
    }

    if (unlikely(get_immediate(args[1], &arg1))) {
      logger(INFO, NO_ERROR, linenumber, "Invalid label or immediate (%s)",
             args[1]);
      free(argstr);
      return 1;
    }

    free(argstr);
    return add_bytecode(*instruction,
                        (struct args_t){UJ_TYPE, .uj = {r0, arg1}});
  }

  while (is_whitespace(*(args[2])) && *(args[1]))
    args[2]++;

  int r1 = get_register_id(args[1]);
  /* check for error */
  if (unlikely(r1 < 0)) {
    free(argstr);
    return 1;
  }

  int r2 = get_register_id(args[2]);

  /* I, S, B and R type args */
  if (r2 < 0) {
    int imm;
    if (unlikely(get_immediate(args[2], &imm))) {
      return 1;
      free(argstr);
    }
    struct args_t opts = (struct args_t){ISB_TYPE, .isb = {r0, r1, imm}};
    free(argstr);
    return add_bytecode(*instruction, opts);
  } else {
    const struct args_t opts = (struct args_t){R_TYPE, .isb = {r0, r1, r2}};
    free(argstr);
    return add_bytecode(*instruction, opts);
  }
}

struct bytecode_t *placeholder_handler(struct instruction_t instruction,
                                       struct args_t args, int position) {
  logger(DEBUG, NO_ERROR, linenumber, "Adding placeholder instruction (%s)", instruction.name);
  add_placeholder((struct placeholder_t){instruction, args, position});
  struct bytecode_t *ph = malloc(sizeof(*ph) + 2 * sizeof(*ph->data));
  ph->size = 2;
  *(uint32_t *)&(ph->data) = 0;
  return ph;
}
int add_bytecode(struct instruction_t instruction, struct args_t args) {
  if (unlikely(outputfile == NULL)) {
    logger(WARN, ERROR_INTERNAL, linenumber,
           "Attempt to add bytecode without file");
    return 0;
  }

  const int fpos = ftell(outputfile);
  if (unlikely(fpos == -1L)) {
    logger(CRITICAL, ERROR_SYSTEM, linenumber,
           "Unable to determine file position");
    return 1;
  }

  if (unlikely(args.type != instruction.argtype)) {
    logger(ERROR, ERROR_INVALID_INSTRUCTION, linenumber,
           "Invalid instruction arguments");
    return 1;
  }

  struct bytecode_t *code = instruction.handler(instruction, args, fpos);

  if (unlikely(code == NULL)) {
    logger(ERROR, ERROR_INTERNAL, linenumber, "Failed to generate bytecode");
    return 1;
  }

  size_t nwritten =
      fwrite(code->data, sizeof(*code->data), code->size, outputfile);

  logger(DEBUG, NO_ERROR, linenumber, "%zu bytes written to output", nwritten);

  const int writeerr = nwritten != code->size;
  free(code);

  if (unlikely(writeerr)) {
    logger(CRITICAL, ERROR_SYSTEM, linenumber, "Error writing bytes to output");
    return 1;
  }

  return 0;
}
