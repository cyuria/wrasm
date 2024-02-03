#pragma once

#include "output.h"

struct label_t {
  char *name;
  struct sectionpos_t position;
};

const char* get_label_name(int);
struct sectionpos_t get_label_position(int);

int create_label(const char *);
void set_labelpos(int, struct sectionpos_t);
int get_label_by_name(const char *);
