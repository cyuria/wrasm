
#include "parsers.h"

#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "elf/output.h"
#include "symbols.h"
#include "xmalloc.h"

/*
 * Clang can correctly optimise fully defined switches over enumerated values.
 * For other compilers, we create an unreachable segment of code, however for
 * clang we do nothing, as clang will raise an error if the switch is ever not
 * fully defined.
 */
#ifdef __clang__
#define FULLY_DEFINED_SWITCH()
#elif defined(__GNUC__)
#define FULLY_DEFINED_SWITCH() __builtin_unreachable()
#elif defined(_MSC_VER)
#define FULLY_DEFINED_SWITCH() __assume(0)
#else
#warning "compiler does not define __GNUC__ and is not MSVC"
#define FULLY_DEFINED_SWITCH()
#endif

const struct bytecode_t error_bytecode = { .size = (size_t)-1, .data = NULL };

enum load_shortcuts {
	load_immediate,
	load_address,
};
enum math_shortcuts {
	math_mv,
	math_not,
	math_neg,
	math_negw,
	math_sextw,
};
enum setif_shortcuts {
	setif_eqz,
	setif_nez,
	setif_ltz,
	setif_gtz,
};
enum branchifz_shortcuts {
	branchifz_eqz,
	branchifz_nez,
	branchifz_lez,
	branchifz_gez,
	branchifz_ltz,
	branchifz_gtz,
};
enum branchifr_shortcuts {
	branchifr_gt,
	branchifr_le,
	branchifr_gtu,
	branchifr_leu,
};
enum jump_shortcuts {
	jump_label,
	jump_register,
};
/* TODO: Add HINT parser support */
const struct parser_t rv64s[] = {
	{ "nop", RV64I_SIZE, &gen_nop, OP_OPI, 0, 0 },
	{ "li", 2 * RV64I_SIZE, &gen_load_short, load_immediate, 0, 0 },
	{ "la", 2 * RV64I_SIZE, &gen_load_short, load_address, 0, 0 },
	{ "mv", RV64I_SIZE, &gen_math, math_mv, 0, 0 },
	{ "not", RV64I_SIZE, &gen_math, math_not, 0, 0 },
	{ "neg", RV64I_SIZE, &gen_math, math_neg, 0, 0 },
	{ "negw", RV64I_SIZE, &gen_math, math_negw, 0, 0 },
	{ "sext.w", RV64I_SIZE, &gen_math, math_sextw, 0, 0 },
	{ "seqz", RV64I_SIZE, &gen_setif, setif_eqz, 0, 0 },
	{ "snez", RV64I_SIZE, &gen_setif, setif_nez, 0, 0 },
	{ "sltz", RV64I_SIZE, &gen_setif, setif_ltz, 0, 0 },
	{ "sgtz", RV64I_SIZE, &gen_setif, setif_gtz, 0, 0 },
	{ "beqz", RV64I_SIZE, &gen_branchifz, branchifz_eqz, 0, 0 },
	{ "bnez", RV64I_SIZE, &gen_branchifz, branchifz_nez, 0, 0 },
	{ "blez", RV64I_SIZE, &gen_branchifz, branchifz_lez, 0, 0 },
	{ "bgez", RV64I_SIZE, &gen_branchifz, branchifz_gez, 0, 0 },
	{ "bltz", RV64I_SIZE, &gen_branchifz, branchifz_ltz, 0, 0 },
	{ "bgtz", RV64I_SIZE, &gen_branchifz, branchifz_gtz, 0, 0 },
	{ "bgt", RV64I_SIZE, &gen_branchifr, branchifr_gt, 0, 0 },
	{ "ble", RV64I_SIZE, &gen_branchifr, branchifr_le, 0, 0 },
	{ "bgtu", RV64I_SIZE, &gen_branchifr, branchifr_gtu, 0, 0 },
	{ "bleu", RV64I_SIZE, &gen_branchifr, branchifr_leu, 0, 0 },
	{ "j", RV64I_SIZE, &gen_jump, jump_label, 0, 0 },
	{ "jr", RV64I_SIZE, &gen_jump, jump_register, 0, 0 },
	{ "ret", RV64I_SIZE, &gen_ret, 0, 0, 0 },
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

static int check_required(const char *name, enum argtype_e types[3],
			  enum argtype_e a, enum argtype_e b, enum argtype_e c)
{
	enum argtype_e required[] = { a, b, c };
	if (!memcmp(types, required, sizeof(required)))
		return 0;
	logger(ERROR, error_invalid_syntax,
	       "Incorrect argument types for instruction %s."
	       " Expected %d, %d, %d but got %d, %d, %d",
	       name, required[0], required[1], required[2], types[0], types[1],
	       types[2]);
	return 1;
}

static int32_t get_relative_address(struct symbol_t *sym, size_t position)
{
	const size_t labelpos = calc_fileoffset((struct sectionpos_t){
		.section = sym->section, .offset = (size_t)sym->value });
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
	logger(DEBUG, no_error, "Generating R type instruction %s",
	       parser.name);
	check_required(parser.name, args.type, arg_register, arg_register,
		       arg_register);

	const uint32_t opcode = parser.opcode;
	const uint32_t rd = args.arg[0] & 0x1F;
	const uint32_t funct3 = parser.funct3 & 0x7;
	const uint32_t rs1 = args.arg[1] & 0x1F;
	const uint32_t rs2 = args.arg[2] & 0x1F;
	const uint32_t funct7 = parser.funct7 & 0x7F;

	struct bytecode_t res = {
		.size = RV64I_SIZE,
		.data = xmalloc(RV64I_SIZE),
	};
	*(uint32_t *)res.data = opcode | (rd << 7) | (funct3 << 12) |
				(rs1 << 15) | (rs2 << 20) | (funct7 << 25);
	return res;
}

struct bytecode_t gen_itype(struct parser_t parser, struct args_t args,
			    size_t position)
{
	(void)position;
	logger(DEBUG, no_error,
	       "Generating I type instruction %s, x%d, x%d, %d", parser.name,
	       args.arg[0], args.arg[1], args.arg[2]);
	check_required(parser.name, args.type, arg_register, arg_register,
		       arg_immediate);

	const uint32_t opcode = parser.opcode;
	const uint32_t rd = args.arg[0] & 0x1F;
	const uint32_t funct3 = parser.funct3;
	const uint32_t rs1 = args.arg[1] & 0x1F;
	const uint32_t imm_11_0 = args.arg[2] & 0xFFF;

	struct bytecode_t res = {
		.size = RV64I_SIZE,
		.data = xmalloc(RV64I_SIZE),
	};
	*(uint32_t *)res.data = opcode | (rd << 7) | (funct3 << 12) |
				(rs1 << 15) | (imm_11_0 << 20);
	return res;
}

struct bytecode_t gen_itype2(struct parser_t parser, struct args_t args,
			     size_t position)
{
	logger(DEBUG, no_error, "Generating I type 2 instruction %s",
	       parser.name);
	struct bytecode_t res = gen_itype(parser, args, position);
	*(uint32_t *)res.data |= 0x40000000; /* set type 2 bit */
	return res;
}

struct bytecode_t gen_stype(struct parser_t parser, struct args_t args,
			    size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating S type instruction %s",
	       parser.name);
	check_required(parser.name, args.type, arg_register, arg_register,
		       arg_immediate);

	const uint32_t opcode = parser.opcode;
	const uint32_t imm_4_0 = args.arg[2] & 0x1F;
	const uint32_t funct3 = parser.funct3;
	const uint32_t rs1 = args.arg[1] & 0x1F;
	const uint32_t rs2 = args.arg[2] & 0x1F;
	const uint32_t imm_11_5 = (args.arg[2] >> 5) & 0x7F;

	struct bytecode_t res = {
		.size = RV64I_SIZE,
		.data = xmalloc(RV64I_SIZE),
	};
	*(uint32_t *)res.data = opcode | (imm_4_0 << 7) | (funct3 << 12) |
				(rs1 << 15) | (rs2 << 20) | (imm_11_5 << 25);
	return res;
}

