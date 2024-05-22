#pragma once

#include <stdint.h>
#include <stdlib.h>

struct formation_t;
struct args_t;
struct idata_t;
typedef struct bytecode_t(form_handler)(const char *, struct idata_t,
					struct args_t, size_t);
typedef struct args_t arg_parser(char *);

struct formation_t {
	const char *name;
	form_handler *form_handler;
	arg_parser *arg_handler;
	struct idata_t {
		size_t sz;
		uint8_t opcode;
		uint8_t funct3;
		uint8_t funct7;
	} idata;
};

struct args_t {
	uint8_t rd;
	uint8_t rs1;
	uint8_t rs2;
	uint32_t imm;
	struct symbol_t *sym;
};

extern const struct args_t empty_args;

struct formation_t parse_form(const char *instruction);
