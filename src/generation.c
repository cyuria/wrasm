#define _CRT_SECURE_NO_WARNINGS
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

static inline int is_whitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n';
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

void parse_file(FILE *ifp, FILE *ofp) {
  char *line = NULL;
  size_t linesize = 0;
  size_t nread;

  linenumber = 0;
  outputfile = ofp;

  while ((nread = getl(&line, &linesize, ifp)) != -1) {
    linenumber++;
    logger(DEBUG, no_error, linenumber, "Parsing line \"%s\"", line);
    const int res = parse_line(line);
    if (res)
      return;
  }

  for (int i = 0; i < placeholders_size; i++) {
    const int res = parse_placeholder(placeholders[i]);
    if (res)
      return;
  }

  free(line);
  linenumber = 0;
  outputfile = NULL;
}

int parse_line(const char *line) {
  while (is_whitespace(*line))
    if (!*(line++))
      return 0;

  if (*line == '#' || *line == '%')
    return parse_preprocessor(line);
  if (*line == '.' || *line == '[')
    return parse_directive(line);
  if (*line == ';')
    return 0;
  if (*line == '/' && *(line + 1) == '/')
    return 0;
  if (strchr(line, ':'))
    return parse_label(line);

  return parse_asm(line);
}

int parse_label(const char *line) {

  if (!valid_labelchar(*line))
    return 0;

  const char *end = strchr(line, ':');
  /* check for invalid label definition */
  if (end == NULL)
    return 0;

  logger(INFO, no_error, linenumber, "Attempting to create label (%s)", line);

  char *label = malloc(end - line + 1);
  memcpy(label, line, end - line);
  label[end - line] = '\0';

  if (get_label_by_name(label) >= 0) {
    logger(ERROR, error_instruction_other, linenumber,
           "Label already defined (%s)", label);
    return 1;
  }

  // TODO: create label
  const int fpos = ftell(outputfile);
  if (unlikely(fpos == -1L)) {
    logger(CRITICAL, error_system, linenumber,
           "Unable to determine file position");
    return 1;
  }

  if (create_label(label, fpos) < 0) {
    logger(CRITICAL, error_internal, linenumber, "Unable to create label");
    return 1;
  }

  do {
    end++;
  } while (is_whitespace(*end) && *end);
  logger(DEBUG, no_error, linenumber, "Moving on to line (%s)", end);
  parse_line(end);

  return 0;
}

int parse_preprocessor(const char *line) {
  /* TODO: implement preprocessor stuff */
  logger(WARN, error_not_implemented, linenumber,
         "Preprocessor not implemented");
  return 0;
}
int parse_directive(const char *line) {
  /* TODO: implement directive stuff */
  logger(WARN, error_not_implemented, linenumber,
         "Assembler directives not implemented");
  return 0;
}

