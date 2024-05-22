
#include <string.h>

#include "asm.h"
#include "debug.h"
#include "form/generic.h"

int compare_formations(struct formation_t *a, struct formation_t *b)
{
	if (a->isize != b->isize)
		return 0;
	if (a->handler != b->handler)
		return 0;
	if (a->opcode != b->opcode)
		return 0;
	if (a->funct3 != b->funct3)
		return 0;
	if (a->funct7 != b->funct7)
		return 0;
	if (strcmp(a->name, b->name))
		return 0;

	return 1;
}

int test_parser(struct formation_t formation)
{
	struct formation_t found;
	if (parse_form(formation.name, &found)) {
		logger(ERROR, error_internal,
		       "Test Failed, parse_parser returned error code with instruction %s",
		       formation.name);
		return 1;
	}
	if (!compare_formations(&formation, &found)) {
		logger(ERROR, error_internal,
		       "Test Failed, parser for %s not correctly found",
		       formation.name);
		return 1;
	}
	return 0;
}

int main(void)
{
	set_exit_loglevel(NODEBUG);
	set_min_loglevel(DEBUG);
	int errors = 0;
	for (size_t i = 0; rv64i[i].name; i++)
		errors += test_parser(rv64i[i]);
	return errors != 0 || get_clean_exit(ERROR);
}
