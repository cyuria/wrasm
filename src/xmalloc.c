
#include "xmalloc.h"

#include <stdlib.h>

#include "debug.h"

#define ABORTFUNC(function)                                                    \
  {                                                                            \
    logger(CRITICAL, error_system,                                             \
           "Call to stdlib function " function " failed");                     \
    abort();                                                                   \
  }

void *xmalloc(size_t sz) {
  void *ptr = malloc(sz);
  if (!ptr)
    ABORTFUNC("malloc");
  return ptr;
}

void *xcalloc(size_t nitems, size_t size) {
  void *ptr = calloc(nitems, size);
  if (!ptr)
    ABORTFUNC("calloc");
  return ptr;
}

void *xrealloc(void *ptr, size_t size) {
  ptr = realloc(ptr, size);
  if (!ptr)
    ABORTFUNC("xrealloc");
  return ptr;
}
