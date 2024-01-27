
#include <stdio.h>
#include <stdlib.h>

#include "args.h"
#include "debug.h"
#include "generation.h"

int main(int argc, char* argv[])
{
  struct cmdargs_t args = new_arguments();
  argp_parse(&argp_config, argc, argv, 0, 0, &args);

  if (args.input == NULL) {
    logger(ERROR, error_other, 0, "No input file specified");
    exit(EXIT_FAILURE);
  }

  FILE* ifp = fopen(args.input, "r");
  if (ifp == NULL) {
    logger(ERROR, error_system, 0, "Unable to open input file");
    return EXIT_FAILURE;
  }
  FILE* ofp = fopen(args.output, "wb");
  if (ofp == NULL) {
    logger(ERROR, error_system, 0, "Unable to open output file");
    return EXIT_FAILURE;
  }

  logger(DEBUG, no_error, 0, "Files opened");

  parse_file(ifp, ofp);

  logger(DEBUG, no_error, 0, "Done generating output");

  fclose(ofp);
  fclose(ifp);

  logger(DEBUG, no_error, 0, "Files closed");

}

