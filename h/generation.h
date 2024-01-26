#pragma once
#include <stdio.h>

/* TODO: add directives and preprocessor */
//#include "directives.h"
#include "instructions.h"
#include "placeholders.h"
//#include "preprocessor.h"

/* general instruction generation */
void parse_file(FILE *, FILE *);
int parse_cycle(FILE *);
int parse_label(const char *);
int parse_line(const char *);

int parse_preprocessor(const char *);
int parse_directive(const char *);
int parse_asm(const char *);

int parse_placeholder(struct placeholder_t);
int write_placeholder(struct instruction_t, struct args_t);
int add_bytecode(struct instruction_t, struct args_t);

int write_bytecode(struct bytecode_t *);
