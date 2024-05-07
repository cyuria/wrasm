
#include "parsers.h"

#include <err.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "elf/output.h"
#include "symbols.h"
#include "xmalloc.h"

const struct bytecode_t error_bytecode = { .size = (size_t)-1, .data = NULL };

enum math_shortcuts {
	math_mv,
	math_not,
	math_neg,
	math_negw,
	math_sextw,
};
/* TODO: Add HINT parser support */
const struct parser_t rv64s[] = {
	{ "nop", 4, &gen_nop, OP_OPI, 0, 0 },
	{ "li", 8, &gen_load_short, 0x0, 0, 0 },
	{ "la", 8, &gen_load_short, 0x1, 0, 0 },
	{ "mv", 4, &gen_math, math_mv, 0, 0 },
	{ "not", 4, &gen_math, math_not, 0, 0 },
	{ "neg", 4, &gen_math, math_neg, 0, 0 },
	{ "negw", 4, &gen_math, math_negw, 0, 0 },
	{ "sext.w", 4, &gen_math, 0x4, 0, 0 },
	{ "seqz", 4, &gen_setif, 0x0, 0, 0 },
	{ "snez", 4, &gen_setif, 0x1, 0, 0 },
	{ "sltz", 4, &gen_setif, 0x2, 0, 0 },
	{ "sgtz", 4, &gen_setif, 0x3, 0, 0 },
	{ "beqz", 4, &gen_branchif, 0x0, 0, 0 },
	{ "bnez", 4, &gen_branchif, 0x1, 0, 0 },
	{ "blez", 4, &gen_branchif, 0x2, 0, 0 },
	{ "bgez", 4, &gen_branchif, 0x3, 0, 0 },
	{ "bltz", 4, &gen_branchif, 0x4, 0, 0 },
	{ "bgtz", 4, &gen_branchif, 0x5, 0, 0 },
	{ "bgt", 4, &gen_branchif, 0x8, 0, 0 },
	{ "ble", 4, &gen_branchif, 0x9, 0, 0 },
	{ "bgtu", 4, &gen_branchif, 0xA, 0, 0 },
	{ "bleu", 4, &gen_branchif, 0xB, 0, 0 },
	{ "j", 4, &gen_jump, 0x0, 0, 0 },
	{ "jr", 4, &gen_jump, 0x1, 0, 0 },
	{ "ret", 4, &gen_ret, 0, 0, 0 },
	{ NULL, 0, NULL, 0, 0, 0 },
};

