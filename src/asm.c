
#include "asm.h"

#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "elf/output.h"
#include "instructions.h"
#include "parsers.h"
#include "registers.h"
#include "stringutil.h"
#include "symbols.h"

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

static int parse_parser(char *, struct parser_t *);
static int parse_args(char *, struct args_t *);
static int find_symbol_or_immediate(const char *, int *);

static struct args_t parse_failargs(int[3], uint32_t);
static struct args_t parse_noargs(int[3], uint32_t);
static struct args_t parse_uj_args(int[3], uint32_t);
static struct args_t parse_isb_args(int[3], uint32_t);
static struct args_t parse_r_args(int[3], uint32_t);

static struct args_t (*arg_parsers[])(int[3], uint32_t) = {
    &parse_noargs,   &parse_failargs, &parse_failargs, &parse_uj_args,
    &parse_failargs, &parse_isb_args, &parse_r_args};

int parse_asm(const char *line, struct sectionpos_t position) {
  logger(DEBUG, no_error, "Parsing assembly %s", line);

  char *linestr = malloc(strlen(line) + 1);
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

  add_instruction((struct instruction_t){.args = args,
                                         .line = linenumber,
                                         .parser = parser,
                                         .position = position});
  inc_outputsize(position.section, sizeof(uint16_t) * RV64I_SIZE);
  logger(DEBUG, no_error, "Updated position to offset (%zu)", position.offset);

  return 0;
}

static int find_symbol_or_immediate(const char *arg, int *immediate) {
  const struct symbol_t *symbol = get_symbol(arg);
  if (!symbol)
    return get_immediate(arg, immediate);
  *immediate = calc_fileoffset((struct sectionpos_t){
      .offset = symbol->value,
      .section = symbol->section,
  });
  return 0;
}

static int parse_parser(char *parserstr, struct parser_t *parser) {
  logger(DEBUG, no_error, "Getting parser for instruction (%s)", parserstr);

  const struct parser_t *parser_cur = rv64i;
  while (strcmp(parserstr, parser_cur->name)) {
    parser_cur++;
    if (unlikely(!parser_cur->name)) {
      logger(ERROR, error_invalid_instruction,
             "Unknown assembly instruction - %s\n", parserstr);
      return 1;
    }
  }
  *parser = *parser_cur;
  return 0;
}

static int parse_args(char *argstr, struct args_t *args) {
  logger(DEBUG, no_error, "Parsing arguments for instruction (%s)", argstr);

  int regcount = 0;
  int has_immediate = 0;
  uint32_t immediate = 0;
  int registers[3];

  char *arg_raw = strtok(argstr, ",");
  while (arg_raw) {
    char *arg = trim_whitespace(arg_raw);

    if (unlikely(regcount >= 3)) {
      logger(ERROR, error_instruction_other,
             "Instruction has too many arguments");
      return 1;
    }

    if ((registers[regcount] = get_register_id(arg)) != -1) {
      regcount++;
      free(arg);
      arg_raw = strtok(NULL, ",");
      continue;
    }

    if (unlikely(find_symbol_or_immediate(arg, (int32_t *)&immediate))) {
      logger(ERROR, error_instruction_other,
             "Unable to calculate value for \"%s\"");
      return 1;
    }

    has_immediate = 1;
    break;
  }

  *args = arg_parsers[regcount * 2 + has_immediate](registers, immediate);
  if (args->type == ERROR_ARGTYPE) {
    logger(ERROR, error_instruction_other, "Invalid arguments given");
    return 1;
  }

  return 0;
}

static struct args_t parse_failargs(int registers[3], uint32_t immediate) {
  (void)registers;
  (void)immediate;
  logger(WARN, error_invalid_instruction, "Unknown argument format found");
  return (struct args_t){.type = ERROR_ARGTYPE};
}
static struct args_t parse_noargs(int registers[3], uint32_t immediate) {
  (void)registers;
  (void)immediate;
  return (struct args_t){.type = NOARGS_TYPE};
}
static struct args_t parse_uj_args(int registers[3], uint32_t immediate) {
  return (struct args_t){.type = UJ_TYPE,
                         .uj = {.rd = registers[0], .imm = immediate}};
}
static struct args_t parse_isb_args(int registers[3], uint32_t immediate) {
  return (struct args_t){
      .type = ISB_TYPE,
      .isb = {.r1 = registers[0], .r2 = registers[1], .imm = immediate}};
}
static struct args_t parse_r_args(int registers[3], uint32_t immediate) {
  (void)immediate;
  return (struct args_t){
      .type = R_TYPE,
      .r = {.rd = registers[0], .rs1 = registers[1], .rs2 = registers[2]}};
}
