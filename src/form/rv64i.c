
#include "form/rv64i.h"

#include <string.h>

#include "debug.h"
#include "elf/output.h"
#include "form/generic.h"
#include "macros.h"
#include "parse.h"
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
#define FULLY_DEFINED_SWITCH() return error_bytecode
#endif

enum load_pseudo {
	load_imm,
	load_addr,
};
enum math_pseudo {
	math_mv,
	math_not,
	math_neg,
	math_negw,
	math_sextw,
};
enum setif_pseudo {
	setif_eqz,
	setif_nez,
	setif_ltz,
	setif_gtz,
};
enum branchifz_pseudo {
	branchifz_eqz,
	branchifz_nez,
	branchifz_lez,
	branchifz_gez,
	branchifz_ltz,
	branchifz_gtz,
};
enum branchifr_pseudo {
	branchifr_gt,
	branchifr_le,
	branchifr_gtu,
	branchifr_leu,
};
enum jump_pseudo {
	jump_j,
	jump_jr,
	jump_ret,
};

/* TODO: Add HINT instruction support */
const struct formation_t rv64i[] = {
	{ "nop", &form_nop, &parse_none, { 4, OP_OPI, 0, 0 } },

	{ "li", &form_load_short, &parse_rtype, { 8, load_imm, 0, 0 } },
	{ "la", &form_load_short, &parse_rtype, { 8, load_addr, 0, 0 } },

	{ "mv", &form_math, &parse_rtype, { 4, math_mv, 0, 0 } },
	{ "not", &form_math, &parse_rtype, { 4, math_not, 0, 0 } },
	{ "neg", &form_math, &parse_rtype, { 4, math_neg, 0, 0 } },
	{ "negw", &form_math, &parse_rtype, { 4, math_negw, 0, 0 } },
	{ "sext.w", &form_math, &parse_rtype, { 4, math_sextw, 0, 0 } },

	{ "seqz", &form_setif, &parse_rtype, { 4, setif_eqz, 0, 0 } },
	{ "snez", &form_setif, &parse_rtype, { 4, setif_nez, 0, 0 } },
	{ "sltz", &form_setif, &parse_rtype, { 4, setif_ltz, 0, 0 } },
	{ "sgtz", &form_setif, &parse_rtype, { 4, setif_gtz, 0, 0 } },

	{ "beqz", &form_branchifz, &parse_rtype, { 4, branchifz_eqz, 0, 0 } },
	{ "bnez", &form_branchifz, &parse_rtype, { 4, branchifz_nez, 0, 0 } },
	{ "blez", &form_branchifz, &parse_rtype, { 4, branchifz_lez, 0, 0 } },
	{ "bgez", &form_branchifz, &parse_rtype, { 4, branchifz_gez, 0, 0 } },
	{ "bltz", &form_branchifz, &parse_rtype, { 4, branchifz_ltz, 0, 0 } },
	{ "bgtz", &form_branchifz, &parse_rtype, { 4, branchifz_gtz, 0, 0 } },
	{ "bgt", &form_branchifr, &parse_rtype, { 4, branchifr_gt, 0, 0 } },
	{ "ble", &form_branchifr, &parse_rtype, { 4, branchifr_le, 0, 0 } },
	{ "bgtu", &form_branchifr, &parse_rtype, { 4, branchifr_gtu, 0, 0 } },
	{ "bleu", &form_branchifr, &parse_rtype, { 4, branchifr_leu, 0, 0 } },

	{ "j", &form_jump, &parse_rtype, { 4, jump_j, 0, 0 } },
	{ "jr", &form_jump, &parse_rtype, { 4, jump_jr, 0, 0 } },
	{ "ret", &form_jump, &parse_none, { 4, jump_ret, 0, 0 } },

	{ "add", &form_rtype, &parse_rtype, { 4, OP_OP, 0x0, 0x00 } },
	{ "addi", &form_itype, &parse_rtype, { 4, OP_OPI, 0x0, 0 } },
	{ "addw", &form_rtype, &parse_rtype, { 4, OP_OP32, 0x0, 0x00 } },
	{ "addiw", &form_itype, &parse_rtype, { 4, OP_OPI32, 0x0, 0 } },
	{ "sub", &form_rtype, &parse_rtype, { 4, OP_OP, 0x0, 0x20 } },
	{ "subw", &form_rtype, &parse_rtype, { 4, OP_OP32, 0x0, 0x20 } },
	{ "and", &form_rtype, &parse_rtype, { 4, OP_OP, 0x7, 0x00 } },
	{ "andi", &form_itype, &parse_rtype, { 4, OP_OPI, 0x7, 0 } },
	{ "or", &form_rtype, &parse_rtype, { 4, OP_OP, 0x6, 0x00 } },
	{ "ori", &form_itype, &parse_rtype, { 4, OP_OPI, 0x6, 0 } },
	{ "xor", &form_rtype, &parse_rtype, { 4, OP_OP, 0x4, 0x00 } },
	{ "xori", &form_itype, &parse_rtype, { 4, OP_OPI, 0x4, 0 } },
	{ "sll", &form_rtype, &parse_rtype, { 4, OP_OP, 0x1, 0x00 } },
	{ "slli", &form_itype, &parse_rtype, { 4, OP_OPI, 0x1, 0x00 } },
	{ "sllw", &form_rtype, &parse_rtype, { 4, OP_OP32, 0x1, 0x00 } },
	{ "slliw", &form_itype, &parse_rtype, { 4, OP_OPI32, 0x0, 0x00 } },
	{ "srl", &form_rtype, &parse_rtype, { 4, OP_OP, 0x5, 0x00 } },
	{ "srli", &form_itype, &parse_rtype, { 4, OP_OPI, 0x5, 0x00 } },
	{ "srlw", &form_rtype, &parse_rtype, { 4, OP_OP32, 0x5, 0x00 } },
	{ "srliw", &form_itype, &parse_rtype, { 4, OP_OPI32, 0x5, 0 } },
	{ "sra", &form_rtype, &parse_rtype, { 4, OP_OP, 0x5, 0x20 } },
	{ "srai", &form_itype2, &parse_rtype, { 4, OP_OPI, 0x5, 0 } },
	{ "sraw", &form_rtype, &parse_rtype, { 4, OP_OP32, 0x5, 0x20 } },
	{ "sraiw", &form_itype2, &parse_rtype, { 4, OP_OPI32, 0x5, 0 } },
	{ "slt", &form_rtype, &parse_rtype, { 4, OP_OP, 0x2, 0x00 } },
	{ "slti", &form_itype, &parse_rtype, { 4, OP_OPI, 0x2, 0 } },
	{ "sltu", &form_rtype, &parse_rtype, { 4, OP_OP, 0x3, 0x00 } },
	{ "sltiu", &form_itype, &parse_rtype, { 4, OP_OPI, 0x3, 0 } },

	{ "beq", &form_btype, &parse_rtype, { 4, OP_BRANCH, 0x0, 0 } },
	{ "bne", &form_btype, &parse_rtype, { 4, OP_BRANCH, 0x1, 0 } },
	{ "bge", &form_btype, &parse_rtype, { 4, OP_BRANCH, 0x5, 0 } },
	{ "bgeu", &form_btype, &parse_rtype, { 4, OP_BRANCH, 0x7, 0 } },
	{ "blt", &form_btype, &parse_rtype, { 4, OP_BRANCH, 0x4, 0 } },
	{ "bltu", &form_btype, &parse_rtype, { 4, OP_BRANCH, 0x6, 0 } },

	{ "jal", &form_jtype, &parse_jal, { 4, OP_JAL, 0, 0 } },
	{ "jalr", &form_itype, &parse_jalr, { 4, OP_JALR, 0x0, 0 } },

	{ "ecall", &form_syscall, &parse_rtype, { 4, OP_SYSTEM, 0x0, 0x000 } },
	{ "ebreak", &form_syscall, &parse_rtype, { 4, OP_SYSTEM, 0x0, 0x001 } },

	{ "lb", &form_itype, &parse_rtype, { 4, OP_LOAD, 0x0, 0 } },
	{ "lh", &form_itype, &parse_rtype, { 4, OP_LOAD, 0x1, 0 } },
	{ "lw", &form_itype, &parse_rtype, { 4, OP_LOAD, 0x2, 0 } },
	{ "ld", &form_itype, &parse_rtype, { 4, OP_LOAD, 0x3, 0 } },
	{ "lbu", &form_itype, &parse_rtype, { 4, OP_LOAD, 0x4, 0 } },
	{ "lhu", &form_itype, &parse_rtype, { 4, OP_LOAD, 0x5, 0 } },
	{ "lwu", &form_itype, &parse_rtype, { 4, OP_LOAD, 0x6, 0 } },

	{ "sb", &form_stype, &parse_rtype, { 4, OP_STORE, 0x0, 0 } },
	{ "sh", &form_stype, &parse_rtype, { 4, OP_STORE, 0x1, 0 } },
	{ "sw", &form_stype, &parse_rtype, { 4, OP_STORE, 0x2, 0 } },
	{ "sd", &form_stype, &parse_rtype, { 4, OP_STORE, 0x3, 0 } },

	{ "lui", &form_utype, &parse_rtype, { 4, OP_LUI, 0, 0 } },
	{ "auipc", &form_utype, &parse_rtype, { 4, OP_AUIPC, 0, 0 } },

	{ "fence", &form_fence, &parse_rtype, { 4, OP_MISC_MEM, 0, 0 } },

	END_FORMATION
};

