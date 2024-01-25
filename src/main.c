
#include <stdio.h>

#include "args.h"
#include "debug.h"
#include "generation.h"

int main(int argc, char* argv[])
{
  struct arguments_t args = new_arguments();
  argp_parse(&argp_config, argc, argv, 0, 0, &args);

  if (args.verbose)
  {
    set_min_loglevel(DEBUG);
  }

  printf("input: %s\n", args.argone);
  printf("output: %s\n", args.argtwo);

  FILE* ifp = fopen(args.argone, "r");
  FILE* ofp = fopen(args.argtwo, "wb");

  logger(DEBUG, NO_ERROR, 0, "Files opened");

  parse_file(ifp, ofp);

  logger(DEBUG, NO_ERROR, 0, "Done generating output");

  fclose(ofp);
  fclose(ifp);

  logger(DEBUG, NO_ERROR, 0, "Files closed");

}

