
#include "form/rv64i.h"

#include "form/generic.h"
#include "form/rv32i.h"
#include "parse.h"

const struct formation_t rv64i[] = {
	{ "negw", &form_math, &parse_pseudo, { 4, math_negw, 0, 0 } },
	{ "sext.w", &form_math, &parse_pseudo, { 4, math_sextw, 0, 0 } },

	{ "addw", &form_rtype, &parse_rtype, { 4, OP_OP32, 0x0, 0x00 } },
	{ "addiw", &form_itype, &parse_itype, { 4, OP_OPI32, 0x0, 0 } },
	{ "subw", &form_rtype, &parse_rtype, { 4, OP_OP32, 0x0, 0x20 } },

	{ "slli", &form_itype, &parse_itype, { 4, OP_OPI, 0x1, 0x00 } },
	{ "sllw", &form_rtype, &parse_rtype, { 4, OP_OP32, 0x1, 0x00 } },
	{ "slliw", &form_itype, &parse_itype, { 4, OP_OPI32, 0x0, 0x00 } },
	{ "srli", &form_itype, &parse_itype, { 4, OP_OPI, 0x5, 0x00 } },
	{ "srlw", &form_rtype, &parse_rtype, { 4, OP_OP32, 0x5, 0x00 } },
	{ "srliw", &form_itype, &parse_itype, { 4, OP_OPI32, 0x5, 0 } },
	{ "srai", &form_itype2, &parse_itype, { 4, OP_OPI, 0x5, 0 } },
	{ "sraw", &form_rtype, &parse_rtype, { 4, OP_OP32, 0x5, 0x20 } },
	{ "sraiw", &form_itype2, &parse_itype, { 4, OP_OPI32, 0x5, 0 } },

	{ "lwu", &form_itype, &parse_itype, { 4, OP_LOAD, 0x6, 0 } },
	{ "ld", &form_itype, &parse_itype, { 4, OP_LOAD, 0x3, 0 } },
	{ "sd", &form_stype, &parse_stype, { 4, OP_STORE, 0x3, 0 } },

	END_FORMATION
};
