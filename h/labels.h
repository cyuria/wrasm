#pragma once

struct label_t {
  char *name;
  int position;
};

const char* get_label_name(int);
int get_label_position(int);

int create_label(const char *, int);
int get_label_by_name(const char *);
