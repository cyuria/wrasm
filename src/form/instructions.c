
#include "form/instructions.h"

#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "form/atomic.h"
#include "form/base.h"
#include "form/csr.h"
#include "form/fencei.h"
#include "form/generic.h"
#include "macros.h"

struct formation parse_form(const char *instruction)
{
	logger(DEBUG, no_error, "Getting formation for instruction %s",
	       instruction);

	const struct formation *sets[] = {
		rv32i, rv64i, rv32a, rv64a, zicsr, zifencei,
	};
	for (size_t i = 0; i < ARRAY_LENGTH(sets); i++) {
		while (sets[i]->name) {
			if (!strcmp(instruction, sets[i]->name))
				return *sets[i];
			sets[i]++;
		}
	}

	logger(ERROR, error_invalid_instruction,
	       "Unknown assembly instruction - %s\n", instruction);
	return (struct formation)END_FORMATION;
}
