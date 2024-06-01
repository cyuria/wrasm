#pragma once

#include "elf/output.h"
#include "form/instructions.h"

extern const struct args_t empty_args;

arg_parser parse_rtype;
arg_parser parse_itype;
arg_parser parse_stype;
arg_parser parse_utype;
arg_parser parse_btype;
arg_parser parse_bztype;
arg_parser parse_ltype;
arg_parser parse_pseudo;
arg_parser parse_fence;
arg_parser parse_none;

arg_parser parse_jal;
arg_parser parse_jalr;
arg_parser parse_ret;
arg_parser parse_li;
arg_parser parse_la;
arg_parser parse_j;
arg_parser parse_jr;
arg_parser parse_ftso;

int parse_asm(const char *, struct sectionpos_t);
