
#include "form/base.h"

#include <assert.h>
#include <string.h>

#include "debug.h"
#include "form/generic.h"
#include "macros.h"
#include "parse.h"
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

/* TODO: Add HINT instruction support */
const struct formation rv32i[] = {
	{ "nop", &form_nop, &parse_none, { 4, OP_OPI, 0, 0 } },

	{ "li", &form_load_pseudo, &parse_li, { 8, LOAD_IMM, 0, 0 } },
	{ "la", &form_load_pseudo, &parse_la, { 8, LOAD_ADDR, 0, 0 } },

	{ "mv", &form_math, &parse_pseudo, { 4, MATH_MV, 0, 0 } },
	{ "not", &form_math, &parse_pseudo, { 4, MATH_NOT, 0, 0 } },
	{ "neg", &form_math, &parse_pseudo, { 4, MATH_NEG, 0, 0 } },

	{ "seqz", &form_setif, &parse_pseudo, { 4, SETIF_EQZ, 0, 0 } },
	{ "snez", &form_setif, &parse_pseudo, { 4, SETIF_NEZ, 0, 0 } },
	{ "sltz", &form_setif, &parse_pseudo, { 4, SETIF_LTZ, 0, 0 } },
	{ "sgtz", &form_setif, &parse_pseudo, { 4, SETIF_GTZ, 0, 0 } },

	{ "beqz", &form_branchifz, &parse_bztype, { 4, BRANCHIFZ_EQZ, 0, 0 } },
	{ "bnez", &form_branchifz, &parse_bztype, { 4, BRANCHIFZ_NEZ, 0, 0 } },
	{ "blez", &form_branchifz, &parse_bztype, { 4, BRANCHIFZ_LEZ, 0, 0 } },
	{ "bgez", &form_branchifz, &parse_bztype, { 4, BRANCHIFZ_GEZ, 0, 0 } },
	{ "bltz", &form_branchifz, &parse_bztype, { 4, BRANCHIFZ_LTZ, 0, 0 } },
	{ "bgtz", &form_branchifz, &parse_bztype, { 4, BRANCHIFZ_GTZ, 0, 0 } },
	{ "bgt", &form_branchifr, &parse_btype, { 4, BRANCHIFR_GT, 0, 0 } },
	{ "ble", &form_branchifr, &parse_btype, { 4, BRANCHIFR_LE, 0, 0 } },
	{ "bgtu", &form_branchifr, &parse_btype, { 4, BRANCHIFR_GTU, 0, 0 } },
	{ "bleu", &form_branchifr, &parse_btype, { 4, BRANCHIFR_LEU, 0, 0 } },

	{ "j", &form_jump, &parse_j, { 4, JUMP_J, 0, 0 } },
	{ "jr", &form_jump, &parse_jr, { 4, JUMP_JR, 0, 0 } },
	{ "ret", &form_jump, &parse_none, { 4, JUMP_RET, 0, 0 } },

	{ "fence.tso", &form_itype, &parse_ftso, { 4, OP_MISC_MEM, 0x0, 0 } },

	{ "add", &form_rtype, &parse_rtype, { 4, OP_OP, 0x0, 0x00 } },
	{ "addi", &form_itype, &parse_itype, { 4, OP_OPI, 0x0, 0 } },
	{ "sub", &form_rtype, &parse_rtype, { 4, OP_OP, 0x0, 0x20 } },
	{ "and", &form_rtype, &parse_rtype, { 4, OP_OP, 0x7, 0x00 } },
	{ "andi", &form_itype, &parse_itype, { 4, OP_OPI, 0x7, 0 } },
	{ "or", &form_rtype, &parse_rtype, { 4, OP_OP, 0x6, 0x00 } },
	{ "ori", &form_itype, &parse_itype, { 4, OP_OPI, 0x6, 0 } },
	{ "xor", &form_rtype, &parse_rtype, { 4, OP_OP, 0x4, 0x00 } },
	{ "xori", &form_itype, &parse_itype, { 4, OP_OPI, 0x4, 0 } },
	{ "sll", &form_rtype, &parse_rtype, { 4, OP_OP, 0x1, 0x00 } },
	{ "srl", &form_rtype, &parse_rtype, { 4, OP_OP, 0x5, 0x00 } },
	{ "sra", &form_rtype, &parse_rtype, { 4, OP_OP, 0x5, 0x20 } },
	{ "slt", &form_rtype, &parse_rtype, { 4, OP_OP, 0x2, 0x00 } },
	{ "slti", &form_itype, &parse_itype, { 4, OP_OPI, 0x2, 0 } },
	{ "sltu", &form_rtype, &parse_rtype, { 4, OP_OP, 0x3, 0x00 } },
	{ "sltiu", &form_itype, &parse_itype, { 4, OP_OPI, 0x3, 0 } },

	{ "beq", &form_btype, &parse_btype, { 4, OP_BRANCH, 0x0, 0 } },
	{ "bne", &form_btype, &parse_btype, { 4, OP_BRANCH, 0x1, 0 } },
	{ "bge", &form_btype, &parse_btype, { 4, OP_BRANCH, 0x5, 0 } },
	{ "bgeu", &form_btype, &parse_btype, { 4, OP_BRANCH, 0x7, 0 } },
	{ "blt", &form_btype, &parse_btype, { 4, OP_BRANCH, 0x4, 0 } },
	{ "bltu", &form_btype, &parse_btype, { 4, OP_BRANCH, 0x6, 0 } },

	{ "jal", &form_jtype, &parse_jal, { 4, OP_JAL, 0, 0 } },
	{ "jalr", &form_itype, &parse_jalr, { 4, OP_JALR, 0x0, 0 } },

	{ "ecall", &form_syscall, &parse_none, { 4, OP_SYSTEM, 0x0, 0x000 } },
	{ "ebreak", &form_syscall, &parse_none, { 4, OP_SYSTEM, 0x0, 0x001 } },

	{ "lb", &form_itype, &parse_ltype, { 4, OP_LOAD, 0x0, 0 } },
	{ "lh", &form_itype, &parse_ltype, { 4, OP_LOAD, 0x1, 0 } },
	{ "lw", &form_itype, &parse_ltype, { 4, OP_LOAD, 0x2, 0 } },
	{ "lbu", &form_itype, &parse_ltype, { 4, OP_LOAD, 0x4, 0 } },
	{ "lhu", &form_itype, &parse_ltype, { 4, OP_LOAD, 0x5, 0 } },

	{ "sb", &form_stype, &parse_stype, { 4, OP_STORE, 0x0, 0 } },
	{ "sh", &form_stype, &parse_stype, { 4, OP_STORE, 0x1, 0 } },
	{ "sw", &form_stype, &parse_stype, { 4, OP_STORE, 0x2, 0 } },

	{ "lui", &form_utype, &parse_utype, { 4, OP_LUI, 0, 0 } },
	{ "auipc", &form_utype, &parse_utype, { 4, OP_AUIPC, 0, 0 } },

	{ "fence", &form_itype, &parse_fence, { 4, OP_MISC_MEM, 0x0, 0 } },

	END_FORMATION
};

