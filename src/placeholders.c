
#include "placeholders.h"

#include <stdlib.h>

#include "debug.h"

struct placeholder_t *placeholders = NULL;
int placeholders_size = 0;

int add_placeholder(struct placeholder_t placeholder) {
  const size_t sz = placeholders_size + 1;
  struct placeholder_t *newplaceholderarr =
      realloc(placeholders, sz * sizeof(*placeholders));

  if (unlikely(newplaceholderarr == NULL)) {
    logger(ERROR, error_internal, 0,
           "Unable to allocate memory for label instruction");
    return 1;
  }

  placeholders = newplaceholderarr;
  placeholders[placeholders_size] = placeholder;
  placeholders_size = sz;

  return 0;
}

void free_placeholders(void) {
  if (placeholders == NULL)
    return;
  free(placeholders);
}
