#pragma once

#include "elf/output.h"

/* general instruction generation */
void parse_file(FILE *, FILE *);
int parse_line(char *, struct sectionpos);

int parse_label(char *, struct sectionpos);
