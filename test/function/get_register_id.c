
#include <stdio.h>

struct {
  const char *symbol;
  const int value;
} tests[] = {
    {"x0", 0},   {"x1", 1},   {"x10", 10},   {"x20", 20},         {"x30", 30},
    {"x31", 31}, {"x32", -1}, {"x3000", -1}, {"x4400000000", -1}, {"fp", 8},
    {"s0", 8},   {"zero", 0}, {"sp", 2},     {"ra", 1},           {"a3", 13},
    {"s2", 18},  {"a8", -1},  {"t2", 7},     {"t3", 28},
};

int main(void) {
  int errors = 0;
  for (size_t i = 0; i < sizeof(tests) / sizeof(*tests); i++) {
    int imm = get_register_id(tests[i].symbol);
    if (imm != tests[i].value) {
      logger(ERROR, error_internal,
             "Test Failed, expected \"%s\" to equal %d but was given %d",
             tests[i].symbol, tests[i].value, imm);
      errors++;
    }
  }
  if (errors)
    logger(CRITICAL, error_internal, "%d tests failed", errors);
  return (bool)errors;
}
