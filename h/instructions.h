#pragma once

#include <stdbool.h>

#include "elf/output.h"
#include "form/generic.h"

struct instruction_t {
	struct formation_t formation;
	struct args_t args;
	struct sectionpos_t position;
	size_t line;
};

struct rawdata_t {
	void *data;
	size_t size;
	struct sectionpos_t position;
	size_t line;
};

int add_instruction(struct instruction_t);
int add_data(struct rawdata_t);

int write_all(void);

int write_all_instructions(void);
int write_instruction(struct instruction_t);

int write_all_data(void);
int write_data(struct rawdata_t);

void free_instructions(void);
