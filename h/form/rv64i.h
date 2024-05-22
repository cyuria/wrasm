#pragma once
#include "form/generic.h"

/* shortcut instructions bytecode generation */
form_handler form_nop;
form_handler form_load_short;
form_handler form_math;
form_handler form_setif;
form_handler form_branchifz;
form_handler form_branchifr;
form_handler form_jump;
form_handler form_jr;
form_handler form_ret;

/* basic integer instruction type bytecode generation */
form_handler form_rtype;
form_handler form_itype;
form_handler form_itype2;
form_handler form_stype;
form_handler form_btype;
form_handler form_utype;
form_handler form_jtype;
form_handler form_syscall;
form_handler form_fence;

/* individual instruction bytecode generation */
form_handler form_jal;
form_handler form_jalr;
