
#include "instructions.h"

#include <stddef.h>
#include <stdlib.h>

#include "debug.h"
#include "generation.h"
#include "output.h"

static struct instruction_t *instructions = NULL;
static size_t instructions_size = 0;

int add_instruction(struct instruction_t instruction) {
  const size_t sz = instructions_size + 1;
  struct instruction_t *newinstructionarr =
      realloc(instructions, sz * sizeof(*instructions));

  if (unlikely(newinstructionarr == NULL)) {
    logger(ERROR, error_internal, 0,
           "Unable to allocate memory for label instruction");
    return 1;
  }

  instructions = newinstructionarr;
  instructions[instructions_size] = instruction;
  instructions_size = sz;

  return 0;
}

int write_all_instructions(void) {
  logger(DEBUG, no_error, "Generating all bytecode...");
  for (size_t i = 0; i < instructions_size; i++)
    if (write_instruction(instructions[i]))
      return 1;
  return 0;
}

int write_instruction(struct instruction_t instruction) {
  linenumber = instruction.line;
  logger(DEBUG, no_error, "Generating bytecode for %s instruction (offset: %zu)",
         instruction.parser.name, instruction.position.offset);
  set_outputpos(instruction.position);
  struct bytecode_t *bytecode =
      instruction.parser.handler(instruction.parser, instruction.args,
                                 calc_fileoffset(instruction.position));
  return write_bytecode(bytecode, instruction.position) == bytecode->size;
}

void free_instructions(void) {
  free(instructions);
  instructions = NULL;
}
