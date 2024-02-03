#pragma once

/* TODO: add directives and preprocessor */
//#include "directives.h"
#include "parsers.h"
#include "output.h"
//#include "preprocessor.h"

/* general instruction generation */
void parse_file(FILE *, FILE *);
int parse_line(char *, struct sectionpos_t);

int label_forward_declare(char *);
int parse_label(char *, struct sectionpos_t);
int parse_directive(const char *);

int write_bytecode(struct bytecode_t *, struct sectionpos_t);