struct bytecode_t form_rtype(const char *name, struct idata_t instruction,
			     struct args_t args, size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating R type instruction %s", name);

	const uint32_t opcode = instruction.opcode;
	const uint32_t rd = args.rd;
	const uint32_t funct3 = instruction.funct3;
	const uint32_t rs1 = args.rs1;
	const uint32_t rs2 = args.rs2;
	const uint32_t funct7 = instruction.funct7;

	struct bytecode_t res = {
		.size = 4,
		.data = xmalloc(4),
	};
	*(uint32_t *)res.data = opcode | (rd << 7) | (funct3 << 12) |
				(rs1 << 15) | (rs2 << 20) | (funct7 << 25);
	return res;
}

struct bytecode_t form_itype(const char *name, struct idata_t instruction,
			     struct args_t args, size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating I type instruction %s", name);

	const uint32_t opcode = instruction.opcode;
	const uint32_t rd = args.rd;
	const uint32_t funct3 = instruction.funct3;
	const uint32_t rs1 = args.rs1;
	const uint32_t imm_11_0 = args.imm & 0xFFF;

	struct bytecode_t res = {
		.size = 4,
		.data = xmalloc(4),
	};
	*(uint32_t *)res.data = opcode | (rd << 7) | (funct3 << 12) |
				(rs1 << 15) | (imm_11_0 << 20);
	return res;
}

