#include "args.h"
#include "debug.h"

#include <argp-standalone/argp.h>
#include <stdio.h>

const char *argp_program_version = "wrasm 0.0.1 alpha";
const char usagestr[] = "INPUT";
const char docstr[] =
    "The wrasm assembler\n"
    "\n"
    "A risc-V macro assembler based on nasm (see https://nasm.us/)\n"
    "Currently still being built\n";

const struct argp_option options[] = {
  { "verbose", 'v', 0, 0, "Produce verbose output", },
  { "output", 'o', "OUTPUT", 0, "The output file. Leave blank to automatically generate based off of input", },
  { 0 }
};

const struct argp argp_config = {options, parseArgs, usagestr, docstr};

error_t parseArgs(int key, char *arg, struct argp_state *state) {
  struct cmdargs_t *args = state->input;

  switch (key) {
  case 'v':
    args->verbose = 1;
    set_min_loglevel(DEBUG);
    return 0;
  case 'o':
    args->output = arg;
    return 0;
  case ARGP_KEY_ARG:
    if (state->arg_num > 1) {
      logger(ERROR, error_other, 0, "Multiple input files specified");
      argp_usage(state);
    }
    args->input = arg;
    return 0;
  default:
    return ARGP_ERR_UNKNOWN;
  }
}

struct cmdargs_t new_arguments(void) {
  struct cmdargs_t args;

  args.verbose = 0;
  args.input = NULL;
  args.output = NULL;

  return args;
}
