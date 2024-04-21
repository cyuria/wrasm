
#include "parsers.h"

#include <stdlib.h>

#include "debug.h"

struct bytecode_t error_bytecode = {.size = (size_t)-1};

/* TODO: Add HINT parser support */
const struct parser_t rv64i[] = {
    {"add", R_TYPE, &gen_rtype, OP_OP, 0x0, 0x00},
    {"addi", ISB_TYPE, &gen_itype, OP_OPI, 0x0, 0},
    {"addw", R_TYPE, &gen_rtype, OP_OP32, 0x0, 0x00},
    {"addiw", ISB_TYPE, &gen_itype, OP_OPI32, 0x0, 0},
    {"sub", R_TYPE, &gen_rtype, OP_OP, 0x0, 0x20},
    {"subw", R_TYPE, &gen_rtype, OP_OP32, 0x0, 0x20},
    {"and", R_TYPE, &gen_rtype, OP_OP, 0x7, 0x00},
    {"andi", ISB_TYPE, &gen_itype, OP_OPI, 0x7, 0},
    {"or", R_TYPE, &gen_rtype, OP_OP, 0x6, 0x00},
    {"ori", ISB_TYPE, &gen_itype, OP_OPI, 0x6, 0},
    {"xor", R_TYPE, &gen_rtype, OP_OP, 0x4, 0x00},
    {"xori", ISB_TYPE, &gen_itype, OP_OPI, 0x4, 0},
    {"sll", R_TYPE, &gen_rtype, OP_OP, 0x1, 0x00},
    {"slli", ISB_TYPE, &gen_itype, OP_OPI, 0x1, 0x00},
    {"sllw", R_TYPE, &gen_rtype, OP_OP32, 0x1, 0x00},
    {"slliw", ISB_TYPE, &gen_itype, OP_OPI32, 0x0, 0x00},
    {"srl", R_TYPE, &gen_rtype, OP_OP, 0x5, 0x00},
    {"srli", ISB_TYPE, &gen_itype, OP_OPI, 0x5, 0x00},
    {"srlw", R_TYPE, &gen_rtype, OP_OP32, 0x5, 0x00},
    {"srliw", ISB_TYPE, &gen_itype, OP_OPI32, 0x5, 0},
    {"sra", R_TYPE, &gen_rtype, OP_OP, 0x5, 0x20},
    {"srai", ISB_TYPE, &gen_itype2, OP_OPI, 0x5, 0},
    {"sraw", R_TYPE, &gen_rtype, OP_OP32, 0x5, 0x20},
    {"sraiw", ISB_TYPE, &gen_itype2, OP_OPI32, 0x5, 0},
    {"slt", R_TYPE, &gen_rtype, OP_OP, 0x2, 0x00},
    {"slti", ISB_TYPE, &gen_itype, OP_OPI, 0x2, 0},
    {"sltu", R_TYPE, &gen_rtype, OP_OP, 0x3, 0x00},
    {"sltiu", ISB_TYPE, &gen_itype, OP_OPI, 0x3, 0},

    {"beq", ISB_TYPE, &gen_btype, OP_BRANCH, 0x0, 0},
    {"bne", ISB_TYPE, &gen_btype, OP_BRANCH, 0x1, 0},
    {"bge", ISB_TYPE, &gen_btype, OP_BRANCH, 0x5, 0},
    {"bgeu", ISB_TYPE, &gen_btype, OP_BRANCH, 0x7, 0},
    {"blt", ISB_TYPE, &gen_btype, OP_BRANCH, 0x4, 0},
    {"bltu", ISB_TYPE, &gen_btype, OP_BRANCH, 0x6, 0},
    {"jal", UJ_TYPE, &gen_jtype, OP_JAL, 0, 0},
    {"jalr", ISB_TYPE, &gen_itype, OP_JALR, 0x0, 0},

    {"ecall", NOARGS_TYPE, &gen_syscall, OP_SYSTEM, 0x0, 0x000},
    {"ebreak", NOARGS_TYPE, &gen_syscall, OP_SYSTEM, 0x0, 0x001},

    {"lb", ISB_TYPE, &gen_itype, OP_LOAD, 0x0, 0},
    {"lh", ISB_TYPE, &gen_itype, OP_LOAD, 0x1, 0},
    {"lw", ISB_TYPE, &gen_itype, OP_LOAD, 0x2, 0},
    {"ld", ISB_TYPE, &gen_itype, OP_LOAD, 0x3, 0},
    {"lbu", ISB_TYPE, &gen_itype, OP_LOAD, 0x4, 0},
    {"lhu", ISB_TYPE, &gen_itype, OP_LOAD, 0x5, 0},
    {"lwu", ISB_TYPE, &gen_itype, OP_LOAD, 0x6, 0},

    {"sb", ISB_TYPE, &gen_stype, OP_STORE, 0x0, 0},
    {"sh", ISB_TYPE, &gen_stype, OP_STORE, 0x1, 0},
    {"sw", ISB_TYPE, &gen_stype, OP_STORE, 0x2, 0},
    {"sd", ISB_TYPE, &gen_stype, OP_STORE, 0x3, 0},

    {"lui", UJ_TYPE, &gen_utype, OP_LUI, 0, 0},
    {"auipc", UJ_TYPE, &gen_utype, OP_AUIPC, 0, 0},

    {"fence", ISB_TYPE, &gen_fence, OP_MISC_MEM, 0x0, 0},
    /* zifencei standard extension */
    // { "FENCEI", ISB_TYPE, &gen_ifence, OP_MISC_MEM, 0b001 },

    {"nop", NOARGS_TYPE, &gen_nop, OP_OPI, 0, 0},

    {NULL, 0, NULL, 0, 0, 0} /* Ending null terminator */
};

