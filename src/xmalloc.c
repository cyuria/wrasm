
#include "xmalloc.h"

#include <stdlib.h>

#include "debug.h"

static void die(const char *function)
{
	logger(CRITICAL, error_system, "Call to %s failed", function);
	exit(EXIT_FAILURE);
}

void *xmalloc(size_t sz)
{
	void *ptr = malloc(sz);
	if (!ptr)
		die("malloc");
	return ptr;
}

void *xcalloc(size_t nitems, size_t size)
{
	void *ptr = calloc(nitems, size);
	if (!ptr)
		die("calloc");
	return ptr;
}

void *xrealloc(void *ptr, size_t size)
{
	ptr = realloc(ptr, size);
	if (!ptr)
		die("xrealloc");
	return ptr;
}