struct bytecode_t gen_btype(struct parser_t parser, struct args_t args,
			    size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating B type instruction %s",
	       parser.name);
	check_required(parser.name, args.type, arg_register, arg_register,
		       arg_symbol);

	const struct symbol_t symbol = *(struct symbol_t *)args.arg[2];
	if (symbol.type != symbol_label)
		logger(ERROR, error_invalid_syntax,
		       "Incorrect argument types for instruction %s."
		       " Expected label, but got a different symbol",
		       symbol.name);

	const uint32_t offset = calc_fileoffset((struct sectionpos_t){
		.section = symbol.section,
		.offset = symbol.value,
	}) - position;
	logger(DEBUG, no_error, "B type instruction has offset of 0x%.04X",
	       (uint32_t)offset);
	const uint32_t opcode = parser.opcode;
	const uint32_t imm_11 = (offset >> 11) & 0x1;
	const uint32_t imm_4_1 = (offset >> 1) & 0xF;
	const uint32_t funct3 = parser.funct3 & 0x7;
	const uint32_t rs1 = args.arg[0] & 0x1F;
	const uint32_t rs2 = args.arg[1] & 0x1F;
	const uint32_t imm_10_5 = (offset >> 5) & 0x3F;
	const uint32_t imm_12 = (offset >> 12) & 0x1;

	struct bytecode_t res = {
		.size = RV64I_SIZE,
		.data = xmalloc(RV64I_SIZE),
	};
	*(uint32_t *)res.data = opcode | (imm_11 << 7) | (imm_4_1 << 8) |
				(funct3 << 12) | (rs1 << 15) | (rs2 << 20) |
				(imm_10_5 << 25) | (imm_12 << 31);
	return res;
}

