
#include "instructions.h"

#include <stdlib.h>

#include "debug.h"
#include "labels.h"
#include "placeholders.h"

/* TODO: instructions
 *
 * First pass: -> DONE:
 *  Add all label requiring instructions to barrier and insert placeholder
 *  instead. Use second pass.
 *
 * Seconds pass: -> TODO:
 *  No second pass, iterate over placeholder instructions and generate actual
 *  ones
 */

struct bytecode_t error_bytecode = {.size = -1};

/* TODO: Add HINT instruction support */
const struct instruction_t rv64i[] = {
    {"add", R_TYPE, &gen_rtype, OP_OP, 0b000, 0x00},
    {"addi", ISB_TYPE, &gen_itype, OP_OPI, 0b000},
    {"addw", R_TYPE, &gen_rtype, OP_OP32, 0b000, 0x00},
    {"addiw", ISB_TYPE, &gen_itype, OP_OPI32, 0b000},
    {"sub", R_TYPE, &gen_rtype, OP_OP, 0b000, 0x20},
    {"subw", R_TYPE, &gen_rtype, OP_OP32, 0b000, 0x20},
    {"and", R_TYPE, &gen_rtype, OP_OP, 0b111, 0x00},
    {"andi", ISB_TYPE, &gen_itype, OP_OPI, 0b111},
    {"or", R_TYPE, &gen_rtype, OP_OP, 0b110, 0x00},
    {"ori", ISB_TYPE, &gen_itype, OP_OPI, 0b110},
    {"xor", R_TYPE, &gen_rtype, OP_OP, 0b100, 0x00},
    {"xori", ISB_TYPE, &gen_itype, OP_OPI, 0b100},
    {"sll", R_TYPE, &gen_rtype, OP_OP, 0b001, 0x00},
    {"slli", ISB_TYPE, &gen_itype, OP_OPI, 0b001, 0x00},
    {"sllw", R_TYPE, &gen_rtype, OP_OP32, 0b001, 0x00},
    {"slliw", ISB_TYPE, &gen_itype, OP_OPI32, 0b000, 0x00},
    {"srl", R_TYPE, &gen_rtype, OP_OP, 0b101, 0x00},
    {"srli", ISB_TYPE, &gen_itype, OP_OPI, 0b101, 0x00},
    {"srlw", R_TYPE, &gen_rtype, OP_OP32, 0b101, 0x00},
    {"srliw", ISB_TYPE, &gen_itype, OP_OPI32, 0b101},
    {"sra", R_TYPE, &gen_rtype, OP_OP, 0b101, 0x20},
    {"srai", ISB_TYPE, &gen_itype2, OP_OPI, 0b101},
    {"sraw", R_TYPE, &gen_rtype, OP_OP32, 0b101, 0x20},
    {"sraiw", ISB_TYPE, &gen_itype2, OP_OPI32, 0b101},
    {"slt", R_TYPE, &gen_rtype, OP_OP, 0b010, 0x00},
    {"slti", ISB_TYPE, &gen_itype, OP_OPI, 0b010},
    {"sltu", R_TYPE, &gen_rtype, OP_OP, 0b011, 0x00},
    {"sltiu", ISB_TYPE, &gen_itype, OP_OPI, 0b011},

    {"beq", ISB_TYPE, &gen_btype, OP_BRANCH, 0b000},
    {"bne", ISB_TYPE, &gen_btype, OP_BRANCH, 0b001},
    {"bge", ISB_TYPE, &gen_btype, OP_BRANCH, 0b101},
    {"bgeu", ISB_TYPE, &gen_btype, OP_BRANCH, 0b111},
    {"blt", ISB_TYPE, &gen_btype, OP_BRANCH, 0b100},
    {"bltu", ISB_TYPE, &gen_btype, OP_BRANCH, 0b110},
    {"jal", UJ_TYPE, &gen_jtype, OP_JAL},
    {"jalr", ISB_TYPE, &gen_itype, OP_JALR, 0b000},

    {"ecall", ISB_TYPE, &gen_itype, OP_SYSTEM, 0b000, 0x000},
    {"ebreak", ISB_TYPE, &gen_itype, OP_SYSTEM, 0b000, 0x001},

    {"lb", ISB_TYPE, &gen_itype, OP_LOAD, 0b000},
    {"lh", ISB_TYPE, &gen_itype, OP_LOAD, 0b001},
    {"lw", ISB_TYPE, &gen_itype, OP_LOAD, 0b010},
    {"ld", ISB_TYPE, &gen_itype, OP_LOAD, 0b011},
    {"lbu", ISB_TYPE, &gen_itype, OP_LOAD, 0b100},
    {"lhu", ISB_TYPE, &gen_itype, OP_LOAD, 0b101},
    {"lwu", ISB_TYPE, &gen_itype, OP_LOAD, 0b110},

    {"sb", ISB_TYPE, &gen_stype, OP_STORE, 0b000},
    {"sh", ISB_TYPE, &gen_stype, OP_STORE, 0b001},
    {"sw", ISB_TYPE, &gen_stype, OP_STORE, 0b010},
    {"sd", ISB_TYPE, &gen_stype, OP_STORE, 0b011},

    {"lui", UJ_TYPE, &gen_utype, OP_LUI},
    {"auipc", UJ_TYPE, &gen_utype, OP_AUIPC},

    {"fence", ISB_TYPE, &gen_fence, OP_MISC_MEM, 0b000},
    /* zifencei standard extension */
    // { "FENCEI", ISB_TYPE, &gen_ifence, OP_MISC_MEM, 0b001 },

    {"nop", NOARGS_TYPE, &gen_nop, OP_OPI},

    {NULL, 0, NULL} /* Ending null terminator */
};

