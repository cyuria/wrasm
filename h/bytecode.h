#pragma once

#include <stdbool.h>

#include "elf/output.h"
#include "form/instructions.h"

struct instruction {
	struct formation formation;
	struct args args;
	struct sectionpos position;
	size_t line;
};

struct rawdata {
	void *data;
	size_t size;
	struct sectionpos position;
	size_t line;
};

int add_instruction(struct instruction);
int add_data(struct rawdata);

int write_all(void);

int write_all_instructions(void);
int write_instruction(struct instruction);

int write_all_data(void);
int write_data(struct rawdata);

void free_instructions(void);
