#pragma once
#include <stdint.h>
#include <stdlib.h>

#define OP_LOAD 0x03
#define OP_STORE 0x23
#define OP_BRANCH 0x63
#define OP_JAL 0x6F
#define OP_JALR 0x67
#define OP_MISC_MEM 0x0F
#define OP_OP 0x33
#define OP_OPI 0x13
#define OP_OP32 0x3B
#define OP_OPI32 0x1B
#define OP_SYSTEM 0x73
#define OP_LUI 0x37
#define OP_AUIPC 0x17

#define RV64I_SIZE 2
#define RV64M_SIZE 2
#define RV64A_SIZE 2
#define RV64F_SIZE 2
#define RV64D_SIZE 2
#define RV64C_SIZE 1
#define RV64Z_SIZE 2

enum argtype_t {
  ERROR_ARGTYPE,
  R_TYPE,
  ISB_TYPE,
  UJ_TYPE,
  NOARGS_TYPE,
};

struct args_t {
  enum argtype_t type;
  union {
    struct {
      uint8_t rd;
      uint8_t rs1;
      uint8_t rs2;
    } r;
    struct {
      uint8_t r1;
      uint8_t r2;
      uint16_t imm;
    } isb;
    struct {
      uint8_t rd;
      uint32_t imm;
    } uj;
  };
};

struct bytecode_t {
  size_t size;
  uint16_t *data;
};

struct parser_t {
  const char *name;
  enum argtype_t argtype;
  struct bytecode_t (*handler)(struct parser_t, struct args_t, size_t);
  uint8_t opcode;
  uint8_t funct1;
  uint16_t funct2;
};

extern struct bytecode_t error_bytecode;

/* parser sets */
extern const struct parser_t rv64s[]; /* Shortcuts */
extern const struct parser_t rv64i[];
extern const struct parser_t rv64m[];
extern const struct parser_t rv64a[];
extern const struct parser_t rv64f[];
extern const struct parser_t rv64d[];
extern const struct parser_t rv64c[];
extern const struct parser_t rv64z[]; /* zifencei and zicsr extensions */

struct bytecode_t gen_empty_bytecode(void);

/* basic integer instruction type bytecode generation */
struct bytecode_t gen_rtype(struct parser_t, struct args_t, size_t);
struct bytecode_t gen_itype(struct parser_t, struct args_t, size_t);
struct bytecode_t gen_itype2(struct parser_t, struct args_t, size_t);
struct bytecode_t gen_stype(struct parser_t, struct args_t, size_t);
struct bytecode_t gen_btype(struct parser_t, struct args_t, size_t);
struct bytecode_t gen_utype(struct parser_t, struct args_t, size_t);
struct bytecode_t gen_jtype(struct parser_t, struct args_t, size_t);
struct bytecode_t gen_syscall(struct parser_t, struct args_t, size_t);
struct bytecode_t gen_fence(struct parser_t, struct args_t, size_t);
struct bytecode_t gen_nop(struct parser_t, struct args_t, size_t);
