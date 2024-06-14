#pragma once

#include <stdlib.h>
#include <stdint.h>

extern const char *reg_abi_map[];
extern const char *float_reg_abi_map[];

size_t get_register_id(const char *);
size_t get_float_register_id(const char *);

int get_immediate(const char *, size_t *);
uint16_t get_csr(const char *);
