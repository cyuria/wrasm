#pragma once

extern const char *reg_abi_map[];
extern const char *float_reg_abi_map[];

int get_register_id(const char *);
int get_float_register_id(const char *);

int get_immediate(const char *, int *);

