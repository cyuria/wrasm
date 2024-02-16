#pragma once

#include "output.h"

/* general instruction generation */
void parse_file(FILE *, FILE *);
int parse_line(char *, struct sectionpos_t);

int label_forward_declare(char *);
int parse_label(char *, struct sectionpos_t);
