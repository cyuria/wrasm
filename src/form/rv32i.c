
#include "form/rv32i.h"

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
const struct formation_t rv32i[] = {
	{ "nop", &form_nop, &parse_none, { 4, OP_OPI, 0, 0 } },

	{ "li", &form_load_pseudo, &parse_li, { 8, load_imm, 0, 0 } },
	{ "la", &form_load_pseudo, &parse_la, { 8, load_addr, 0, 0 } },

	{ "mv", &form_math, &parse_pseudo, { 4, math_mv, 0, 0 } },
	{ "not", &form_math, &parse_pseudo, { 4, math_not, 0, 0 } },
	{ "neg", &form_math, &parse_pseudo, { 4, math_neg, 0, 0 } },

	{ "seqz", &form_setif, &parse_pseudo, { 4, setif_eqz, 0, 0 } },
	{ "snez", &form_setif, &parse_pseudo, { 4, setif_nez, 0, 0 } },
	{ "sltz", &form_setif, &parse_pseudo, { 4, setif_ltz, 0, 0 } },
	{ "sgtz", &form_setif, &parse_pseudo, { 4, setif_gtz, 0, 0 } },

	{ "beqz", &form_branchifz, &parse_bztype, { 4, branchifz_eqz, 0, 0 } },
	{ "bnez", &form_branchifz, &parse_bztype, { 4, branchifz_nez, 0, 0 } },
	{ "blez", &form_branchifz, &parse_bztype, { 4, branchifz_lez, 0, 0 } },
	{ "bgez", &form_branchifz, &parse_bztype, { 4, branchifz_gez, 0, 0 } },
	{ "bltz", &form_branchifz, &parse_bztype, { 4, branchifz_ltz, 0, 0 } },
	{ "bgtz", &form_branchifz, &parse_bztype, { 4, branchifz_gtz, 0, 0 } },
	{ "bgt", &form_branchifr, &parse_btype, { 4, branchifr_gt, 0, 0 } },
	{ "ble", &form_branchifr, &parse_btype, { 4, branchifr_le, 0, 0 } },
	{ "bgtu", &form_branchifr, &parse_btype, { 4, branchifr_gtu, 0, 0 } },
	{ "bleu", &form_branchifr, &parse_btype, { 4, branchifr_leu, 0, 0 } },

	{ "j", &form_jump, &parse_j, { 4, jump_j, 0, 0 } },
	{ "jr", &form_jump, &parse_jr, { 4, jump_jr, 0, 0 } },
	{ "ret", &form_jump, &parse_none, { 4, jump_ret, 0, 0 } },

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

struct bytecode_t form_syscall(const char *name, struct idata_t instruction,
			       struct args_t args, size_t position)
{
	(void)args;
	(void)position;
	logger(DEBUG, no_error, "Generating syscall %s", name);

	const uint32_t opcode = instruction.opcode;
	const uint32_t funct3 = instruction.funct3;
	const uint32_t funct7 = instruction.funct7;

	assert(instruction.sz == 4);

	struct bytecode_t res = {
		.size = 4,
		.data = xmalloc(4),
	};
	*(uint32_t *)res.data = opcode | (funct3 << 12) | (funct7 << 20);
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

struct bytecode_t form_load_pseudo(const char *name, struct idata_t instruction,
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
