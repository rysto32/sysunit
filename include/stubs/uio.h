#ifndef STUBS_UIO_H
#define STUBS_UIO_H

struct uio;

#include <cstdlib>

extern "C" int
uiomove(void *cp, int n, struct uio *uio)
{
	abort();
}

#endif