struct bytecode_t form_itype2(const char *name, struct idata_t instruction,
			      struct args_t args, size_t position)
{
	logger(DEBUG, no_error, "Generating I type 2 instruction %s", name);
	struct bytecode_t res = form_itype(name, instruction, args, position);
	*(uint32_t *)res.data |= 0x40000000; /* set type 2 bit */
	return res;
}

struct bytecode_t form_stype(const char *name, struct idata_t instruction,
			     struct args_t args, size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating S type instruction %s", name);

	const uint32_t opcode = instruction.opcode;
	const uint32_t imm_4_0 = args.imm & 0x1F;
	const uint32_t funct3 = instruction.funct3;
	const uint32_t rs1 = args.rs1;
	const uint32_t rs2 = args.rs2;
	const uint32_t imm_11_5 = (args.imm >> 5) & 0x7F;

	struct bytecode_t res = {
		.size = 4,
		.data = xmalloc(4),
	};
	*(uint32_t *)res.data = opcode | (imm_4_0 << 7) | (funct3 << 12) |
				(rs1 << 15) | (rs2 << 20) | (imm_11_5 << 25);
	return res;
}

struct bytecode_t form_btype(const char *name, struct idata_t instruction,
			     struct args_t args, size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating B type instruction %s", name);

	const struct symbol_t symbol = *args.sym;
	if (symbol.type != symbol_label)
		logger(ERROR, error_invalid_syntax,
		       "Incorrect argument types for instruction %s."
		       " Expected label, but got a different symbol",
		       symbol.name);

	const uint32_t offset =
		(uint32_t)(calc_fileoffset((struct sectionpos_t){
				   .section = symbol.section,
				   .offset = symbol.value,
			   }) -
			   position);
	logger(DEBUG, no_error, "B type instruction has offset of 0x%.04X",
	       (uint32_t)offset);
	const uint32_t opcode = instruction.opcode;
	const uint32_t imm_11 = (offset >> 11) & 0x1;
	const uint32_t imm_4_1 = (offset >> 1) & 0xF;
	const uint32_t funct3 = instruction.funct3;
	const uint32_t rs1 = args.rs1;
	const uint32_t rs2 = args.rs2;
	const uint32_t imm_10_5 = (offset >> 5) & 0x3F;
	const uint32_t imm_12 = (offset >> 12) & 0x1;

	struct bytecode_t res = {
		.size = 4,
		.data = xmalloc(4),
	};
	*(uint32_t *)res.data = opcode | (imm_11 << 7) | (imm_4_1 << 8) |
				(funct3 << 12) | (rs1 << 15) | (rs2 << 20) |
				(imm_10_5 << 25) | (imm_12 << 31);
	return res;
}

