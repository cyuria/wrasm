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
    fclose(inputfile);
  if (outputfile)
    fclose(inputfile);
}

void open_files(void) {
  logger(DEBUG, no_error, 0, "Opening files");

  outputtempfile = tmpfile();
  if (outputtempfile == NULL) {
    logger(ERROR, error_system, 0, "Unable to create temporary file");
    exit(EXIT_FAILURE);
  }

  inputfile = fopen(*cmdargs.inputfile->filename, "r");
  if (inputfile == NULL) {
    logger(ERROR, error_system, 0, "Unable to open input file");
    exit(EXIT_FAILURE);
  }

  if (**cmdargs.outputfile->filename) {
    outputfile = fopen(*cmdargs.outputfile->filename, "wb");
    if (outputfile == NULL) {
      logger(ERROR, error_system, 0, "Unable to open output file");
      perror("Error: ");
      exit(EXIT_FAILURE);
    }
  } else {
    outputfile = stdout;
  }
}

void copy_files(FILE *dest, FILE *src) {
  const size_t pos = ftell(src);
  fseek(src, 0L, SEEK_END);
  size_t sz = ftell(outputtempfile);
  rewind(outputtempfile);

  while (sz > BUFSIZ) {
    char buffer[BUFSIZ];
    fread(buffer, 1, sizeof(buffer), src);
    fwrite(buffer, 1, sizeof(buffer), dest);
    sz -= BUFSIZ;
  }

  char buffer[sz];
  fread(buffer, 1, sizeof(buffer), src);
  fwrite(buffer, 1, sizeof(buffer), dest);

  fseek(src, pos, SEEK_SET);
}

int main(int argc, char *argv[]) {

  parse_args(argc, argv);
  open_files();
  parse_file(inputfile, outputtempfile);

  logger(DEBUG, no_error, 0, "Done generating bytecode");
  copy_files(outputfile, outputtempfile);
  logger(DEBUG, no_error, 0, "Finished writing bytecode to output");
}
