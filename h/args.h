
#include <argp.h>

extern const char* argp_program_version;
extern const char usagestr[];
extern const char docstr[];

extern const struct argp_option options[];

struct arguments
{
  int verbose;
  char* args[2];
  char* argone;
  char* argtwo;
};

extern const struct argp argp_config;

error_t parseArgs(int, char*, struct argp_state*);

struct arguments new_arguments(void);