const struct formation rv64i[] = {
	{ "negw", &form_math, &parse_pseudo, { 4, MATH_NEGW, 0, 0 } },
	{ "sext.w", &form_math, &parse_pseudo, { 4, MATH_SEXTW, 0, 0 } },

	{ "addw", &form_rtype, &parse_rtype, { 4, OP_OP32, 0x0, 0x00 } },
	{ "addiw", &form_itype, &parse_itype, { 4, OP_OPI32, 0x0, 0 } },
	{ "subw", &form_rtype, &parse_rtype, { 4, OP_OP32, 0x0, 0x20 } },

	{ "slli", &form_itype, &parse_itype, { 4, OP_OPI, 0x1, 0x00 } },
	{ "sllw", &form_rtype, &parse_rtype, { 4, OP_OP32, 0x1, 0x00 } },
	{ "slliw", &form_itype, &parse_itype, { 4, OP_OPI32, 0x0, 0x00 } },
	{ "srli", &form_itype, &parse_itype, { 4, OP_OPI, 0x5, 0x00 } },
	{ "srlw", &form_rtype, &parse_rtype, { 4, OP_OP32, 0x5, 0x00 } },
	{ "srliw", &form_itype, &parse_itype, { 4, OP_OPI32, 0x5, 0 } },
	{ "srai", &form_itype2, &parse_itype, { 4, OP_OPI, 0x5, 0 } },
	{ "sraw", &form_rtype, &parse_rtype, { 4, OP_OP32, 0x5, 0x20 } },
	{ "sraiw", &form_itype2, &parse_itype, { 4, OP_OPI32, 0x5, 0 } },

	{ "lwu", &form_itype, &parse_itype, { 4, OP_LOAD, 0x6, 0 } },
	{ "ld", &form_itype, &parse_itype, { 4, OP_LOAD, 0x3, 0 } },
	{ "sd", &form_stype, &parse_stype, { 4, OP_STORE, 0x3, 0 } },

	END_FORMATION
};

