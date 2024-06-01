#pragma once
#include "form/instructions.h"

extern const struct formation_t rv32i[];
extern const struct formation_t rv64i[];

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
