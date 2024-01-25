
#include "labels.h"

#include <stdlib.h>
#include <string.h>

#include "debug.h"

static int labelcount = 0;
static struct label_t *labels = NULL;

const char *get_label_name(int id) {
  if (id >= labelcount || id < 0)
    return NULL;
  return labels[id].name;
}
int get_label_position(int id) {
  if (id >= labelcount || id < 0) {
    logger(INFO, ERROR_INTERNAL, 0, "Unregistered label position fetched as 0");
    return -1;
  }
  return labels[id].position;
}

int create_label(const char *name, int position) {
  struct label_t *ptr = realloc(labels, (labelcount + 1) * sizeof(*labels));
  if (ptr == NULL)
    return -1;
  labels = ptr;

  char *n = malloc(strlen(name) + 1);
  if (n == NULL)
    return -1;
  strcpy(n, name);

  logger(DEBUG, NO_ERROR, 0, "Creating label named \"%s\"", name);

  labels[labelcount] = (struct label_t){n, position};
  labelcount++;
  return labelcount - 1;
}
int get_label_by_name(const char *name) {
  logger(DEBUG, NO_ERROR, 0, "Searching for label with name \"%s\"", name);
  for (int i = 0; i < labelcount; i++) {
    if (!strcmp(labels[i].name, name)) {
      logger(DEBUG, NO_ERROR, 0, "Label found (0x%x)", i);
      return i;
    }
  }
  logger(DEBUG, NO_ERROR, 0, "Label not found");
  return -1;
}

void free_labels(void) {
  for (int i = 0; i < labelcount; i++)
    free(labels[i].name);
  free(labels);
}