struct bytecode form_syscall(const char *name, struct idata instruction,
			     struct args args, size_t position)
{
	(void)args;
	(void)position;
	logger(DEBUG, no_error, "Generating syscall %s", name);

	const uint32_t opcode = instruction.opcode;
	const uint32_t funct3 = instruction.funct3;
	const uint32_t funct7 = instruction.funct7;

	assert(instruction.sz == 4);

	struct bytecode res = {
		.size = 4,
		.data = xmalloc(4),
	};
	*(uint32_t *)res.data = opcode | (funct3 << 12) | (funct7 << 20);
	return res;
}

struct bytecode form_nop(const char *name, struct idata instruction,
			 struct args args, size_t position)
{
	(void)args;
	(void)position;
	logger(DEBUG, no_error, "Generating nop instruction %s", name);

	return form_itype(name, instruction,
			  (struct args){ .rd = 0, .rs1 = 0, .imm = 0 },
			  position);
}

struct bytecode form_load_pseudo(const char *name, struct idata instruction,
				 struct args args, size_t position)
{
	logger(DEBUG, no_error, "Generating load instruction %s", name);

	enum load_pseudo type = instruction.opcode;
	uint8_t rd = args.rd;

	uint8_t opcode;
	uint32_t value;
	switch (type) {
	case LOAD_IMM:
		opcode = OP_LUI;
		value = args.imm;
		break;
	case LOAD_ADDR:
		opcode = OP_AUIPC;
		value = (uint32_t)calc_symbol_offset(args.sym, position);
		break;
	default:
		UNREACHABLE();
#ifdef NO_UNREACHABLE
		/* ensure warnings aren't emitted if the UNREACHABLE hint
		 * can't be defined */
		opcode = 0;
		value = 0;
		break;
#endif
	}

	logger(DEBUG, no_error, "Load psuedoinstruction has value %d", value);

	const char *uppernames[] = { "lui (li)", "auipc (la)" };
	const char *lowernames[] = { "addi (li)", "addi (la)" };

	struct bytecode upper = form_utype(uppernames[type],
					   (struct idata){ 4, opcode, 0, 0 },
					   (struct args){
						   .rd = rd,
						   .imm = value & 0xFFFFF000,
					   },
					   position);
	struct bytecode lower = form_itype(lowernames[type],
					   (struct idata){ 4, OP_OPI, 0x0, 0 },
					   (struct args){
						   .rd = rd,
						   .rs1 = rd,
						   .imm = value & 0xFFF,
					   },
					   position + 4);

	unsigned char *data = malloc(upper.size + lower.size);
	memcpy(data, upper.data, upper.size);
	memcpy(data + upper.size, lower.data, lower.size);

	return (struct bytecode){
		.size = upper.size + lower.size,
		.data = data,
	};
}

struct bytecode form_math(const char *name, struct idata instruction,
			  struct args args, size_t position)
{
	logger(DEBUG, no_error, "Generating math instruction %s", name);

	const enum math_pseudo type = instruction.opcode;
	// op rd, rs1
	switch (type) {
	case MATH_MV: // addi rd, rs, 0
		args.imm = 0;
		return form_itype("mv (addi)",
				  (struct idata){ 4, OP_OPI, 0x0, 0 }, args,
				  position);
	case MATH_NOT: // xori rd, rs, -1
		args.imm = (uint32_t)-1;
		return form_itype("not (xori)",
				  (struct idata){ 4, OP_OPI, 0x4, 0 }, args,
				  position);
	case MATH_NEG: // sub rd, x0, rs
		args.rs2 = args.rs1;
		args.rs1 = 0;
		return form_rtype("neg (sub)",
				  (struct idata){ 4, OP_OP, 0x0, 0x20 }, args,
				  position);
	case MATH_NEGW: // subw rd, x0, rs
		args.rs2 = args.rs1;
		args.rs1 = 0;
		return form_rtype("neg (sub)",
				  (struct idata){ 4, OP_OP32, 0x0, 0x20 }, args,
				  position);
	case MATH_SEXTW: // addiw rd, rs, 0
		args.imm = 0;
		return form_itype("sextw (addiw)",
				  (struct idata){ 4, OP_OPI32, 0x0, 0 }, args,
				  position);
	}
	FULLY_DEFINED_SWITCH();
}

