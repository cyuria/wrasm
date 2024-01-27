
#include "registers.h"
#include "debug.h"

#include <stdlib.h>
#include <string.h>

/* TODO: fix fp alias of s0 */
#define BASE_REG_COUNT 32
const char *reg_abi_map[BASE_REG_COUNT] = {
    "zero", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
    "a1",   "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
    "s6",   "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6",
};

/* TODO: implement w/ float extension */
#define FLOAT_REG_COUNT 0
const char *float_reg_abi_map[FLOAT_REG_COUNT] = {};

int get_register_id(const char *reg) {
  logger(DEBUG, no_error, 0, "Searching for register (%s)", reg);

  if (*reg == 'x')
    return atoi(reg + 1);

  for (int i = 0; i < BASE_REG_COUNT; i++)
    if (!strcmp(reg, reg_abi_map[i]))
      return i;

  return -1;
}

int get_float_register_id(const char *reg) {
  if (*reg != 'f')
    return -1;

  if (reg[1] >= '0' && reg[1] <= '9')
    return atoi(reg + 1);

  for (int i = 0; i < FLOAT_REG_COUNT; i++)
    if (!strcmp(reg, float_reg_abi_map[i]))
      return i;

  return -1;
}

int calc_digit(char digit) {
  if (digit > 'z') return -1;
  if (digit >= 'a') return digit - 'a' + 10;
  if (digit > 'Z') return -1;
  if (digit >= 'A') return digit - 'A' + 10;
  if (digit > '9') return -1;
  return digit - '0';
}

int get_immediate(const char *imm, int *res) {
  int base = 10;
  if (!strncmp(imm, "0x", 2)) {
    base = 16;
    imm += 2;
  } else if (!strncmp(imm, "0b", 2)) {
    base = 2;
    imm += 2;
  }

  *res = strtol(imm, NULL, base);

  /* We know strtol didn't fail */
  if (*res) return 0;

  /* check for anything that could've caused a failure
   * NOTE: "-0" will cause a fail */
  while (*imm) {
    int digit = calc_digit(*imm);
    if (digit >= base || digit < 0)
      return 1;
    imm++;
  }

  return 0;
}
