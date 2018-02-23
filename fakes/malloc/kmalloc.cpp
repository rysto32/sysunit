
extern "C" {
#define _KERNEL_UT 1

#include <kern_include/sys/types.h>
#include <kern_include/sys/systm.h>
#include <kern_include/sys/lock.h>
#include <kern_include/sys/malloc.h>
}

void
malloc_init(void *data)
{
	MPASS(false);
}

void
malloc_uninit(void *data)
{
	MPASS(false);
}

extern "C" void *
kmalloc(size_t size, struct malloc_type *mtp, int flags)
{
	void * mem = ::operator new(size);

	if (flags & M_ZERO)
		memset(mem, 0, size);

	return (mem);
}

extern "C" void
kfree(void *mem, struct malloc_type *mtp)
{
	::operator delete(mem);
}


