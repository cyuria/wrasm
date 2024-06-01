#pragma once
#include "form/instructions.h"

extern const struct formation rv32i[];
extern const struct formation rv64i[];

enum load_pseudo {
	LOAD_IMM,
	LOAD_ADDR,
};
enum math_pseudo {
	MATH_MV,
	MATH_NOT,
	MATH_NEG,
	MATH_NEGW,
	MATH_SEXTW,
};
enum setif_pseudo {
	SETIF_EQZ,
	SETIF_NEZ,
	SETIF_LTZ,
	SETIF_GTZ,
};
enum branchifz_pseudo {
	BRANCHIFZ_EQZ,
	BRANCHIFZ_NEZ,
	BRANCHIFZ_LEZ,
	BRANCHIFZ_GEZ,
	BRANCHIFZ_LTZ,
	BRANCHIFZ_GTZ,
};
enum branchifr_pseudo {
	BRANCHIFR_GT = 0x4,
	BRANCHIFR_LE = 0x5,
	BRANCHIFR_GTU = 0x6,
	BRANCHIFR_LEU = 0x7,
};
enum jump_pseudo {
	JUMP_J,
	JUMP_JR,
	JUMP_RET,
};

/* shortcut instructions bytecode generation */
form_handler form_nop;
form_handler form_load_pseudo;
form_handler form_math;
form_handler form_setif;
form_handler form_branchifz;
form_handler form_branchifr;
form_handler form_jump;
form_handler form_jr;
form_handler form_ret;

/* basic integer instruction type bytecode generation */
form_handler form_syscall;

/* individual instruction bytecode generation */
form_handler form_jal;
form_handler form_jalr;
