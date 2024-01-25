#pragma once

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define NO_ERROR                  0x00
#define ERROR_INVALID_INSTRUCTION 0x01
#define ERROR_INSTRUCTION_OTHER   0x02
#define ERROR_NOT_IMPLEMENTED     0x03
#define ERROR_INTERNAL            0x04
#define ERROR_SYSTEM              0x05
#define ERROR_UNKNOWN             0x06
#define ERROR_OTHER               0x07

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
extern const char* error_ids[];

enum loglvl_t {
  DEBUG,
  INFO,
  WARN,
  ERROR,
  CRITICAL,
  NONE,
};

void set_min_loglevel(enum loglvl_t);
void logger(enum loglvl_t, int, int, const char*, ...);

