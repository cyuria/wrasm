#include "args.h"
#include "argtable3.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>

const char progname[] = "wrasm";
const char *progcall = NULL;
const struct versioninfo_t versioninfo = {0, 0, 1, "alpha"};
const char helpstr[] =
    "The wrasm assembler\n"
    "\n"
    "A risc-V macro assembler based on nasm (see https://nasm.us/)\n"
    "Currently still being built\n";

struct cmdargs_t cmdargs;

void parse_cmdargs(int argc, char *argv[]) {
  const char *progcall = argv[0];

  void *argtable[] = {
      cmdargs.help = arg_litn("h", "help", 0, 1, "display this help and exit"),
      cmdargs.version =
          arg_litn("V", "version", 0, 1, "display version info and exit"),
      cmdargs.verbose = arg_litn("v", "verbose", 0, 1, "verbose output"),
      cmdargs.inputfile = arg_file1(NULL, NULL, "<input>", "input file"),
      cmdargs.outputfile = arg_file0("o", "output", "<filename>",
                                     "output file, leave blank for stdout"),
      cmdargs.end = arg_end(20)};

  int nerrors = arg_parse(argc, argv, argtable);

  if (cmdargs.help->count) {
    printf("Usage: %s", progcall);
    arg_print_syntax(stdout, argtable, "\n");
    puts(helpstr);
    arg_print_glossary(stdout, argtable, "  %-25s %s\n");
    arg_freetable(argtable, sizeof(argtable) / sizeof(*argtable));
    exit(EXIT_SUCCESS);
  }

  if (cmdargs.version->count) {
    printf("%s version %d.%d.%d %s\n", progname, versioninfo.major,
           versioninfo.minor, versioninfo.patch, versioninfo.note);
    arg_freetable(argtable, sizeof(argtable) / sizeof(*argtable));
    exit(EXIT_SUCCESS);
  }

  if (nerrors) {
    arg_print_errors(stdout, cmdargs.end, progname);
    printf("Try '%s --help' for more information\n", progcall);
    arg_freetable(argtable, sizeof(argtable) / sizeof(*argtable));
    exit(EXIT_FAILURE);
  }

  if (cmdargs.verbose->count) {
    set_min_loglevel(DEBUG);
  }

  arg_freetable(argtable, sizeof(argtable) / sizeof(*argtable));
}
