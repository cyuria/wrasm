
#include <stdio.h>
#include "args.h"

#define println(str) printf("%s\n", str)

int main(int argc, char* argv[])
{
  struct arguments args = new_arguments();
  argp_parse(&argp_config, argc, argv, 0, 0, &args);

  if (args.verbose)
    println("Verbose mode enabled");

  printf("argone: %s\n", args.argone);
  printf("argtwo: %s\n", args.argtwo);

  println("Hello, World!");
}

