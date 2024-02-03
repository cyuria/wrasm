#include "generation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"
#include "debug.h"
#include "labels.h"
#include "output.h"
#include "instructions.h"
#include "stringutil.h"

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

void parse_file(FILE *ifp, FILE *ofp) {
  char *line = NULL;
  size_t linesize = 0;
  size_t nread;

  linenumber = 0;

  while ((nread = getl(&line, &linesize, ifp)) != -1)
    label_forward_declare(line);
  fseek(ifp, 0L, SEEK_SET);

  while ((nread = getl(&line, &linesize, ifp)) != -1) {
    linenumber++;
    logger(DEBUG, no_error, "Parsing line \"%s\"", line);
    if (parse_line(line, get_outputpos()))
      return;
    logger(DEBUG, no_error, " | Finished parsing line");
  }

  alloc_output();

  write_all_instructions();

  free(line);

  linenumber = 0;

  flush_output(ofp);

  free_instructions();
}

int parse_line_trimmed(char *, struct sectionpos_t);
int parse_line(char *line, struct sectionpos_t position) {
  char *trimmed_line = trim_whitespace(line);
  const int result = parse_line_trimmed(trimmed_line, position);
  free(trimmed_line);
  return result;
}

int parse_line_trimmed(char *line, struct sectionpos_t position) {
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

int label_forward_declare(char *line) {
  char *colon = strchr(line, ':');
  if (colon == NULL)
    return 0;
  *colon = '\0';
  char *label = trim_whitespace(line);
  return create_label(label);
}

int parse_label(char *line, struct sectionpos_t position) {
  char *end = strchr(line, ':');
  /* check for invalid label definition */
  if (end == NULL)
    return 0;

  logger(INFO, no_error, "Setting position of label (%s)", line);

  *(end++) = '\0';
  char *labelstr = trim_whitespace(line);

  const int label = get_label_by_name(labelstr);
  if (label < 0) {
    logger(ERROR, error_internal, "Unknown label encountered (%s)", labelstr);
    free(labelstr);
    return 1;
  }

  free(labelstr);

  const struct sectionpos_t fpos = get_outputpos();
  if (unlikely(fpos.offset == -1L)) {
    logger(CRITICAL, error_system, "Unable to determine section file position");
    return 1;
  }

  set_labelpos(label, fpos);

  logger(DEBUG, no_error, "Moving on to line (%s %p)", end, end);
  return parse_line(end, position);
}

int parse_preprocessor(const char *line) {
  /* TODO: implement preprocessor stuff */
  logger(WARN, error_not_implemented, "Preprocessor not implemented");
  return 0;
}
int parse_directive(const char *line) {
  /* TODO: implement directive stuff */
  logger(WARN, error_not_implemented, "Assembler directives not implemented");
  return 0;
}

int write_bytecode(struct bytecode_t *code, struct sectionpos_t position) {

  if (unlikely(code == NULL)) {
    logger(ERROR, error_internal, "Received invalid bytecode");
    return 1;
  }

  size_t nwritten =
      write_sectiondata((char *)code->data, code->size * sizeof(*code->data), position);

  logger(DEBUG, no_error, "%zu bytes written to output", nwritten);

  const int writeerr = nwritten != code->size * sizeof(*code->data);

  free(code);

  if (unlikely(writeerr)) {
    logger(CRITICAL, error_system, "Error writing bytes to output");
    return 1;
  }

  return 0;
}
