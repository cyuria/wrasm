#pragma once
#include <stdint.h>

#ifndef RISCV_OPCODES
#define RISCV_OPCODES

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

#endif

struct iarg {
  enum { reg, val } type;
  union {
    int reg;
    uint32_t val;
  };
};

struct instruction_t
{
  const char* name;
  enum {
    R_TYPE,
    I_TYPE,
    S_TYPE,
    B_TYPE,
    U_TYPE,
    J_TYPE,
    FENCE_TYPE,
    FLAT_TYPE,
  } type;
  const uint8_t opcode;
  const uint8_t funct1;
  const uint16_t funct2;
};

/* instruction sets */
extern const struct instruction_t rv64i[];
extern const struct instruction_t rv64m[];
extern const struct instruction_t rv64a[];
extern const struct instruction_t rv64c[];
extern const struct instruction_t rv64z[]; /* zifencei and zicsr extensions */

/* basic instruction type generation */
uint32_t gen_rtype(struct instruction_t inst, int rd, int rs1, int rs2);
uint32_t gen_itype(struct instruction_t inst, int rd, int rs1, uint32_t imm);
uint32_t gen_stype(struct instruction_t inst, int rs1, int rs2, uint32_t imm);
uint32_t gen_btype(struct instruction_t inst, int rs1, int rs2, uint32_t imm);
uint32_t gen_utype(struct instruction_t inst, int rd, uint32_t imm);
uint32_t gen_jtype(struct instruction_t inst, int rd, uint32_t imm);
uint32_t gen_fencetype(struct instruction_t inst, uint8_t flags);

