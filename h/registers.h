#pragma once

#include <stdlib.h>

extern const char *reg_abi_map[];
extern const char *float_reg_abi_map[];

size_t get_register_id(const char *);
size_t get_float_register_id(const char *);

size_t get_immediate(const char *, size_t *);
