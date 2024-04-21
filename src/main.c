
#define __STDC_WANT_LIB_EXT1__ 1

#include <stdio.h>
#include <stdlib.h>

#include "args.h"
#include "debug.h"
#include "generation.h"

FILE *inputfile = NULL;
FILE *outputtempfile = NULL;
FILE *outputfile = NULL;

void closefiles(void) {
  if (inputfile)
    fclose(inputfile);
  if (outputtempfile)
    fclose(outputtempfile);
  if (outputfile && outputfile != stdin && outputfile != stdout &&
      outputfile != stderr)
    fclose(outputfile);
}

void open_files(void) {
  logger(DEBUG, no_error, "Opening files");

  outputtempfile = tmpfile();
  if (outputtempfile == NULL) {
    logger(ERROR, error_system, "Unable to create temporary file");
    exit(EXIT_FAILURE);
  }

#ifdef __STDC_LIB_EXT1__
  if (fopen_s(&inputfile, *cmdargs.inputfile->filename, "r"))
#else
  inputfile = fopen(*cmdargs.inputfile->filename, "r");
  if (inputfile == NULL)
#endif
  {
    logger(ERROR, error_system, "Unable to open input file");
    exit(EXIT_FAILURE);
  }

  if (!**cmdargs.outputfile->filename) {
    outputfile = stdout;
    return;
  }

#ifdef __STDC_LIB_EXT1__
  if (fopen_s(&outputfile, *cmdargs.outputfile->filename, "wb"))
#else
  outputfile = fopen(*cmdargs.outputfile->filename, "wb");
  if (!outputfile)
#endif
  {
    logger(ERROR, error_system, "Unable to open output file");
    perror("Error: ");
    exit(EXIT_FAILURE);
  }
}

void copy_files(FILE *dest, FILE *src) {
  const long pos = ftell(src);
  fseek(src, 0L, SEEK_END);
  size_t sz = ftell(outputtempfile);
  rewind(outputtempfile);

  char *buffer = malloc(BUFSIZ);
  while (sz > BUFSIZ) {
    fread(buffer, 1, BUFSIZ, src);
    fwrite(buffer, 1, BUFSIZ, dest);
    sz -= BUFSIZ;
  }
  buffer = realloc(buffer, sz);
  fread(buffer, 1, sz, src);
  fwrite(buffer, 1, sz, dest);
  free(buffer);

  fseek(src, pos, SEEK_SET);
}

int main(int argc, char *argv[]) {
  parse_cmdargs(argc, argv);
  open_files();
  parse_file(inputfile, outputtempfile);

  logger(DEBUG, no_error, "Done generating bytecode");
  copy_files(outputfile, outputtempfile);
  logger(DEBUG, no_error, "Finished writing bytecode to output");
  closefiles();
}