struct bytecode_t gen_utype(struct parser_t parser, struct args_t args,
			    size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating U type instruction %s",
	       parser.name);
	check_required(parser.name, args.type, arg_register, arg_immediate,
		       arg_none);

	const uint32_t opcode = parser.opcode;
	const uint32_t rd = args.arg[0] & 0x1F;
	const uint32_t imm_12_31 = (args.arg[1] >> 12) & 0xFFFFF;

	struct bytecode_t res = {
		.size = RV64I_SIZE,
		.data = xmalloc(RV64I_SIZE),
	};
	*(uint32_t *)res.data = opcode | (rd << 7) | (imm_12_31 << 12);
	return res;
}

struct bytecode_t gen_jtype(struct parser_t parser, struct args_t args,
			    size_t position)
{
	logger(DEBUG, no_error, "Generating J type instruction %s",
	       parser.name);
	check_required(parser.name, args.type, arg_register, arg_immediate,
		       arg_none);

	int32_t offset = (int32_t)args.arg[1];
	offset -= (int32_t)position;
	logger(DEBUG, no_error, "Offset of J type instruction is 0x%x", offset);

	const uint32_t opcode = parser.opcode;
	const uint32_t rd = args.arg[0] & 0x1F;
	const uint32_t imm_19_12 = (args.arg[1] >> 12) & 0xFF;
	const uint32_t imm_11 = (args.arg[1] >> 11) & 0x1;
	const uint32_t imm_10_1 = (args.arg[1] >> 1) & 0x3FF;
	const uint32_t imm_20 = (args.arg[1] >> 20) & 0x1;

	struct bytecode_t res = {
		.size = RV64I_SIZE,
		.data = xmalloc(RV64I_SIZE),
	};
	*(uint32_t *)res.data = opcode | (rd << 7) | (imm_19_12 << 12) |
				(imm_11 << 20) | (imm_10_1 << 21) |
				(imm_20 << 31);
	return res;
}

struct bytecode_t gen_syscall(struct parser_t parser, struct args_t args,
			      size_t position)
{
	(void)args;
	(void)position;
	logger(DEBUG, no_error, "Generating syscall %s", parser.name);
	check_required(parser.name, args.type, arg_none, arg_none, arg_none);

	const uint32_t opcode = parser.opcode;
	const uint32_t funct3 = parser.funct3;
	const uint32_t funct7 = parser.funct7;

	struct bytecode_t res = {
		.size = RV64I_SIZE,
		.data = xmalloc(RV64I_SIZE),
	};
	*(uint32_t *)res.data = opcode | (funct3 << 12) | (funct7 << 20);
	return res;
}

/* TODO: implement fence instruction */
struct bytecode_t gen_fence(struct parser_t parser, struct args_t args,
			    size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating fence instruction %s", parser.name);
	check_required(parser.name, args.type, arg_immediate, arg_none,
		       arg_none);

	struct bytecode_t res = { .size = RV64I_SIZE,
				  .data = xmalloc(RV64I_SIZE) };

	*(uint32_t *)res.data =
		((uint32_t)parser.opcode | ((uint32_t)parser.funct3 << 12) |
		 (((uint32_t)args.arg[0]) << 20));
	return res;
}

