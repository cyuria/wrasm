#pragma once

#include "elf/output.h"
#include "parsers.h"

int parse_asm(const char *, struct sectionpos_t);
int parse_parser(const char *parserstr, struct parser_t *);
int parse_args(char *argstr, struct args_t *);
