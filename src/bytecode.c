
#include "bytecode.h"

#include <stddef.h>
#include <stdlib.h>

#include "debug.h"
#include "elf/output.h"
#include "form/generic.h"
#include "xmalloc.h"

static struct instruction *instructions = NULL;
static size_t instructions_size = 0;
static struct rawdata *dataitems = NULL;
static size_t dataitems_size = 0;

int add_instruction(struct instruction instruction)
{
	const size_t sz = instructions_size + 1;
	instructions = xrealloc(instructions, sz * sizeof(*instructions));
	instructions[instructions_size] = instruction;
	instructions_size = sz;

	return 0;
}

int add_data(struct rawdata dataitem)
{
	const size_t sz = dataitems_size + 1;
	struct rawdata *newdataarr =
		xrealloc(dataitems, sz * sizeof(*dataitems));

	if (newdataarr == NULL) {
		logger(ERROR, error_internal, 0,
		       "Unable to allocate memory for label instruction");
		return 1;
	}

	dataitems = newdataarr;
	dataitems[dataitems_size] = dataitem;
	dataitems_size = sz;

	return 0;
}

int write_all(void)
{
	if (write_all_instructions())
		return 1;
	if (write_all_data())
		return 1;
	return 0;
}

int write_all_instructions(void)
{
	linenumber = 0;
	logger(DEBUG, no_error, "Generating all instruction bytecode...");
	for (size_t i = 0; i < instructions_size; i++)
		if (write_instruction(instructions[i]))
			return 1;
	return 0;
}

int write_instruction(struct instruction i)
{
	linenumber = i.line;
	logger(DEBUG, no_error,
	       "Generating bytecode for %s instruction (offset: %zu)",
	       i.formation.name, i.position.offset);
	set_section(i.position.section);
	struct bytecode bytecode =
		i.formation.form_handler(i.formation.name, i.formation.idata,
					 i.args, calc_fileoffset(i.position));
	logger(DEBUG, no_error, "Bytecode finished generating");
	if (!bytecode.size) {
		logger(WARN, no_error,
		       "No bytecode generated from instruction");
		return 0;
	}
	if (!bytecode.data) {
		logger(ERROR, error_internal, "Received invalid bytecode");
		return 1;
	}
	const size_t sz = (size_t)bytecode.size;
	size_t nwritten =
		write_sectiondata((char *)bytecode.data, sz, i.position);
	free(bytecode.data);
	if (nwritten != sz) {
		logger(CRITICAL, error_system, "Error writing bytes to output");
		return 1;
	}
	return 0;
}

int write_all_data(void)
{
	linenumber = 0;
	logger(DEBUG, no_error, "Writing all data bytes...");
	for (size_t i = 0; i < dataitems_size; i++)
		if (write_data(dataitems[i]))
			return 1;
	return 0;
}

int write_data(struct rawdata data)
{
	linenumber = data.line;
	logger(DEBUG, no_error, "Writing data (offset: %zu)",
	       data.position.offset);
	set_section(data.position.section);
	const size_t written =
		write_sectiondata(data.data, data.size, data.position);
	free(data.data);
	if (written != data.size) {
		logger(CRITICAL, error_system, "Error writing bytes to output");
		return 1;
	}
	return 0;
}

void free_instructions(void)
{
	free(instructions);
	instructions = NULL;
	instructions_size = 0;
}

void free_data(void)
{
	free(dataitems);
	dataitems = NULL;
	dataitems_size = 0;
}