struct bytecode_t form_utype(const char *name, struct idata_t instruction,
			     struct args_t args, size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating U type instruction %s", name);

	const uint32_t opcode = instruction.opcode;
	const uint32_t rd = args.rd & 0x1F;
	const uint32_t imm_12_31 = (args.imm >> 12) & 0xFFFFF;

	struct bytecode_t res = {
		.size = 4,
		.data = xmalloc(4),
	};
	*(uint32_t *)res.data = opcode | (rd << 7) | (imm_12_31 << 12);
	return res;
}

struct bytecode_t form_jtype(const char *name, struct idata_t instruction,
			     struct args_t args, size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating J type instruction %s", name);

	int32_t offset = calc_relative_address(args.sym, position);
	logger(DEBUG, no_error, "Offset of J type instruction is 0x%x", offset);

	const uint32_t opcode = instruction.opcode;
	const uint32_t rd = args.rd & 0x1F;
	const uint32_t imm_19_12 = (offset >> 12) & 0xFF;
	const uint32_t imm_11 = (offset >> 11) & 0x1;
	const uint32_t imm_10_1 = (offset >> 1) & 0x3FF;
	const uint32_t imm_20 = (offset >> 20) & 0x1;

	struct bytecode_t res = {
		.size = 4,
		.data = xmalloc(4),
	};
	*(uint32_t *)res.data = opcode | (rd << 7) | (imm_19_12 << 12) |
				(imm_11 << 20) | (imm_10_1 << 21) |
				(imm_20 << 31);
	return res;
}

struct bytecode_t form_syscall(const char *name, struct idata_t instruction,
			       struct args_t args, size_t position)
{
	(void)args;
	(void)position;
	logger(DEBUG, no_error, "Generating syscall %s", name);

	const uint32_t opcode = instruction.opcode;
	const uint32_t funct3 = instruction.funct3;
	const uint32_t funct7 = instruction.funct7;

	struct bytecode_t res = {
		.size = 4,
		.data = xmalloc(4),
	};
	*(uint32_t *)res.data = opcode | (funct3 << 12) | (funct7 << 20);
	return res;
}

/* TODO: implement fence instruction */
struct bytecode_t form_fence(const char *name, struct idata_t instruction,
			     struct args_t args, size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating fence instruction %s", name);

	struct bytecode_t res = { .size = 4, .data = xmalloc(4) };

	*(uint32_t *)res.data = ((uint32_t)instruction.opcode |
				 ((uint32_t)instruction.funct3 << 12) |
				 (((uint32_t)args.imm) << 20));
	return res;
}

struct bytecode_t form_nop(const char *name, struct idata_t instruction,
			   struct args_t args, size_t position)
{
	(void)args;
	(void)position;
	logger(DEBUG, no_error, "Generating nop instruction %s", name);

	return form_itype(name, instruction,
			  (struct args_t){ .rd = 0, .rs1 = 0, .imm = 0 },
			  position);
}