struct bytecode_t gen_nop(struct parser_t parser, struct args_t args,
			  size_t position)
{
	(void)args;
	(void)position;
	logger(DEBUG, no_error, "Generating nop instruction %s", parser.name);
	check_required(parser.name, args.type, arg_none, arg_none, arg_none);

	return gen_itype(
		parser,
		(struct args_t){ { arg_register, arg_register, arg_immediate },
				 { 0, 0, 0 } },
		position);
}

struct bytecode_t gen_load_short(struct parser_t parser, struct args_t args,
				 size_t position)
{
	logger(DEBUG, no_error, "Generating load instruction %s", parser.name);
	check_required(parser.name, args.type, arg_register,
		       parser.opcode ? arg_symbol : arg_immediate, arg_none);

	enum load_shortcuts type = parser.opcode;
	uint32_t rd = (uint32_t)args.arg[0];
	uint32_t value = (uint32_t)args.arg[1];
	if (type == load_address)
		value = (uint32_t)get_relative_address((void *)args.arg[1],
						       position);

	struct bytecode_t upper = gen_utype(
		(struct parser_t){ "internal", RV64I_SIZE, NULL,
				   type == load_address ? OP_AUIPC : OP_LUI, 0,
				   0 },
		(struct args_t){ { arg_register, arg_immediate, arg_none },
				 { rd, value & 0xFFFFF000, 0 } },
		position);
	struct bytecode_t lower = gen_itype(
		(struct parser_t){ "internal", RV64I_SIZE, NULL, OP_OPI, 0x0,
				   0 },
		(struct args_t){ { arg_register, arg_register, arg_immediate },
				 { rd, rd, value & 0xFFF } },
		position + RV64I_SIZE);

	unsigned char *data = malloc(upper.size + lower.size);
	memcpy(data, upper.data, upper.size);
	memcpy(data + upper.size, lower.data, lower.size);

	return (struct bytecode_t){
		.size = upper.size + lower.size,
		.data = data,
	};
}

struct bytecode_t gen_math(struct parser_t parser, struct args_t args,
			   size_t position)
{
	logger(DEBUG, no_error, "Generating math instruction %s", parser.name);
	check_required(parser.name, args.type, arg_register, arg_register,
		       arg_none);
	const enum math_shortcuts type = parser.opcode;
	// op rd, rs1
	switch (type) {
	case math_mv: // addi rd, rs, 0
		args.type[2] = arg_immediate;
		args.arg[2] = 0;
		return gen_itype((struct parser_t){ "mv (addi)", RV64I_SIZE,
						    NULL, OP_OPI, 0x0, 0 },
				 args, position);
	case math_not: // xori rd, rs, -1
		args.type[2] = arg_immediate;
		args.arg[2] = (uint32_t)-1;
		return gen_itype((struct parser_t){ "not (xori)", RV64I_SIZE,
						    NULL, OP_OPI, 0x4, 0 },
				 args, position);
	case math_neg: // sub rd, x0, rs
		args.type[2] = args.type[1];
		args.arg[2] = args.arg[1];
		args.type[1] = arg_register;
		args.arg[1] = 0;
		return gen_rtype((struct parser_t){ "neg (sub)", RV64I_SIZE,
						    NULL, OP_OP, 0x0, 0x20 },
				 args, position);
	case math_negw: // subw rd, x0, rs
		args.type[2] = args.type[1];
		args.arg[2] = args.arg[1];
		args.type[1] = arg_register;
		args.arg[1] = 0;
		return gen_rtype((struct parser_t){ "negw (subw)", RV64I_SIZE,
						    NULL, OP_OP32, 0x0, 0x20 },
				 args, position);
	case math_sextw: // addiw rd, rs, 1
		args.type[2] = arg_immediate;
		args.arg[2] = 0;
		return gen_itype((struct parser_t){ "sextw (addiw)", RV64I_SIZE,
						    NULL, OP_OPI32, 0x0, 0 },
				 args, position);
	}
	FULLY_DEFINED_SWITCH();
}

