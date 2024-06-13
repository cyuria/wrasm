
#include "form/csr.h"

#include "form/generic.h"
#include "parse.h"

const struct formation zicsr[] = {
	{ "csrrw", &form_itype, &parse_csr, { 4, OP_SYSTEM, 0x1, 0 } },
	{ "csrrs", &form_itype, &parse_csr, { 4, OP_SYSTEM, 0x2, 0 } },
	{ "csrrc", &form_itype, &parse_csr, { 4, OP_SYSTEM, 0x3, 0 } },
	{ "csrrwi", &form_itype, &parse_csri, { 4, OP_SYSTEM, 0x5, 0 } },
	{ "csrrsi", &form_itype, &parse_csri, { 4, OP_SYSTEM, 0x6, 0 } },
	{ "csrrci", &form_itype, &parse_csri, { 4, OP_SYSTEM, 0x7, 0 } },

	END_FORMATION
};
