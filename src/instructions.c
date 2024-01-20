
#include "instructions.h"

/* TODO: Add HINT instruction support */
const struct instruction_t rv64i[] = {
  { "ADD",   R_TYPE, OP_OP,    0b000, 0x00 },
  { "ADDI",  I_TYPE, OP_OPI,   0b000 },
  { "ADDW",  R_TYPE, OP_OP32,  0b0 }, /* TODO: */
  { "ADDIW", I_TYPE, OP_OPI32, 0b0 }, /* TODO: */
  { "SUB",   R_TYPE, OP_OP,    0b000, 0x20 },
  { "SUBW",  R_TYPE, OP_OP32,  0b0 }, /* TODO: */
  { "AND",   R_TYPE, OP_OP,    0b111, 0x00 },
  { "ANDI",  I_TYPE, OP_OPI,   0b111 },
  { "OR",    R_TYPE, OP_OP,    0b110, 0x00 },
  { "ORI",   I_TYPE, OP_OPI,   0b110 },
  { "XOR",   R_TYPE, OP_OP,    0b100, 0x00 },
  { "XORI",  I_TYPE, OP_OPI,   0b100 },
  { "SLL",   R_TYPE, OP_OP,    0b001, 0x00 },
  { "SLLI",  I_TYPE, OP_OPI,   0b001, 0x00 },
  { "SLLW",  R_TYPE, OP_OP32,  }, /* TODO: */
  { "SLLIW", I_TYPE, OP_OPI32, 0b0 }, /* TODO: */
  { "SRL",   R_TYPE, OP_OP,    0b101, 0x00 },
  { "SRLI",  I_TYPE, OP_OPI,   0b101, 0x00 },
  { "SRLW",  R_TYPE, OP_OP,    }, /* TODO: */
  { "SRLIW", I_TYPE, OP_OPI32, 0b0 }, /* TODO: */
  { "SRA",   R_TYPE, OP_OP,    0b101, 0x20 },
  { "SRAI",  I_TYPE, OP_OPI,   0b101, 0x20 },
  { "SRAW",  R_TYPE, OP_OP32,  }, /* TODO: */
  { "SRAIW", I_TYPE, OP_OPI32, 0b0 }, /* TODO: */
  { "SLT",   R_TYPE, OP_OP,    0b010, 0x00 },
  { "SLTI",  I_TYPE, OP_OPI,   0b010 },
  { "SLTU",  R_TYPE, OP_OP,    }, /* TODO: */
  { "SLTIU", I_TYPE, OP_OPI,   0b011 },

  { "BEQ",  B_TYPE, OP_BRANCH, 0b000 },
  { "BNE",  B_TYPE, OP_BRANCH, 0b001 },
  { "BGE",  B_TYPE, OP_BRANCH, 0b101 },
  { "BGEU", B_TYPE, OP_BRANCH, 0b111 },
  { "BLT",  B_TYPE, OP_BRANCH, 0b100 },
  { "BLTU", B_TYPE, OP_BRANCH, 0b110 },
  { "JAL",  J_TYPE, OP_JAL           },
  { "JALR", I_TYPE, OP_JALR,   0b000 },

  { "ECALL",  I_TYPE, OP_SYSTEM, 0b000, 0x000 },
  { "EBREAK", I_TYPE, OP_SYSTEM, 0b000, 0x001 },

  { "LB",  I_TYPE, OP_LOAD, 0b000 },
  { "LH",  I_TYPE, OP_LOAD, 0b001 },
  { "LW",  I_TYPE, OP_LOAD, 0b010 },
  { "LD",  I_TYPE, OP_LOAD, 0b011 },
  { "LBU", I_TYPE, OP_LOAD, 0b100 },
  { "LHU", I_TYPE, OP_LOAD, 0b101 },
  { "LWU", I_TYPE, OP_LOAD, 0b110 },

  { "SB",  S_TYPE, OP_STORE, 0b000 },
  { "SH",  S_TYPE, OP_STORE, 0b001 },
  { "SW",  S_TYPE, OP_STORE, 0b010 },
  { "SD",  S_TYPE, OP_STORE, 0b011 },

  { "LUI",   U_TYPE, OP_LUI },
  { "AUIPC", U_TYPE, OP_AUIPC },

  { "FENCE",  FENCE_TYPE, OP_MISC_MEM, 0b000 },
  // { "FENCEI", I_TYPE,     OP_MISC_MEM, 0b001 }, /* zifencei standard extension */

  { "NOP", FLAT_TYPE, OP_OPI, 0x00, 0x0000 }, /* TODO: encoded as ADDI x0, x0, 0 */
};


uint32_t gen_rtype(struct instruction_t inst, int rd, int rs1, int rs2)
{
  return (uint32_t)(
    inst.opcode 
    | (rd << 7) | (rs1 << 15) | (rs2 << 20)
    | (inst.funct1 << 12) | (inst.funct2 << 25)
  );
}

uint32_t gen_itype(struct instruction_t inst, int rd, int rs1, uint32_t imm)
{
  return (uint32_t)(
    inst.opcode 
    | (rd << 7) | (rs1 << 15)
    | (inst.funct1 << 12)
    | ((imm & 0x7FF) << 22)
  );
}

uint32_t gen_stype(struct instruction_t inst, int rs1, int rs2, uint32_t imm)
{
  return (uint32_t)(
    inst.opcode 
    | (rs1 << 15) | (rs2 << 20)
    | (inst.funct1 << 12)
    | ((imm & 0x1F) << 7) | ((imm & 0xFE0) << 21)
  );
}

uint32_t gen_btype(struct instruction_t inst, int rs1, int rs2, uint32_t imm)
{
  const uint32_t dup = imm >> 1;
  imm &= 0x7FE;
  imm |= (dup >> 10) & 0x1;
  imm |= dup & 0x800;
  return gen_stype(inst, rs1, rs2, imm);
}

uint32_t gen_utype(struct instruction_t inst, int rd, uint32_t imm)
{
  return (uint32_t)(
    inst.opcode 
    | (rd << 7)
    | (imm & 0xFFFFF000)
  );
}

uint32_t gen_jtype(struct instruction_t inst, int rd, uint32_t imm)
{
  const uint32_t a = (imm & 0x00007FE) << 20;
  const uint32_t b = (imm & 0x0000800) << 9;
  const uint32_t c = (imm & 0x0FFF000);
  const uint32_t d = (imm & 0x1000000) << 11;

  return gen_utype(inst, rd, a | b | c | d);
}

uint32_t gen_fencetype(struct instruction_t inst, uint8_t flags)
{
  return (
    inst.opcode
    | (inst.funct1 << 12)
    | (((uint32_t) flags) << 20)
  );
}