const struct parser_t rv64i[] = {
	{ "add", RV64I_SIZE, &gen_rtype, OP_OP, 0x0, 0x00 },
	{ "addi", RV64I_SIZE, &gen_itype, OP_OPI, 0x0, 0 },
	{ "addw", RV64I_SIZE, &gen_rtype, OP_OP32, 0x0, 0x00 },
	{ "addiw", RV64I_SIZE, &gen_itype, OP_OPI32, 0x0, 0 },
	{ "sub", RV64I_SIZE, &gen_rtype, OP_OP, 0x0, 0x20 },
	{ "subw", RV64I_SIZE, &gen_rtype, OP_OP32, 0x0, 0x20 },
	{ "and", RV64I_SIZE, &gen_rtype, OP_OP, 0x7, 0x00 },
	{ "andi", RV64I_SIZE, &gen_itype, OP_OPI, 0x7, 0 },
	{ "or", RV64I_SIZE, &gen_rtype, OP_OP, 0x6, 0x00 },
	{ "ori", RV64I_SIZE, &gen_itype, OP_OPI, 0x6, 0 },
	{ "xor", RV64I_SIZE, &gen_rtype, OP_OP, 0x4, 0x00 },
	{ "xori", RV64I_SIZE, &gen_itype, OP_OPI, 0x4, 0 },
	{ "sll", RV64I_SIZE, &gen_rtype, OP_OP, 0x1, 0x00 },
	{ "slli", RV64I_SIZE, &gen_itype, OP_OPI, 0x1, 0x00 },
	{ "sllw", RV64I_SIZE, &gen_rtype, OP_OP32, 0x1, 0x00 },
	{ "slliw", RV64I_SIZE, &gen_itype, OP_OPI32, 0x0, 0x00 },
	{ "srl", RV64I_SIZE, &gen_rtype, OP_OP, 0x5, 0x00 },
	{ "srli", RV64I_SIZE, &gen_itype, OP_OPI, 0x5, 0x00 },
	{ "srlw", RV64I_SIZE, &gen_rtype, OP_OP32, 0x5, 0x00 },
	{ "srliw", RV64I_SIZE, &gen_itype, OP_OPI32, 0x5, 0 },
	{ "sra", RV64I_SIZE, &gen_rtype, OP_OP, 0x5, 0x20 },
	{ "srai", RV64I_SIZE, &gen_itype2, OP_OPI, 0x5, 0 },
	{ "sraw", RV64I_SIZE, &gen_rtype, OP_OP32, 0x5, 0x20 },
	{ "sraiw", RV64I_SIZE, &gen_itype2, OP_OPI32, 0x5, 0 },
	{ "slt", RV64I_SIZE, &gen_rtype, OP_OP, 0x2, 0x00 },
	{ "slti", RV64I_SIZE, &gen_itype, OP_OPI, 0x2, 0 },
	{ "sltu", RV64I_SIZE, &gen_rtype, OP_OP, 0x3, 0x00 },
	{ "sltiu", RV64I_SIZE, &gen_itype, OP_OPI, 0x3, 0 },

	{ "beq", RV64I_SIZE, &gen_btype, OP_BRANCH, 0x0, 0 },
	{ "bne", RV64I_SIZE, &gen_btype, OP_BRANCH, 0x1, 0 },
	{ "bge", RV64I_SIZE, &gen_btype, OP_BRANCH, 0x5, 0 },
	{ "bgeu", RV64I_SIZE, &gen_btype, OP_BRANCH, 0x7, 0 },
	{ "blt", RV64I_SIZE, &gen_btype, OP_BRANCH, 0x4, 0 },
	{ "bltu", RV64I_SIZE, &gen_btype, OP_BRANCH, 0x6, 0 },
	{ "jal", RV64I_SIZE, &gen_jtype, OP_JAL, 0, 0 },
	{ "jalr", RV64I_SIZE, &gen_itype, OP_JALR, 0x0, 0 },

	{ "ecall", RV64I_SIZE, &gen_syscall, OP_SYSTEM, 0x0, 0x000 },
	{ "ebreak", RV64I_SIZE, &gen_syscall, OP_SYSTEM, 0x0, 0x001 },

	{ "lb", RV64I_SIZE, &gen_itype, OP_LOAD, 0x0, 0 },
	{ "lh", RV64I_SIZE, &gen_itype, OP_LOAD, 0x1, 0 },
	{ "lw", RV64I_SIZE, &gen_itype, OP_LOAD, 0x2, 0 },
	{ "ld", RV64I_SIZE, &gen_itype, OP_LOAD, 0x3, 0 },
	{ "lbu", RV64I_SIZE, &gen_itype, OP_LOAD, 0x4, 0 },
	{ "lhu", RV64I_SIZE, &gen_itype, OP_LOAD, 0x5, 0 },
	{ "lwu", RV64I_SIZE, &gen_itype, OP_LOAD, 0x6, 0 },

	{ "sb", RV64I_SIZE, &gen_stype, OP_STORE, 0x0, 0 },
	{ "sh", RV64I_SIZE, &gen_stype, OP_STORE, 0x1, 0 },
	{ "sw", RV64I_SIZE, &gen_stype, OP_STORE, 0x2, 0 },
	{ "sd", RV64I_SIZE, &gen_stype, OP_STORE, 0x3, 0 },

	{ "lui", RV64I_SIZE, &gen_utype, OP_LUI, 0, 0 },
	{ "auipc", RV64I_SIZE, &gen_utype, OP_AUIPC, 0, 0 },

	{ "fence", RV64I_SIZE, &gen_fence, OP_MISC_MEM, 0x0, 0 },
	/* zifencei standard extension */
	// { "FENCEI", RV64I_SIZE, &gen_ifence, OP_MISC_MEM, 0b001 },

	{ NULL, 0, NULL, 0, 0, 0 } /* Ending null terminator */
};

#define CHECK_REQUIRED(a, b, c)                                          \
	{                                                                \
		enum argtype_e required[] = { a, b, c };                 \
		if (memcmp(args.type, required, sizeof(required))) {     \
			logger(ERROR, error_invalid_syntax,              \
			       "Incorrect arguments for instruction %s", \
			       parser.name);                             \
			return error_bytecode;                           \
		}                                                        \
	}

static int32_t get_relative_address(struct symbol_t *sym, size_t position)
{
	const size_t labelpos = calc_fileoffset((struct sectionpos_t){
		.section = sym->section, .offset = sym->value });
	return (int32_t)(labelpos - position);
}

struct bytecode_t gen_empty_bytecode(void)
{
	logger(DEBUG, no_error, "Generating empty parser");
	return (struct bytecode_t){ .size = 0, .data = NULL };
}

