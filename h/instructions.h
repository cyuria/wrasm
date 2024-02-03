#pragma once

#include "parsers.h"
#include "output.h"

struct instruction_t {
  struct parser_t parser;
  struct args_t args;
  struct sectionpos_t position;
  size_t line;
};

int add_instruction(struct instruction_t);

int write_all_instructions(void);
int write_instruction(struct instruction_t);

void free_instructions(void);
