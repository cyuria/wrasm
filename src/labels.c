
#include "labels.h"

#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "output.h"

static int labelcount = 0;
static struct label_t *labels = NULL;

const char *get_label_name(int id) {
  if (id >= labelcount || id < 0)
    return NULL;
  return labels[id].name;
}
struct sectionpos_t get_label_position(int id) {
  if (id >= labelcount || id < 0) {
    logger(INFO, error_internal, "Unregistered label position fetched as 0");
    return (struct sectionpos_t){0, -1};
  }
  return labels[id].position;
}

int create_label(const char *name) {
  struct label_t *ptr = realloc(labels, (labelcount + 1) * sizeof(*labels));
  if (ptr == NULL)
    return -1;
  labels = ptr;

  char *n = malloc(strlen(name) + 1);
  if (n == NULL)
    return -1;
  strcpy(n, name);

  logger(DEBUG, no_error, "Creating label named \"%s\"", name);

  labels[labelcount] = (struct label_t){n};
  labelcount++;
  return labelcount - 1;

}

void set_labelpos(const int id, struct sectionpos_t position) {
  if (id >= labelcount || id < 0)
    return;
  labels[id].position = position;
}
int get_label_by_name(const char *name) {
  logger(DEBUG, no_error, "Searching for label with name \"%s\"", name);
  for (int i = 0; i < labelcount; i++) {
    if (!strcmp(labels[i].name, name)) {
      logger(DEBUG, no_error, "Label found (0x%x)", i);
      return i;
    }
  }
  logger(DEBUG, no_error, "Label not found");
  return -1;
}

void free_labels(void) {
  for (int i = 0; i < labelcount; i++)
    free(labels[i].name);
  free(labels);
}
