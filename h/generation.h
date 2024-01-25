#pragma once
#include <stdio.h>

/* TODO: add directives and preprocessor */
//#include "directives.h"
#include "instructions.h"
//#include "preprocessor.h"

/* general instruction generation */
void parse_file(FILE *, FILE *);
int parse_cycle(FILE *);
int parse_label(const char *);
int parse_line(const char *);

int parse_preprocessor(const char *);
int parse_directive(const char *);
int parse_asm(const char *);

struct bytecode_t *placeholder_handler(struct instruction_t instruction,
                                       struct args_t args, int position);
int add_bytecode(struct instruction_t, struct args_t);

