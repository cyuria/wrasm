
#include <stdlib.h>

#include "debug.h"
#include "macros.h"
#include "registers.h"

struct {
	const char *symbol;
	const size_t value;
} tests[] = {
	{ "x0", 0 },
	{ "x1", 1 },
	{ "x10", 10 },
	{ "x20", 20 },
	{ "x30", 30 },
	{ "x31", 31 },
	{ "x32", (size_t)-1 },
	{ "x3000", (size_t)-1 },
	{ "x4400000000", (size_t)-1 },
	{ "fp", 8 },
	{ "s0", 8 },
	{ "zero", 0 },
	{ "sp", 2 },
	{ "ra", 1 },
	{ "a3", 13 },
	{ "s2", 18 },
	{ "a8", (size_t)-1 },
	{ "t2", 7 },
	{ "t3", 28 },
	{ "a1", 11 },
};

int main(void)
{
	set_exit_loglevel(NODEBUG);
	set_min_loglevel(DEBUG);
	int errors = 0;
	for (size_t i = 0; i < ARRAY_LENGTH(tests); i++) {
		size_t imm = get_register_id(tests[i].symbol);
		if (imm != tests[i].value) {
			logger(ERROR, error_internal,
			       "Test Failed, expected \"%s\" to equal %d but was given %d",
			       tests[i].symbol, tests[i].value, imm);
			errors++;
		}
	}
	if (errors)
		logger(CRITICAL, error_internal, "%d tests failed", errors);
	return errors != 0 || get_clean_exit(ERROR);
}
