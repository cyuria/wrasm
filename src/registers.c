
#include "registers.h"
#include "debug.h"
#include "macros.h"

#include <stdlib.h>
#include <string.h>

const char *reg_abi_map[] = {
	"zero", "ra", "sp", "gp", "tp",	 "t0",	"t1", "t2", "s0", "s1", "a0",
	"a1",	"a2", "a3", "a4", "a5",	 "a6",	"a7", "s2", "s3", "s4", "s5",
	"s6",	"s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6",
};

/* TODO: implement w/ float extension */
const char *float_reg_abi_map[] = { 0 };

size_t get_register_id(const char *reg)
{
	logger(DEBUG, no_error, "Searching for register (%s)", reg);

	/* limit the number of possible characters in the register */
	int l = 0;
	while (reg[l] && reg[l] != ' ')
		l++;
	if (l > 4)
		return (size_t)-1;

	if (*reg == 'x') {
		size_t r = (size_t)atol(reg + 1);
		if (r >= 32)
			return (size_t)-1;
		return r;
	}

	for (size_t i = 0; i < ARRAY_LENGTH(reg_abi_map); i++)
		if (!strcmp(reg, reg_abi_map[i]))
			return i;

	/* Check for "fp" alias of register "s0"/"x8" */
	if (!strcmp(reg, "fp"))
		return 8;

	logger(INFO, no_error, "unknown register (%s)", reg);

	return (size_t)-1;
}

size_t get_float_register_id(const char *reg)
{
	if (*reg != 'f')
		return (size_t)-1;

	if (reg[1] >= '0' && reg[1] <= '9')
		return atoi(reg + 1);

	for (size_t i = 0; i < ARRAY_LENGTH(float_reg_abi_map); i++)
		if (!strcmp(reg, float_reg_abi_map[i]))
			return i;

	return (size_t)-1;
}

static int calc_digit(char digit)
{
	if (digit > 'z')
		return -1;
	if (digit >= 'a')
		return digit - 'a' + 10;
	if (digit > 'Z')
		return -1;
	if (digit >= 'A')
		return digit - 'A' + 10;
	if (digit > '9')
		return -1;
	return digit - '0';
}

static size_t find_base(const char **imm)
{
	if ((*imm)[0] != '0')
		return 10;

	const char delim = (*imm)[1];
	*imm += 2;

	switch (delim) {
	case 'x':
		return 16;
	case 'o':
		return 8;
	case 'b':
		return 2;
	default:
		return 10;
	}
}

int get_immediate(const char *imm, size_t *res)
{
	const unsigned base = find_base(&imm);

	*res = (size_t)strtoll(imm, NULL, (int)base);

	/* We know strtol didn't fail */
	if (*res)
		return 0;

	/*
	 * check for anything that could've caused a failure
	 * the immediate negative zero "-0" will cause a fail
	 */
	while (*imm) {
		int digit = calc_digit(*imm);
		if (digit >= (int)base || digit < 0)
			return 1;
		imm++;
	}

	return 0;
}
