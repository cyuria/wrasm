
#include "form/generic.h"

#include <assert.h>
#include <string.h>

#include "debug.h"
#include "symbols.h"
#include "xmalloc.h"

const struct bytecode error_bytecode = { .size = (size_t)-1, .data = NULL };

int32_t calc_symbol_offset(const struct symbol *sym, size_t position)
{
	const size_t sympos = calc_fileoffset((struct sectionpos){
		.section = sym->section,
		.offset = sym->value,
	});
	return (int32_t)(sympos - position);
}

struct bytecode form_empty_bytecode(void)
{
	logger(DEBUG, no_error, "Generating empty bytecode");
	return (struct bytecode){ .size = 0, .data = NULL };
}

struct bytecode form_rtype(const char *name, struct idata instruction,
			   struct args args, size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating R type instruction %s", name);

	const uint32_t opcode = instruction.opcode;
	const uint32_t rd = args.rd;
	const uint32_t funct3 = instruction.funct3;
	const uint32_t rs1 = args.rs1;
	const uint32_t rs2 = args.rs2;
	const uint32_t funct7 = instruction.funct7;

	assert(instruction.sz == 4);

	struct bytecode res = {
		.size = 4,
		.data = xmalloc(4),
	};
	*(uint32_t *)res.data = opcode | (rd << 7) | (funct3 << 12) |
				(rs1 << 15) | (rs2 << 20) | (funct7 << 25);
	return res;
}

struct bytecode form_itype(const char *name, struct idata instruction,
			   struct args args, size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating I type instruction %s", name);

	const uint32_t opcode = instruction.opcode;
	const uint32_t rd = args.rd;
	const uint32_t funct3 = instruction.funct3;
	const uint32_t rs1 = args.rs1;
	const uint32_t imm_11_0 = args.imm & 0xFFF;

	assert(instruction.sz == 4);

	struct bytecode res = {
		.size = 4,
		.data = xmalloc(4),
	};
	*(uint32_t *)res.data = opcode | (rd << 7) | (funct3 << 12) |
				(rs1 << 15) | (imm_11_0 << 20);
	return res;
}

struct bytecode form_itype2(const char *name, struct idata instruction,
			    struct args args, size_t position)
{
	logger(DEBUG, no_error, "Generating I type 2 instruction %s", name);
	struct bytecode res = form_itype(name, instruction, args, position);
	*(uint32_t *)res.data |= 0x40000000; /* set type 2 bit */
	return res;
}

struct bytecode form_stype(const char *name, struct idata instruction,
			   struct args args, size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating S type instruction %s", name);

	const uint32_t opcode = instruction.opcode;
	const uint32_t imm_4_0 = args.imm & 0x1F;
	const uint32_t funct3 = instruction.funct3;
	const uint32_t rs1 = args.rs1;
	const uint32_t rs2 = args.rs2;
	const uint32_t imm_11_5 = (args.imm >> 5) & 0x7F;

	assert(instruction.sz == 4);

	struct bytecode res = {
		.size = 4,
		.data = xmalloc(4),
	};
	*(uint32_t *)res.data = opcode | (imm_4_0 << 7) | (funct3 << 12) |
				(rs1 << 15) | (rs2 << 20) | (imm_11_5 << 25);
	return res;
}

struct bytecode form_btype(const char *name, struct idata instruction,
			   struct args args, size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating B type instruction %s", name);

	if (args.sym->type != SYMBOL_LABEL)
		logger(ERROR, error_invalid_syntax,
		       "Incorrect argument types for instruction %s."
		       " Expected label, but got a different symbol",
		       args.sym->name);

	const uint32_t offset =
		(uint32_t)calc_symbol_offset(args.sym, position);
	const uint32_t opcode = instruction.opcode;
	const uint32_t imm_11 = (offset >> 11) & 0x1;
	const uint32_t imm_4_1 = (offset >> 1) & 0xF;
	const uint32_t funct3 = instruction.funct3;
	const uint32_t rs1 = args.rs1;
	const uint32_t rs2 = args.rs2;
	const uint32_t imm_10_5 = (offset >> 5) & 0x3F;
	const uint32_t imm_12 = (offset >> 12) & 0x1;

	logger(DEBUG, no_error, "B type instruction has offset of 0x%.04X",
	       offset);

	assert(instruction.sz == 4);

	struct bytecode res = {
		.size = 4,
		.data = xmalloc(4),
	};
	*(uint32_t *)res.data = opcode | (imm_11 << 7) | (imm_4_1 << 8) |
				(funct3 << 12) | (rs1 << 15) | (rs2 << 20) |
				(imm_10_5 << 25) | (imm_12 << 31);
	return res;
}

struct bytecode form_utype(const char *name, struct idata instruction,
			   struct args args, size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating U type instruction %s", name);

	const uint32_t opcode = instruction.opcode;
	const uint32_t rd = args.rd & 0x1F;
	const uint32_t imm_12_31 = (args.imm >> 12) & 0xFFFFF;

	assert(instruction.sz == 4);

	struct bytecode res = {
		.size = 4,
		.data = xmalloc(4),
	};
	*(uint32_t *)res.data = opcode | (rd << 7) | (imm_12_31 << 12);
	return res;
}

struct bytecode form_jtype(const char *name, struct idata instruction,
			   struct args args, size_t position)
{
	(void)position;
	logger(DEBUG, no_error, "Generating J type instruction %s", name);

	int32_t offset = calc_symbol_offset(args.sym, position);
	logger(DEBUG, no_error, "Offset of J type instruction is 0x%x", offset);

	const uint32_t opcode = instruction.opcode;
	const uint32_t rd = args.rd & 0x1F;
	const uint32_t imm_19_12 = (offset >> 12) & 0xFF;
	const uint32_t imm_11 = (offset >> 11) & 0x1;
	const uint32_t imm_10_1 = (offset >> 1) & 0x3FF;
	const uint32_t imm_20 = (offset >> 20) & 0x1;

	assert(instruction.sz == 4);

	struct bytecode res = {
		.size = 4,
		.data = xmalloc(4),
	};
	*(uint32_t *)res.data = opcode | (rd << 7) | (imm_19_12 << 12) |
				(imm_11 << 20) | (imm_10_1 << 21) |
				(imm_20 << 31);
	return res;
}