struct bytecode_t form_load_short(const char *name, struct idata_t instruction,
				  struct args_t args, size_t position)
{
	logger(DEBUG, no_error, "Generating load instruction %s", name);

	enum load_pseudo type = instruction.opcode;
	uint8_t rd = args.rd;

	uint8_t opcode;
	uint32_t value;
	switch (type) {
	case load_imm:
		opcode = OP_LUI;
		value = args.imm;
		break;
	case load_addr:
		opcode = OP_AUIPC;
		value = (uint32_t)args.sym->value;
		break;
	default:
		UNREACHABLE();
		/* ensure warnings aren't emitted if the UNREACHABLE hint
		 * doesn't do anything */
		opcode = 0;
		value = 0;
		break;
	}

	const char *uppernames[] = { "lui (li)", "auipc (la)" };
	const char *lowernames[] = { "addi (li)", "addi (la)" };

	struct bytecode_t upper = form_utype(
		uppernames[type], (struct idata_t){ 4, opcode, 0, 0 },
		(struct args_t){
			.rd = rd,
			.imm = value & 0xFFFFF000,
		},
		position);
	struct bytecode_t lower = form_itype(
		lowernames[type], (struct idata_t){ 4, OP_OPI, 0x0, 0 },
		(struct args_t){
			.rd = rd,
			.rs1 = rd,
			.imm = value & 0xFFF,
		},
		position + 4);

	unsigned char *data = malloc(upper.size + lower.size);
	memcpy(data, upper.data, upper.size);
	memcpy(data + upper.size, lower.data, lower.size);

	return (struct bytecode_t){
		.size = upper.size + lower.size,
		.data = data,
	};
}

struct bytecode_t form_math(const char *name, struct idata_t instruction,
			    struct args_t args, size_t position)
{
	logger(DEBUG, no_error, "Generating math instruction %s", name);

	const enum math_pseudo type = instruction.opcode;
	// op rd, rs1
	switch (type) {
	case math_mv: // addi rd, rs, 0
		args.imm = 0;
		return form_itype("mv (addi)",
				  (struct idata_t){ 4, OP_OPI, 0x0, 0 }, args,
				  position);
	case math_not: // xori rd, rs, -1
		args.imm = (uint32_t)-1;
		return form_itype("not (xori)",
				  (struct idata_t){ 4, OP_OPI, 0x4, 0 }, args,
				  position);
	case math_neg: // sub rd, x0, rs
		args.rs2 = args.rs1;
		args.rs1 = 0;
		return form_rtype("neg (sub)",
				  (struct idata_t){ 4, OP_OP, 0x0, 0x20 }, args,
				  position);
	case math_negw: // subw rd, x0, rs
		args.rs2 = args.rs1;
		args.rs1 = 0;
		return form_rtype("neg (sub)",
				  (struct idata_t){ 4, OP_OP32, 0x0, 0x20 },
				  args, position);
	case math_sextw: // addiw rd, rs, 0
		args.imm = 0;
		return form_itype("sextw (addiw)",
				  (struct idata_t){ 4, OP_OPI32, 0x0, 0 }, args,
				  position);
	}
	FULLY_DEFINED_SWITCH();
}

struct bytecode_t form_setif(const char *name, struct idata_t instruction,
			     struct args_t args, size_t position)
{
	logger(DEBUG, no_error, "Generating conditional set intruction %s",
	       name);
	const enum setif_pseudo type = instruction.opcode;
	// op rd, rs1
	switch (type) {
	case setif_eqz: // sltiu rd, rs, 1
		args.imm = 1;
		return form_itype("sltiu (snez)",
				  (struct idata_t){ 4, OP_OPI, 0x3, 0x00 },
				  args, position);
	case setif_nez: // sltu rd, x0, rs
		args.rs2 = args.rs1;
		args.rs1 = 0;
		return form_rtype("sltu (snez)",
				  (struct idata_t){ 4, OP_OP, 0x3, 0x00 }, args,
				  position);
	case setif_ltz: // slt rd, rs, x0
		args.rs2 = 0;
		return form_rtype("slt (sltz)",
				  (struct idata_t){ 4, OP_OP, 0x2, 0x00 }, args,
				  position);
	case setif_gtz: // slt rd, x0, rs
		args.rs2 = args.rs1;
		args.rs1 = 0;
		return form_rtype("slt (sgtz)",
				  (struct idata_t){ 4, OP_OP, 0x2, 0x00 }, args,
				  position);
	}
	FULLY_DEFINED_SWITCH();
}