struct bytecode_t gen_rtype(struct parser_t parser, struct args_t args,
			    size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating R type parser (%s)", parser.name);
	CHECK_REQUIRED(arg_register, arg_register, arg_register);

	struct bytecode_t res = { .size = RV64I_SIZE,
				  .data = xmalloc(RV64I_SIZE) };

	*(uint32_t *)&(res.data) =
		(parser.opcode | (args.arg[0] << 7) | (args.arg[1] << 15) |
		 (args.arg[2] << 20) | (parser.funct1 << 12) |
		 (parser.funct2 << 25));
	return res;
}

struct bytecode_t gen_itype(struct parser_t parser, struct args_t args,
			    size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating I type parser (%s, x%d, x%d, %d)",
	       parser.name, args.arg[0], args.arg[1], args.arg[2]);
	CHECK_REQUIRED(arg_register, arg_register, arg_immediate);

	struct bytecode_t res = { .size = RV64I_SIZE,
				  .data = xmalloc(RV64I_SIZE) };

	*(uint32_t *)res.data =
		(parser.opcode & 0x7F) | ((args.arg[0] & 0x1F) << 7) |
		((parser.funct1 & 0x7) << 12) | ((args.arg[1] & 0x1F) << 15) |
		((args.arg[2] & 0xFFF) << 20);

	logger(DEBUG, no_error, " | Machine Code: %.08x",
	       *(uint32_t *)res.data);

	return res;
}

struct bytecode_t gen_itype2(struct parser_t parser, struct args_t args,
			     size_t position)
{
	logger(DEBUG, no_error, "Generating I type 2 parser (%s)", parser.name);
	struct bytecode_t res = gen_itype(parser, args, position);
	*(uint32_t *)res.data |= 0x40000000; /* set type 2 bit */
	return res;
}

struct bytecode_t gen_stype(struct parser_t parser, struct args_t args,
			    size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating S type parser (%s)", parser.name);
	CHECK_REQUIRED(arg_register, arg_register, arg_immediate)
	struct bytecode_t res = { .size = RV64I_SIZE,
				  .data = xmalloc(RV64I_SIZE) };

	*(uint32_t *)res.data =
		(parser.opcode | (args.arg[0] << 15) | (args.arg[1] << 20) |
		 (parser.funct1 << 12) | ((args.arg[2] & 0x1F) << 7) |
		 ((args.arg[2] & 0xFE0) << 21));
	return res;
}

struct bytecode_t gen_btype(struct parser_t parser, struct args_t args,
			    size_t position)
{
	logger(DEBUG, no_error, "Generating B type parser (%s)", parser.name);

	const uint32_t dup = args.arg[2] >> 1;
	args.arg[2] &= 0x7FE;
	args.arg[2] |= (dup >> 10) & 0x1;
	args.arg[2] |= dup & 0x800;

	return gen_stype(parser, args, position);
}

struct bytecode_t gen_utype(struct parser_t parser, struct args_t args,
			    size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating U type parser (%s)", parser.name);
	CHECK_REQUIRED(arg_register, arg_immediate, arg_none)

	struct bytecode_t res = { .size = RV64I_SIZE,
				  .data = xmalloc(RV64I_SIZE) };

	*(uint32_t *)res.data = (parser.opcode | (args.arg[0] << 7) |
				 (args.arg[1] & 0xFFFFF000));
	return res;
}

struct bytecode_t gen_jtype(struct parser_t parser, struct args_t args,
			    size_t position)
{
	logger(DEBUG, no_error, "Generating J type parser (%s)", parser.name);
	CHECK_REQUIRED(arg_register, arg_immediate, arg_none)

	int offset = args.arg[2];
	offset -= (int)position;
	logger(DEBUG, no_error, "Offset of J type parser is 0x%x", offset);

	const uint32_t a = (offset & 0x0007FE) << 20;
	const uint32_t b = (offset & 0x000800) << 9;
	const uint32_t c = (offset & 0x0FF000);
	const uint32_t d = (offset & 0x100000) << 11;

	args.arg[2] = a | b | c | d;
	return gen_utype(parser, args, position);
}

struct bytecode_t gen_syscall(struct parser_t parser, struct args_t args,
			      size_t position)
{
	(void)args;
	(void)position;
	CHECK_REQUIRED(arg_none, arg_none, arg_none)

	struct bytecode_t res = { .size = RV64I_SIZE,
				  .data = xmalloc(RV64I_SIZE) };

	*(uint32_t *)res.data =
		(parser.opcode | (parser.funct1 << 12) | (parser.funct2 << 20));
	return res;
}

