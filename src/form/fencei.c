
#include "form/fencei.h"

#include "form/generic.h"
#include "parse.h"

const struct formation zifencei[] = {
	{ "fence.i", &form_itype, &parse_none, { 4, OP_MISC_MEM, 0x1, 0 } },

	END_FORMATION
};