struct bytecode form_setif(const char *name, struct idata instruction,
			   struct args args, size_t position)
{
	logger(DEBUG, no_error, "Generating conditional set intruction %s",
	       name);
	const enum setif_pseudo type = instruction.opcode;
	// op rd, rs1
	switch (type) {
	case SETIF_EQZ: // sltiu rd, rs, 1
		args.imm = 1;
		return form_itype("sltiu (snez)",
				  (struct idata){ 4, OP_OPI, 0x3, 0x00 }, args,
				  position);
	case SETIF_NEZ: // sltu rd, x0, rs
		args.rs2 = args.rs1;
		args.rs1 = 0;
		return form_rtype("sltu (snez)",
				  (struct idata){ 4, OP_OP, 0x3, 0x00 }, args,
				  position);
	case SETIF_LTZ: // slt rd, rs, x0
		args.rs2 = 0;
		return form_rtype("slt (sltz)",
				  (struct idata){ 4, OP_OP, 0x2, 0x00 }, args,
				  position);
	case SETIF_GTZ: // slt rd, x0, rs
		args.rs2 = args.rs1;
		args.rs1 = 0;
		return form_rtype("slt (sgtz)",
				  (struct idata){ 4, OP_OP, 0x2, 0x00 }, args,
				  position);
	}
	FULLY_DEFINED_SWITCH();
}

struct bytecode form_branchifz(const char *name, struct idata instruction,
			       struct args args, size_t position)
{
	logger(DEBUG, no_error, "Generating conditional branch intruction %s",
	       name);
	const enum branchifz_pseudo type = instruction.opcode;
	// op rs1, offset
	switch (type) {
	case BRANCHIFZ_EQZ: // beq rs, x0, offset
		args.rs2 = 0;
		return form_btype("beq (beqz)",
				  (struct idata){ 4, OP_BRANCH, 0x0, 0 }, args,
				  position);
	case BRANCHIFZ_NEZ: // bne rs, x0, offset
		args.rs2 = 0;
		return form_btype("bne (bnez)",
				  (struct idata){ 4, OP_BRANCH, 0x1, 0 }, args,
				  position);
	case BRANCHIFZ_LEZ: // bge x0, rs, offset
		args.rs2 = args.rs1;
		args.rs1 = 0;
		return form_btype("bge (blez)",
				  (struct idata){ 4, OP_BRANCH, 0x5, 0 }, args,
				  position);
	case BRANCHIFZ_GEZ: // bge rs, x0, offset
		args.rs2 = 0;
		return form_btype("bge (blez)",
				  (struct idata){ 4, OP_BRANCH, 0x5, 0 }, args,
				  position);
	case BRANCHIFZ_LTZ: // blt rs, x0, offset
		args.rs2 = 0;
		return form_btype("bge (blez)",
				  (struct idata){ 4, OP_BRANCH, 0x4, 0 }, args,
				  position);
	case BRANCHIFZ_GTZ: // blt x0, rs, offset
		args.rs2 = args.rs1;
		args.rs1 = 0;
		return form_btype("bge (blez)",
				  (struct idata){ 4, OP_BRANCH, 0x4, 0 }, args,
				  position);
	}
	FULLY_DEFINED_SWITCH();
}

struct bytecode form_branchifr(const char *name, struct idata instruction,
			       struct args args, size_t position)
{
	logger(DEBUG, no_error, "Generating conditional branch intruction %s",
	       name);
	const enum branchifr_pseudo type = instruction.opcode;

	const uint8_t rs1 = args.rs1;
	args.rs1 = args.rs2;
	args.rs2 = rs1;

	const uint8_t funct3 = (uint8_t)(type);
	const char *names[] = { "blt (bgt)", "bge (ble)", "bltu (bgtu)",
				"bgeu (bleu)" };
	return form_btype(names[type - BRANCHIFR_GT],
			  (struct idata){ 4, OP_BRANCH, funct3, 0 }, args,
			  position);
}

struct bytecode form_jump(const char *name, struct idata instruction,
			  struct args args, size_t position)
{
	logger(DEBUG, no_error, "Generating unconditional jump intruction %s",
	       name);
	const enum jump_pseudo type = instruction.opcode;
	switch (type) {
	case JUMP_J:
		args.rd = 0;
		return form_jtype("jal (j)", (struct idata){ 4, OP_JAL, 0, 0 },
				  args, position);
	case JUMP_RET:
		args.rs1 = 1;
		// fall through
	case JUMP_JR:
		args.rd = 0;
		args.imm = 0;
		return form_itype("jalr (jr)",
				  (struct idata){ 4, OP_JALR, 0x0, 0 }, args,
				  position);
	}
	FULLY_DEFINED_SWITCH();
}
