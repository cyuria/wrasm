#pragma once

#include <stdint.h>
#include <stdlib.h>

struct formation;
struct args;
struct idata;
typedef struct bytecode(form_handler)(const char *, struct idata, struct args,
				      size_t);
typedef struct args arg_parser(char *);

struct formation {
	const char *name;
	form_handler *form_handler;
	arg_parser *arg_handler;
	struct idata {
		size_t sz;
		uint8_t opcode;
		uint8_t funct3;
		uint8_t funct7;
	} idata;
};

struct args {
	uint8_t rd;
	uint8_t rs1;
	uint8_t rs2;
	int32_t imm;
	struct symbol *sym;
};

extern const struct args empty_args;

struct formation parse_form(const char *instruction);