int parse_asm(const char *line) {
  logger(DEBUG, no_error, linenumber, "Parsing assembly %s", line);

  // if line is empty
  if (!*line)
    return 0;

  const char *end = line;

  while (*end && !is_whitespace(*end))
    end++;

  const size_t instidsize = end - line + 1;
  char *const instid = malloc(instidsize);
  if (unlikely(instid == NULL)) {
    logger(CRITICAL, error_internal, linenumber, "Unable to allocate memory");
    return 1;
  }
  memcpy(instid, line, instidsize - 1);
  instid[instidsize - 1] = '\0';

  logger(DEBUG, no_error, linenumber, "Assembly instruction name (%s)", instid);

  const struct instruction_t *instruction = rv64i;
  while (strcmp(instid, instruction->name)) {
    instruction++;
    if (unlikely(!instruction->name)) {
      logger(ERROR, error_invalid_instruction, linenumber,
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
    logger(ERROR, error_invalid_instruction, linenumber,
           "Invalid instruction arguments (expected 0 or 1 argument/s)");
    free(argstr);

    return 1;
  }

  while (is_whitespace(*(args[1])) && *(args[1]))
    args[1]++;

  int r0 = get_register_id(args[0]);

  /* U and J type arg formats */
  if (args[2] == NULL) {
    logger(DEBUG, no_error, linenumber, "Found U/J type argument format");
    if (unlikely(r0 < 0)) {
      logger(ERROR, error_invalid_instruction, linenumber,
             "Invalid instruction arguments (expected register in first argument)");
      return 1;
    }

    int arg1 = get_label_by_name(args[1]);
    if (arg1 != -1) {
      free(argstr);
      return write_placeholder(*instruction,
                               (struct args_t){UJ_TYPE, .uj = {r0, arg1}});
    }

    if (unlikely(get_immediate(args[1], &arg1))) {
      logger(INFO, no_error, linenumber, "Invalid label or immediate (%s)",
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

int parse_placeholder(struct placeholder_t ph) {
  logger(DEBUG, no_error, 0, "Parsing placeholder (%s)", ph.instruction.name);
  if (unlikely(outputfile == NULL)) {
    logger(WARN, error_internal, 0,
           "Attempt to add placeholder bytecode without file");
    return 0;
  }
  if (fseek(outputfile, ph.position, SEEK_SET)) {
    logger(ERROR, error_internal, 0,
           "Unable to set file cursor position while replacing placeholder");
    return 1;
  }
  ph.args.uj.imm = get_label_position(ph.args.uj.imm);
  if (ph.args.uj.imm < 0) {
    logger(ERROR, error_internal, 0, "Label not found");
    return 1;
  }
  return add_bytecode(ph.instruction, ph.args);
}

int write_placeholder(struct instruction_t instruction, struct args_t args) {
  const int position = ftell(outputfile);
  if (unlikely(position == -1L)) {
    logger(CRITICAL, error_system, linenumber,
           "Unable to determine file position");
    return 1;
  }
  logger(DEBUG, no_error, linenumber,
         "Adding placeholder instruction (%s 0x%.08x)", instruction.name,
         position);
  add_placeholder((struct placeholder_t){instruction, args, position});
  struct bytecode_t *code =
      malloc(sizeof(*code) + RV64I_SIZE * sizeof(*code->data));
  code->size = RV64I_SIZE;
  *(uint32_t *)&(code->data) = 0;
  return write_bytecode(code);
}
int add_bytecode(struct instruction_t instruction, struct args_t args) {
  if (unlikely(outputfile == NULL)) {
    logger(WARN, error_internal, linenumber,
           "Attempt to add bytecode without file");
    return 0;
  }

  const int fpos = ftell(outputfile);
  if (unlikely(fpos == -1L)) {
    logger(CRITICAL, error_system, linenumber,
           "Unable to determine file position");
    return 1;
  }

  if (unlikely(args.type != instruction.argtype)) {
    logger(ERROR, error_invalid_instruction, linenumber,
           "Invalid instruction arguments (Expected different argument types)");
    return 1;
  }

  logger(DEBUG, no_error, linenumber,
         "Generating code for instruction at offset 0x%.08x", fpos);

  struct bytecode_t *code = instruction.handler(instruction, args, fpos);

  if (unlikely(code == NULL)) {
    logger(ERROR, error_internal, linenumber, "Failed to generate bytecode");
    return 1;
  }

  return write_bytecode(code);
}

int write_bytecode(struct bytecode_t *code) {

  if (unlikely(code == NULL)) {
    logger(ERROR, error_internal, linenumber, "Received invalid bytecode");
    return 1;
  }

  size_t nwritten =
      fwrite(code->data, sizeof(*code->data), code->size, outputfile);

  logger(DEBUG, no_error, linenumber, "%zu bytes written to output", nwritten);

  const int writeerr = nwritten != code->size;

  free(code);

  if (unlikely(writeerr)) {
    logger(CRITICAL, error_system, linenumber, "Error writing bytes to output");
    return 1;
  }

  return 0;
}
