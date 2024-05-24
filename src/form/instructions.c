
#include "form/instructions.h"

#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "form/generic.h"
#include "form/rv32i.h"
#include "form/rv64i.h"
#include "macros.h"

struct formation_t parse_form(const char *instruction)
{
	logger(DEBUG, no_error, "Getting formation for instruction %s",
	       instruction);

	const struct formation_t *sets[] = {
		rv32i,
		rv64i,
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
	return (struct formation_t)END_FORMATION;
}
