
#include <string.h>

#include "form/instructions.h"
#include "form/rv64i.h"
#include "debug.h"

int compare_formations(struct formation_t *a, struct formation_t *b)
{
	if (a->arg_handler != b->arg_handler)
		return 0;
	if (a->form_handler != b->form_handler)
		return 0;
	if (a->idata.sz != b->idata.sz)
		return 0;
	if (a->idata.opcode != b->idata.opcode)
		return 0;
	if (a->idata.funct3 != b->idata.funct3)
		return 0;
	if (a->idata.funct7 != b->idata.funct7)
		return 0;
	if (strcmp(a->name, b->name))
		return 0;

	return 1;
}

int test_parser(struct formation_t formation)
{
	struct formation_t found = parse_form(formation.name);
	if (!found.name) {
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
