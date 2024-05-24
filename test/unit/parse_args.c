
#include "parse.h"

#include <string.h>

#include "debug.h"
#include "macros.h"
#include "xmalloc.h"

typedef int test_parse(char *, struct args_t);

struct case_t {
	const char *argstr;
	struct args_t expected;
};

struct case_t cases_rtype[] = {
	{ "x2, x25, x12", { .rd = 2, .rs1 = 25, .rs2 = 12 } },
	{ "sp, zero, t1", { .rd = 2, .rs1 = 0, .rs2 = 6 } },
};

struct case_t cases_itype[] = {
	{ "x2, x25, 5", { .rd = 2, .rs1 = 25, .imm = 5 } },
	{ "sp, zero, -2024", { .rd = 2, .rs1 = 0, .imm = (uint32_t)-2024 } },
};

int test_rtype(char *argstr, struct args_t expected)
{
	const struct args_t args = parse_rtype(argstr);
	const int rd = args.rd == expected.rd;
	const int rs1 = args.rs1 == expected.rs1;
	const int rs2 = args.rs2 == expected.rs2;

	if (!rd || !rs1 || !rs2) {
		logger(ERROR, error_internal,
		       "rtype argument string incorrectly parsed as x%d, x%d, x%d",
		       args.rd, args.rs1, args.rs2);
		return 1;
	}
	return 0;
}

int test_itype(char *argstr, struct args_t expected)
{
	const struct args_t args = parse_itype(argstr);
	const int rd = args.rd == expected.rd;
	const int rs1 = args.rs1 == expected.rs1;
	const int imm = args.imm == expected.imm;

	if (!rd || !rs1 || !imm) {
		logger(ERROR, error_internal,
		       "itype argument string incorrectly parsed as x%d, x%d, %d",
		       args.rd, args.rs1, args.imm);
		logger(INFO, error_internal, "expected x%d, x%d, %d",
		       expected.rd, expected.rs1, expected.imm);
		return 1;
	}
	return 0;
}

int test_cases(struct case_t *cases, size_t count, test_parse test)
{
	int errors = 0;
	for (size_t i = 0; i < count; i++) {
		const size_t str_sz = strlen(cases[i].argstr) + 1;
		char *argstr = xmalloc(str_sz);
		memcpy(argstr, cases[i].argstr, str_sz);
		if (test(argstr, cases[i].expected))
			errors++;
		free(argstr);
	}
	return errors != 0;
}

int main(void)
{
	set_exit_loglevel(NODEBUG);
	set_min_loglevel(DEBUG);

	int errors = 0;
	errors +=
		test_cases(cases_rtype, ARRAY_LENGTH(cases_rtype), &test_rtype);
	errors +=
		test_cases(cases_itype, ARRAY_LENGTH(cases_itype), &test_itype);
	return errors != 0 || get_clean_exit(ERROR);
}
