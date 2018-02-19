
extern "C" {
#define _KERNEL 1

#include <sys/types.h>
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
	return ::operator new(size);
}

extern "C" void
kfree(void *mem, struct malloc_type *mtp)
{
	::operator delete(mem);
}


