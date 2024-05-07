#include "args.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>

#include <argtable3.h>

const char progname[] = "wrasm";
const struct versioninfo_t versioninfo = { 0, 0, 1, "alpha" };
const char helpstr[] =
	"The wrasm assembler\n"
	"\n"
	"A risc-V macro assembler based on nasm (see https://nasm.us/)\n"
	"Currently still being built\n";

struct cmdargs_t cmdargs;

void *argtable[6];
static void free_argtable(void);

void parse_cmdargs(int argc, char *argv[])
{
	const char *progcall = argv[0];

	argtable[0] = cmdargs.help =
		arg_litn("h", "help", 0, 1, "display this help and exit");
	argtable[1] = cmdargs.version =
		arg_litn("V", "version", 0, 1, "display version info and exit");
	argtable[2] = cmdargs.verbose =
		arg_litn("v", "verbose", 0, 1, "verbose output");
	argtable[3] = cmdargs.inputfile =
		arg_filen(NULL, NULL, "<input>", 1, 3, "input file");
	argtable[4] = cmdargs.outputfile =
		arg_filen("o", "output", "<filename>", 1, 3, "output file");
	argtable[5] = cmdargs.end = arg_end(20);

	atexit(&free_argtable);

	int nerrors = arg_parse(argc, argv, argtable);

	if (cmdargs.help->count) {
		printf("Usage: %s", progcall);
		arg_print_syntax(stdout, argtable, "\n");
		puts(helpstr);
		arg_print_glossary(stdout, argtable, "  %-25s %s\n");
		exit(EXIT_SUCCESS);
	}

	if (cmdargs.version->count) {
		printf("%s version %d.%d.%d %s\n", progname, versioninfo.major,
		       versioninfo.minor, versioninfo.patch, versioninfo.note);
		exit(EXIT_SUCCESS);
	}

	if (cmdargs.inputfile->count > 1 || cmdargs.outputfile->count > 1)
		nerrors++;

	if (nerrors) {
		arg_print_errors(stdout, cmdargs.end, progname);
		printf("Try '%s --help' for more information\n", progcall);
		exit(EXIT_FAILURE);
	}

	if (cmdargs.verbose->count) {
		set_min_loglevel(DEBUG);
	}
}

static void free_argtable(void)
{
	arg_freetable(argtable, sizeof(argtable) / sizeof(*argtable));
}
