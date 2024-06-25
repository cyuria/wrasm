
#include "form/fsingle.h"

#include "form/generic.h"
#include "parse.h"

const struct formation rv32f[] = {
	{ "FLW", &form_itype, &parse_fltype, { 4, OP_LOADF, 0x2, 0 } },
	END_FORMATION
};
