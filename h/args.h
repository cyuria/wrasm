#pragma once

#include <argtable3.h>

struct versioninfo_t {
	int major, minor, patch;
	const char *note;
};

extern const char progname[];
extern const char helpstr[];
extern const struct versioninfo_t versioninfo;

struct cmdargs_t {
	struct arg_lit *help, *version;
	struct arg_lit *verbose;
	struct arg_file *inputfile, *outputfile;
	struct arg_end *end;
};
extern struct cmdargs_t cmdargs;

void parse_cmdargs(int argc, char *argv[]);
