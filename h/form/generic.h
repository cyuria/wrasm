#pragma once
#include <stdint.h>

#include "symbols.h"
#include "form/instructions.h"

#define OP_LOAD 0x03
#define OP_STORE 0x23
#define OP_BRANCH 0x63
#define OP_JAL 0x6F
#define OP_JALR 0x67
#define OP_MISC_MEM 0x0F
#define OP_OP 0x33
#define OP_OPI 0x13
#define OP_OP32 0x3B
#define OP_OPI32 0x1B
#define OP_SYSTEM 0x73
#define OP_LUI 0x37
#define OP_AUIPC 0x17
#define OP_AMO 0x2F

#define END_FORMATION              \
	{                          \
		NULL, NULL, NULL,  \
		{                  \
			0, 0, 0, 0 \
		}                  \
	}

struct bytecode {
	size_t size;
	unsigned char *data;
};

extern const struct bytecode error_bytecode;

/* helper functions */
struct bytecode form_empty_bytecode(void);

int32_t calc_symbol_offset(const struct symbol *, size_t);

form_handler form_rtype;
form_handler form_itype;
form_handler form_itype2;
form_handler form_stype;
form_handler form_btype;
form_handler form_utype;
form_handler form_jtype;
