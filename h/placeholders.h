#pragma once

#include "instructions.h"

struct placeholder_t {
  struct instruction_t inst;
  struct args_t args;
  int position;
};

extern struct placeholder_t *placeholders;
extern int placeholders_size;

int add_placeholder(struct placeholder_t);
void free_placeholders(void);