struct bytecode_t form_branchifz(const char *name, struct idata_t instruction,
				 struct args_t args, size_t position)
{
	logger(DEBUG, no_error, "Generating conditional branch intruction %s",
	       name);
	const enum branchifz_pseudo type = instruction.opcode;
	// op rs1, offset
	switch (type) {
	case branchifz_eqz: // beq rs, x0, offset
		args.rs2 = 0;
		return form_btype("beq (beqz)",
				  (struct idata_t){ 4, OP_BRANCH, 0x0, 0 },
				  args, position);
	case branchifz_nez: // bne rs, x0, offset
		args.rs2 = 0;
		return form_btype("bne (bnez)",
				  (struct idata_t){ 4, OP_BRANCH, 0x1, 0 },
				  args, position);
	case branchifz_lez: // bge x0, rs, offset
		args.rs2 = args.rs1;
		args.rs1 = 0;
		return form_btype("bge (blez)",
				  (struct idata_t){ 4, OP_BRANCH, 0x5, 0 },
				  args, position);
	case branchifz_gez: // bge rs, x0, offset
		args.rs2 = 0;
		return form_btype("bge (blez)",
				  (struct idata_t){ 4, OP_BRANCH, 0x5, 0 },
				  args, position);
	case branchifz_ltz: // blt rs, x0, offset
		args.rs2 = 0;
		return form_btype("bge (blez)",
				  (struct idata_t){ 4, OP_BRANCH, 0x4, 0 },
				  args, position);
	case branchifz_gtz: // blt x0, rs, offset
		args.rs2 = args.rs1;
		args.rs1 = 0;
		return form_btype("bge (blez)",
				  (struct idata_t){ 4, OP_BRANCH, 0x4, 0 },
				  args, position);
	}
	FULLY_DEFINED_SWITCH();
}

struct bytecode_t form_branchifr(const char *name, struct idata_t instruction,
				 struct args_t args, size_t position)
{
	logger(DEBUG, no_error, "Generating conditional branch intruction %s",
	       name);
	const enum branchifr_pseudo type = instruction.opcode;

	const uint8_t rs1 = args.rs1;
	args.rs1 = args.rs2;
	args.rs2 = rs1;

	/* Types happen to align correctly with the correct funct3 field for this trick to work */
	/* { "blt",  &form_btype, &parse_rtype, { 4, OP_BRANCH, 0x4, 0 } }, */
	/* { "bge",  &form_btype, &parse_rtype, { 4, OP_BRANCH, 0x5, 0 } }, */
	/* { "bltu", &form_btype, &parse_rtype, { 4, OP_BRANCH, 0x6, 0 } }, */
	/* { "bgeu", &form_btype, &parse_rtype, { 4, OP_BRANCH, 0x7, 0 } }, */
	const uint8_t funct3 = (uint8_t)(type + 0x4);
	const char *names[] = { "blt (bgt)", "bge (ble)", "bltu (bgtu)",
				"bgeu (bleu)" };
	return form_btype(names[type],
			  (struct idata_t){ 4, OP_BRANCH, funct3, 0 }, args,
			  position);
}

struct bytecode_t form_jump(const char *name, struct idata_t instruction,
			    struct args_t args, size_t position)
{
	logger(DEBUG, no_error, "Generating unconditional jump intruction %s",
	       name);
	const enum jump_pseudo type = instruction.opcode;
	switch (type) {
	case jump_j:
		args.rd = 0;
		return form_jtype("jal (j)",
				  (struct idata_t){ 4, OP_JAL, 0, 0 }, args,
				  position);
	case jump_ret:
		args.rs1 = 1;
		// fall through
	case jump_jr:
		args.rd = 0;
		args.imm = 0;
		return form_itype("jalr (jr)",
				  (struct idata_t){ 4, OP_JALR, 0x0, 0 }, args,
				  position);
	}
	FULLY_DEFINED_SWITCH();
}
