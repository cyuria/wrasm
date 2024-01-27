#pragma once

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/* ERROR IDs
 *
 * 0x00 No error
 * 0x01 Invalid instruction
 * 0x02 Not implemented
 * 0x03 Internal error
 * 0x04 System error
 * 0x05 Unknown error
 * 0x06 Other error
 */
extern const char *error_ids[];
enum error_t {
  no_error,
  error_invalid_instruction,
  error_instruction_other,
  error_not_implemented,
  error_internal,
  error_system,
  error_unknown,
  error_other,
};

enum loglvl_t {
  DEBUG,
  INFO,
  WARN,
  ERROR,
  CRITICAL,
  NONE,
};

void set_min_loglevel(enum loglvl_t);
void logger(enum loglvl_t, int, int, const char *, ...);