struct bytecode_t gen_empty_bytecode(void) {
  logger(DEBUG, no_error, "Generating empty parser");
  return (struct bytecode_t){.size = 0, .data = NULL};
}

struct bytecode_t gen_rtype(struct parser_t parser, struct args_t args,
                            size_t position) {
  (void)position;
  logger(DEBUG, no_error, "Generating R type parser (%s)", parser.name);

  struct bytecode_t res = {.size = RV64I_SIZE,
                           .data = malloc(RV64I_SIZE * sizeof(*res.data))};

  *(uint32_t *)&(res.data) =
      (parser.opcode | (args.r.rd << 7) | (args.r.rs1 << 15) |
       (args.r.rs2 << 20) | (parser.funct1 << 12) | (parser.funct2 << 25));
  return res;
}

struct bytecode_t gen_itype(struct parser_t parser, struct args_t args,
                            size_t position) {
  (void)position;
  logger(DEBUG, no_error, "Generating I type parser (%s, x%d, x%d, %d)",
         parser.name, args.isb.r1, args.isb.r2, args.isb.imm);

  struct bytecode_t res = {.size = RV64I_SIZE,
                           .data = malloc(RV64I_SIZE * sizeof(*res.data))};

  *(uint32_t *)res.data =
      (parser.opcode | (args.isb.r1 << 7) | (args.isb.r2 << 15) |
       (parser.funct1 << 12) | ((args.isb.imm & 0xFFF) << 20));
  return res;
}

struct bytecode_t gen_itype2(struct parser_t parser, struct args_t args,
                             size_t position) {
  logger(DEBUG, no_error, "Generating I type 2 parser (%s)", parser.name);
  struct bytecode_t res = gen_itype(parser, args, position);
  *(uint32_t *)res.data |= 0x40000000; /* set type 2 bit */
  return res;
}

struct bytecode_t gen_stype(struct parser_t parser, struct args_t args,
                            size_t position) {
  (void)position;
  logger(DEBUG, no_error, "Generating S type parser (%s)", parser.name);

  struct bytecode_t res = {.size = RV64I_SIZE,
                           .data = malloc(RV64I_SIZE * sizeof(*res.data))};

  *(uint32_t *)res.data =
      (parser.opcode | (args.isb.r1 << 15) | (args.isb.r2 << 20) |
       (parser.funct1 << 12) | ((args.isb.imm & 0x1F) << 7) |
       ((args.isb.imm & 0xFE0) << 21));
  return res;
}

struct bytecode_t gen_btype(struct parser_t parser, struct args_t args,
                            size_t position) {
  logger(DEBUG, no_error, "Generating B type parser (%s)", parser.name);

  const uint32_t dup = args.isb.imm >> 1;
  args.isb.imm &= 0x7FE;
  args.isb.imm |= (dup >> 10) & 0x1;
  args.isb.imm |= dup & 0x800;

  return gen_stype(parser, args, position);
}

struct bytecode_t gen_utype(struct parser_t parser, struct args_t args,
                            size_t position) {
  (void)position;
  logger(DEBUG, no_error, "Generating U type parser (%s)", parser.name);

  struct bytecode_t res = {.size = RV64I_SIZE,
                           .data = malloc(RV64I_SIZE * sizeof(*res.data))};

  *(uint32_t *)res.data =
      (parser.opcode | (args.uj.rd << 7) | (args.uj.imm & 0xFFFFF000));
  return res;
}

struct bytecode_t gen_jtype(struct parser_t parser, struct args_t args,
                            size_t position) {
  logger(DEBUG, no_error, "Generating J type parser (%s)", parser.name);

  int offset = args.uj.imm;
  offset -= (int)position;
  logger(DEBUG, no_error, "Offset of J type parser is 0x%x", offset);

  const uint32_t a = (offset & 0x0007FE) << 20;
  const uint32_t b = (offset & 0x000800) << 9;
  const uint32_t c = (offset & 0x0FF000);
  const uint32_t d = (offset & 0x100000) << 11;

  args.uj.imm = a | b | c | d;
  return gen_utype(parser, args, position);
}

struct bytecode_t gen_syscall(struct parser_t parser, struct args_t args,
                              size_t position) {
  (void)args;
  (void)position;

  struct bytecode_t res = {.size = RV64I_SIZE,
                           .data = malloc(RV64I_SIZE * sizeof(*res.data))};

  *(uint32_t *)res.data =
      (parser.opcode | (parser.funct1 << 12) | (parser.funct2 << 20));
  return res;
}

struct bytecode_t gen_fence(struct parser_t parser, struct args_t args,
                            size_t position) {
  (void)position;
  /* TODO: fix fence implementation w/ flags and stuff */
  logger(DEBUG, no_error, "Generating FENCE parser (%s)", parser.name);

  struct bytecode_t res = {.size = RV64I_SIZE,
                           .data = malloc(RV64I_SIZE * sizeof(*res.data))};

  *(uint32_t *)res.data = (parser.opcode | (parser.funct1 << 12) |
                           (((uint32_t)args.isb.imm) << 20));
  return res;
}

struct bytecode_t gen_nop(struct parser_t parser, struct args_t args,
                          size_t position) {
  (void)args;
  (void)position;
  logger(DEBUG, no_error, "Generating NOP instruction (%s)", parser.name);
  return gen_itype(parser, (struct args_t){ISB_TYPE, {{0, 0, 0}}}, position);
}