struct bytecode_t *gen_empty_bytecode(void) {
  logger(DEBUG, NO_ERROR, 0, "Generating empty instruction");
  struct bytecode_t *res = malloc(sizeof(*res) + 2 * sizeof(*res->data));
  if (!res)
    return NULL; /* if malloc fails */
  res->size = 0;
  return res;
}

struct bytecode_t *gen_rtype(struct instruction_t inst, struct args_t args,
                             int position) {
  logger(DEBUG, NO_ERROR, 0, "Generating R type instruction (%s)", inst.name);

  struct bytecode_t *res = malloc(sizeof(*res) + 2 * sizeof(*res->data));
  if (!res)
    return NULL;

  res->size = 2;
  *(uint32_t *)&(res->data) =
      (inst.opcode | (args.r.rd << 7) | (args.r.rs1 << 15) |
       (args.r.rs2 << 20) | (inst.funct1 << 12) | (inst.funct2 << 25));
  return res;
}

struct bytecode_t *gen_itype(struct instruction_t inst, struct args_t args,
                             int position) {
  logger(DEBUG, NO_ERROR, 0, "Generating I type instruction (%s)", inst.name);

  struct bytecode_t *res = malloc(sizeof(*res) + 2 * sizeof(*res->data));
  if (!res)
    return NULL;

  res->size = 2;
  *(uint32_t *)&(res->data) =
      (inst.opcode | (args.isb.r1 << 7) | (args.isb.r2 << 15) |
       (inst.funct1 << 12) | ((args.isb.imm & 0x7FF) << 22));
  return res;
}

struct bytecode_t *gen_itype2(struct instruction_t inst, struct args_t args,
                              int position) {
  logger(DEBUG, NO_ERROR, 0, "Generating I type 2 instruction (%s)", inst.name);
  struct bytecode_t *res = gen_itype(inst, args, position);
  *(uint32_t *)&(res->data) |= 0x40000000; /* set type 2 bit */
  return res;
}

struct bytecode_t *gen_stype(struct instruction_t inst, struct args_t args,
                             int position) {
  logger(DEBUG, NO_ERROR, 0, "Generating S type instruction (%s)", inst.name);

  struct bytecode_t *res = malloc(sizeof(*res) + 2 * sizeof(*res->data));
  if (!res)
    return NULL;

  res->size = 2;
  *(uint32_t *)&(res->data) =
      (inst.opcode | (args.isb.r1 << 15) | (args.isb.r2 << 20) |
       (inst.funct1 << 12) | ((args.isb.imm & 0x1F) << 7) |
       ((args.isb.imm & 0xFE0) << 21));
  return res;
}

struct bytecode_t *gen_btype(struct instruction_t inst, struct args_t args,
                             int position) {
  logger(DEBUG, NO_ERROR, 0, "Generating B type instruction (%s)", inst.name);

  const uint32_t dup = args.isb.imm >> 1;
  args.isb.imm &= 0x7FE;
  args.isb.imm |= (dup >> 10) & 0x1;
  args.isb.imm |= dup & 0x800;

  return gen_stype(inst, args, position);
}

struct bytecode_t *gen_utype(struct instruction_t inst, struct args_t args,
                             int position) {
  logger(DEBUG, NO_ERROR, 0, "Generating U type instruction (%s)", inst.name);

  struct bytecode_t *res = malloc(sizeof(*res) + 2 * sizeof(*res->data));
  if (!res)
    return &error_bytecode;

  res->size = 2;
  *(uint32_t *)&(res->data) =
      (inst.opcode | (args.uj.rd << 7) | (args.uj.imm & 0xFFFFF000));
  return res;
}

struct bytecode_t *gen_jtype(struct instruction_t inst, struct args_t args,
                             int position) {
  logger(DEBUG, NO_ERROR, 0, "Generating J type instruction (%s)", inst.name);

  int offset = args.uj.imm;
  if (unlikely(offset < 0))
    return NULL;
  offset -= position;
  logger(DEBUG, NO_ERROR, 0, "Offset of J type instruction is 0x%x", offset);

  const uint32_t a = (offset & 0x00007FE) << 20;
  const uint32_t b = (offset & 0x0000800) << 9;
  const uint32_t c = (offset & 0x0FFF000);
  const uint32_t d = (offset & 0x1000000) << 11;

  args.uj.imm = a | b | c | d;
  return gen_utype(inst, args, position);
}

struct bytecode_t *gen_fence(struct instruction_t inst, struct args_t args,
                             int position) {
  /* TODO: fix fence implementation w/ flags and stuff */
  logger(DEBUG, NO_ERROR, 0, "Generating FENCE instruction (%s)", inst.name);

  struct bytecode_t *res = malloc(sizeof(*res) + 2 * sizeof(*res->data));
  if (!res)
    return NULL;

  res->size = 2;
  *(uint32_t *)&(res->data) =
      (inst.opcode | (inst.funct1 << 12) | (((uint32_t)args.isb.imm) << 20));
  return res;
}

struct bytecode_t *gen_nop(struct instruction_t inst, struct args_t args,
                           int position) {
  /* TODO: check encoding correct (req. reg impl) */
  logger(DEBUG, NO_ERROR, 0, "Generating NOP instruction (%s)", inst.name);
  return gen_itype(inst, (struct args_t){ISB_TYPE, {{0, 0, 0}}}, position);
}
