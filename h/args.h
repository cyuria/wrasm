
#include <argp-standalone/argp.h>

extern const char *argp_program_version;
extern const char usagestr[];
extern const char docstr[];

extern const struct argp_option options[];

struct cmdargs_t {
  int verbose;
  char *args[2];
  char *input;
  char *output;
};

extern const struct argp argp_config;

error_t parseArgs(int, char *, struct argp_state *);

struct cmdargs_t new_arguments(void);

