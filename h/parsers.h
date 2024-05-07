#pragma once
#include <stdint.h>
#include <stdlib.h>

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

#define RV64I_SIZE 4
#define RV64M_SIZE 4
#define RV64A_SIZE 4
#define RV64F_SIZE 4
#define RV64D_SIZE 4
#define RV64C_SIZE 2
#define RV64Z_SIZE 4

enum argtype_e {
	arg_none,
	arg_immediate,
	arg_register,
	arg_symbol,
};

struct args_t {
	enum argtype_e type[3];
	size_t arg[3];
};

struct bytecode_t {
	size_t size;
	unsigned char *data;
};

struct parser_t;
typedef struct bytecode_t(parser)(struct parser_t, struct args_t, size_t);

struct parser_t {
	const char *name;
	size_t isize;
	parser *handler;
	uint8_t opcode;
	uint8_t funct1;
	uint16_t funct2;
};

extern const struct bytecode_t error_bytecode;

/* parser sets */
extern const struct parser_t rv64s[]; /* Shortcuts */
extern const struct parser_t rv64i[];
extern const struct parser_t rv64m[];
extern const struct parser_t rv64a[];
extern const struct parser_t rv64f[];
extern const struct parser_t rv64d[];
extern const struct parser_t rv64c[];
extern const struct parser_t rv64z[]; /* zifencei and zicsr extensions */

struct bytecode_t gen_empty_bytecode(void);

/* shortcut instructions bytecode generation */
parser gen_nop;
parser gen_load_short;
parser gen_math;
parser gen_setif;
parser gen_single; // implement with float extension
parser gen_double; // ^^^
parser gen_branchif;
parser gen_jump;
parser gen_ret;

/* basic integer instruction type bytecode generation */
parser gen_rtype;
parser gen_itype;
parser gen_itype2;
parser gen_stype;
parser gen_btype;
parser gen_utype;
parser gen_jtype;
parser gen_syscall;
parser gen_fence;
