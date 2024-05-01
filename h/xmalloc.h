#pragma once

#include <stdlib.h>

#define xmalloc wrasm_xmalloc
#define xcalloc wrasm_xcalloc
#define xrealloc wrasm_xrealloc

void *xmalloc(size_t);
void *xcalloc(size_t, size_t);
void *xrealloc(void *, size_t);
