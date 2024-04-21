
#include "xmalloc.h"

#include <stdlib.h>

#include "debug.h"

void *xmalloc(size_t sz) {
  void *ptr = malloc(sz);
  if (!ptr) {
    logger(CRITICAL, error_system, "Call to stdlib function malloc failed");
    exit(EXIT_FAILURE);
  }
  return ptr;
}