struct bytecode_t gen_fence(struct parser_t parser, struct args_t args,
			    size_t position)
{
	(void)position;
	/* TODO: fix fence implementation w/ flags and stuff */
	logger(DEBUG, no_error, "Generating FENCE parser (%s)", parser.name);
	CHECK_REQUIRED(arg_immediate, arg_none, arg_none)

	struct bytecode_t res = { .size = RV64I_SIZE,
				  .data = xmalloc(RV64I_SIZE) };

	*(uint32_t *)res.data = (parser.opcode | (parser.funct1 << 12) |
				 (((uint32_t)args.arg[0]) << 20));
	return res;
}

struct bytecode_t gen_nop(struct parser_t parser, struct args_t args,
			  size_t position)
{
	(void)args;
	(void)position;
	logger(DEBUG, no_error, "Generating NOP instruction (%s)", parser.name);
	CHECK_REQUIRED(arg_none, arg_none, arg_none)

	return gen_itype(
		parser,
		(struct args_t){ { arg_register, arg_register, arg_immediate },
				 { 0, 0, 0 } },
		position);
}

struct bytecode_t gen_load_short(struct parser_t parser, struct args_t args,
				 size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating load instruction (%s)",
	       parser.name);
	CHECK_REQUIRED(arg_register, parser.opcode ? arg_symbol : arg_immediate,
		       arg_none)

	uint32_t rd = args.arg[0];
	logger(DEBUG, no_error, " | register: x%d", rd);
	uint32_t value = (uint32_t)args.arg[1];
	if (parser.opcode)
		value = (uint32_t)get_relative_address((void *)args.arg[1],
						       position);
	logger(DEBUG, no_error, " | value: %.08x", value);

	struct bytecode_t upper = gen_utype(
		(struct parser_t){ "internal", RV64I_SIZE, NULL,
				   parser.opcode ? OP_AUIPC : OP_LUI, 0, 0 },
		(struct args_t){ { arg_register, arg_immediate, arg_none },
				 { rd, value & 0xFFFFF000, 0 } },
		0);
	struct bytecode_t lower = gen_itype(
		(struct parser_t){ "internal", RV64I_SIZE, NULL, OP_OPI, 0x0,
				   0 },
		(struct args_t){ { arg_register, arg_register, arg_immediate },
				 { rd, rd, value & 0xFFF } },
		0);

	unsigned char *data = malloc(upper.size + lower.size);
	memcpy(data, upper.data, upper.size);
	memcpy(data + upper.size, lower.data, lower.size);

	return (struct bytecode_t){
		.size = upper.size + lower.size,
		.data = data,
	};
}

/* TODO: implement math shortcut parser */
struct bytecode_t gen_math(struct parser_t parser, struct args_t args,
			   size_t position)
{
	(void)args;
	(void)position;
	// op rd, rd1 =>
	switch ((enum math_shortcuts)parser.opcode) {
	case math_mv:
		// addi rd, rs, 0
		args.type[2] = arg_immediate;
		args.arg[2] = 0;
		return gen_itype((struct parser_t){ "addi", RV64I_SIZE, NULL,
						    OP_OPI, 0x0, 0 },
				 args, position);
	case math_not: // xori rd, rs, -1
		args.type[2] = arg_immediate;
		args.arg[2] = -1;
		// { "xori", RV64I_SIZE, &gen_itype, OP_OPI, 0x4, 0 },
		return gen_itype((struct parser_t){ "xori", RV64I_SIZE, NULL,
						    OP_OPI, 0x4, 0 },
				 args, position);
	case math_neg: // sub rd, x0, rs
		return error_bytecode;
	case math_negw: // subw rd, x0, rs
		return error_bytecode;
	case math_sextw: // addiw rd, rs, 1
		return error_bytecode;
	}
}

/* TODO: implement setif parser */
struct bytecode_t gen_setif(struct parser_t parser, struct args_t args,
			    size_t position)
{
	(void)parser;
	(void)args;
	(void)position;
	return error_bytecode;
}

/* TODO: implement branchif parser */
struct bytecode_t gen_branchif(struct parser_t parser, struct args_t args,
			       size_t position)
{
	(void)parser;
	(void)args;
	(void)position;
	return error_bytecode;
}

/* TODO: implement jump parser */
struct bytecode_t gen_jump(struct parser_t parser, struct args_t args,
			   size_t position)
{
	(void)parser;
	(void)args;
	(void)position;
	return error_bytecode;
}

/* TODO: implement return parser */
struct bytecode_t gen_ret(struct parser_t parser, struct args_t args,
			  size_t position)
{
	(void)parser;
	(void)args;
	(void)position;
	return error_bytecode;
}
