
#include <stdlib.h>

#include "debug.h"
#include "macros.h"
#include "registers.h"

struct {
	const char *symbol;
	const size_t value;
} tests[] = {
	{ "0", 0 },	      { "1", 1 },
	{ "10", 10 },	      { "100", 100 },
	{ "95", 95 },	      { "207", 207 },
	{ "263", 263 },	      { "3000", 3000 },
	{ "0x1", 1 },	      { "0x6", 6 },
	{ "0xA3", 163 },      { "0xa3", 163 },
	{ "0b01001010", 74 }, { "0b11110000", 240 },
	{ "0b00110011", 51 },
};

int main(void)
{
	int errors = 0;
	for (size_t i = 0; i < ARRAY_LENGTH(tests); i++) {
		size_t imm = 0;
		get_immediate(tests[i].symbol, &imm);
		if (imm != tests[i].value) {
			logger(CRITICAL, error_internal,
			       "Test Failed, expected \"%s\" to equal %d but was given %d",
			       tests[i].symbol, tests[i].value, imm);
			errors++;
		}
	}
	if (errors)
		logger(CRITICAL, error_internal, "%d tests failed", errors);
	return errors != 0;
}
