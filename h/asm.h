#pragma once

#include "elf/output.h"
#include "form/generic.h"

int parse_asm(const char *, struct sectionpos_t);
int parse_form(const char *parserstr, struct formation_t *);
int parse_args(char *argstr, struct args_t *);