struct bytecode_t gen_setif(struct parser_t parser, struct args_t args,
			    size_t position)
{
	logger(DEBUG, no_error, "Generating conditional set intruction %s",
	       parser.name);
	check_required(parser.name, args.type, arg_register, arg_register,
		       arg_none);
	const enum setif_shortcuts type = parser.opcode;
	// op rd, rs1
	switch (type) {
	case setif_eqz: // sltiu rd, rs, 1
		args.type[2] = arg_immediate;
		args.arg[2] = 1;
		return gen_itype((struct parser_t){ "sltiu (snez)", RV64I_SIZE,
						    NULL, OP_OPI, 0x3, 0x00 },
				 args, position);
	case setif_nez: // sltu rd, x0, rs
		args.type[2] = arg_register;
		args.arg[2] = args.arg[1];
		args.arg[1] = 0;
		return gen_rtype((struct parser_t){ "sltu (snez)", RV64I_SIZE,
						    NULL, OP_OP, 0x3, 0x00 },
				 args, position);
	case setif_ltz: // slt rd, rs, x0
		args.type[2] = arg_register;
		args.arg[2] = 0;
		return gen_rtype((struct parser_t){ "slt (sltz)", RV64I_SIZE,
						    NULL, OP_OP, 0x2, 0x00 },
				 args, position);
	case setif_gtz: // slt rd, x0, rs
		args.type[2] = arg_register;
		args.arg[2] = args.arg[1];
		args.arg[1] = 0;
		return gen_rtype((struct parser_t){ "slt (sgtz)", RV64I_SIZE,
						    NULL, OP_OP, 0x2, 0x00 },
				 args, position);
	}
	FULLY_DEFINED_SWITCH();
}

struct bytecode_t gen_branchifz(struct parser_t parser, struct args_t args,
				size_t position)
{
	logger(DEBUG, no_error, "Generating conditional branch intruction %s",
	       parser.name);
	check_required(parser.name, args.type, arg_register, arg_symbol,
		       arg_none);
	const enum branchifz_shortcuts type = parser.opcode;
	args.type[1] = arg_register;
	args.type[2] = arg_symbol;
	args.arg[2] = args.arg[1];
	// op rs1, offset
	switch (type) {
	case branchifz_eqz: // beq rs, x0, offset
		args.arg[1] = 0;
		return gen_btype((struct parser_t){ "beq (beqz)", RV64I_SIZE,
						    NULL, OP_BRANCH, 0x0, 0 },
				 args, position);
	case branchifz_nez: // bne rs, x0, offset
		args.arg[1] = 0;
		return gen_btype((struct parser_t){ "bne (bnez)", RV64I_SIZE,
						    NULL, OP_BRANCH, 0x1, 0 },
				 args, position);
	case branchifz_lez: // bge x0, rs, offset
		args.arg[1] = args.arg[0];
		args.arg[0] = 0;
		return gen_btype((struct parser_t){ "bge (blez)", RV64I_SIZE,
						    NULL, OP_BRANCH, 0x5, 0 },
				 args, position);
	case branchifz_gez: // bge rs, x0, offset
		args.arg[1] = 0;
		return gen_btype((struct parser_t){ "bge (blez)", RV64I_SIZE,
						    NULL, OP_BRANCH, 0x5, 0 },
				 args, position);
	case branchifz_ltz: // blt rs, x0, offset
		args.arg[1] = 0;
		return gen_btype((struct parser_t){ "bge (blez)", RV64I_SIZE,
						    NULL, OP_BRANCH, 0x4, 0 },
				 args, position);
	case branchifz_gtz: // blt x0, rs, offset
		args.arg[1] = args.arg[0];
		args.arg[0] = 0;
		return gen_btype((struct parser_t){ "bge (blez)", RV64I_SIZE,
						    NULL, OP_BRANCH, 0x4, 0 },
				 args, position);
	}
	FULLY_DEFINED_SWITCH();
}

struct bytecode_t gen_branchifr(struct parser_t parser, struct args_t args,
				size_t position)
{
	logger(DEBUG, no_error, "Generating conditional branch intruction %s",
	       parser.name);
	check_required(parser.name, args.type, arg_register, arg_register,
		       arg_symbol);
	const enum branchifr_shortcuts type = parser.opcode;

	register size_t rs = args.arg[0];
	args.arg[0] = args.arg[1];
	args.arg[1] = rs;

	const uint16_t funct1 = type + 0x4;
	const char *names[] = { "blt (bgt)", "bge (ble)", "bltu (bgtu)",
				"bgeu (bleu)" };
	return gen_btype((struct parser_t){ names[type], RV64I_SIZE, NULL,
					    OP_BRANCH, funct1, 0 },
			 args, position);
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
