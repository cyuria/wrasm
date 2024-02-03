
#include "parsers.h"

#include <stdlib.h>

#include "debug.h"

struct bytecode_t error_bytecode = {.size = -1};

/* TODO: Add HINT parser support */
const struct parser_t rv64i[] = {
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

    {"ecall", NOARGS_TYPE, &gen_syscall, OP_SYSTEM, 0b000, 0x000},
    {"ebreak", NOARGS_TYPE, &gen_syscall, OP_SYSTEM, 0b000, 0x001},

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
  logger(DEBUG, no_error, "Generating empty parser");
  struct bytecode_t *res = malloc(sizeof(*res));
  if (!res)
    return NULL; /* if malloc fails */
  res->size = 0;
  return res;
}

struct bytecode_t *gen_rtype(struct parser_t parser, struct args_t args,
                             int position) {
  logger(DEBUG, no_error, "Generating R type parser (%s)", parser.name);

  struct bytecode_t *res = malloc(sizeof(*res) + RV64I_SIZE * sizeof(*res->data));
  if (!res)
    return NULL;

  res->size = RV64I_SIZE;
  *(uint32_t *)&(res->data) =
      (parser.opcode | (args.r.rd << 7) | (args.r.rs1 << 15) |
       (args.r.rs2 << 20) | (parser.funct1 << 12) | (parser.funct2 << 25));
  return res;
}

struct bytecode_t *gen_itype(struct parser_t parser, struct args_t args,
                             int position) {
  logger(DEBUG, no_error, "Generating I type parser (%s, x%d, x%d, %d)",
         parser.name, args.isb.r1, args.isb.r2, args.isb.imm);

  struct bytecode_t *res =
      malloc(sizeof(*res) + RV64I_SIZE * sizeof(*res->data));
  if (!res)
    return NULL;

  res->size = RV64I_SIZE;
  *(uint32_t *)&(res->data) =
      (parser.opcode | (args.isb.r1 << 7) | (args.isb.r2 << 15) |
       (parser.funct1 << 12) | ((args.isb.imm & 0xFFF) << 20));
  return res;
}

struct bytecode_t *gen_itype2(struct parser_t parser, struct args_t args,
                              int position) {
  logger(DEBUG, no_error, "Generating I type 2 parser (%s)", parser.name);
  struct bytecode_t *res = gen_itype(parser, args, position);
  *(uint32_t *)&(res->data) |= 0x40000000; /* set type 2 bit */
  return res;
}

struct bytecode_t *gen_stype(struct parser_t parser, struct args_t args,
                             int position) {
  logger(DEBUG, no_error, "Generating S type parser (%s)", parser.name);

  struct bytecode_t *res =
      malloc(sizeof(*res) + RV64I_SIZE * sizeof(*res->data));
  if (!res)
    return NULL;

  res->size = RV64I_SIZE;
  *(uint32_t *)&(res->data) =
      (parser.opcode | (args.isb.r1 << 15) | (args.isb.r2 << 20) |
       (parser.funct1 << 12) | ((args.isb.imm & 0x1F) << 7) |
       ((args.isb.imm & 0xFE0) << 21));
  return res;
}

struct bytecode_t *gen_btype(struct parser_t parser, struct args_t args,
                             int position) {
  logger(DEBUG, no_error, "Generating B type parser (%s)", parser.name);

  const uint32_t dup = args.isb.imm >> 1;
  args.isb.imm &= 0x7FE;
  args.isb.imm |= (dup >> 10) & 0x1;
  args.isb.imm |= dup & 0x800;

  return gen_stype(parser, args, position);
}

struct bytecode_t *gen_utype(struct parser_t parser, struct args_t args,
                             int position) {
  logger(DEBUG, no_error, "Generating U type parser (%s)", parser.name);

  struct bytecode_t *res =
      malloc(sizeof(*res) + RV64I_SIZE * sizeof(*res->data));
  if (!res)
    return &error_bytecode;

  res->size = RV64I_SIZE;
  *(uint32_t *)&(res->data) =
      (parser.opcode | (args.uj.rd << 7) | (args.uj.imm & 0xFFFFF000));
  return res;
}

struct bytecode_t *gen_jtype(struct parser_t parser, struct args_t args,
                             int position) {
  logger(DEBUG, no_error, "Generating J type parser (%s)", parser.name);

  int offset = args.uj.imm;
  if (unlikely(offset < 0))
    return NULL;
  offset -= position;
  logger(DEBUG, no_error, "Offset of J type parser is 0x%x", offset);

  const uint32_t a = (offset & 0x0007FE) << 20;
  const uint32_t b = (offset & 0x000800) << 9;
  const uint32_t c = (offset & 0x0FF000);
  const uint32_t d = (offset & 0x100000) << 11;

  args.uj.imm = a | b | c | d;
  return gen_utype(parser, args, position);
}

struct bytecode_t *gen_syscall(struct parser_t parser, struct args_t args,
                               int position) {
  struct bytecode_t *res =
      malloc(sizeof(*res) + RV64I_SIZE * sizeof(*res->data));
  if (!res)
    return NULL;

  res->size = RV64I_SIZE;
  *(uint32_t *)&(res->data) =
      (parser.opcode | (parser.funct1 << 12) | (parser.funct2 << 20));
  return res;
}

struct bytecode_t *gen_fence(struct parser_t parser, struct args_t args,
                             int position) {
  /* TODO: fix fence implementation w/ flags and stuff */
  logger(DEBUG, no_error, "Generating FENCE parser (%s)", parser.name);

  struct bytecode_t *res =
      malloc(sizeof(*res) + RV64I_SIZE * sizeof(*res->data));
  if (!res)
    return NULL;

  res->size = RV64I_SIZE;
  *(uint32_t *)&(res->data) =
      (parser.opcode | (parser.funct1 << 12) | (((uint32_t)args.isb.imm) << 20));
  return res;
}

struct bytecode_t *gen_nop(struct parser_t parser, struct args_t args,
                           int position) {
  logger(DEBUG, no_error, "Generating NOP instruction (%s)", parser.name);
  return gen_itype(parser, (struct args_t){ISB_TYPE, {{0, 0, 0}}}, position);
}
