
#define _KERNEL 1

#include <stdint.h>

typedef unsigned u_int;

extern "C" {
#include <kern_include/sys/_lock.h>
#include <kern_include/sys/_mutex.h>
}

#undef _KERNEL
#include <gtest/gtest.h>

#define	mtxlock2mtx(c)	(__containerof(c, struct mtx, mtx_lock))

#define MTX_UNOWNED 0

extern "C" void 
__mtx_lock_sleep(volatile uintptr_t *c, int opts, const char *file,
	    int line)
{
	struct mtx *m;

	m = mtxlock2mtx(c);

	ASSERT_EQ(m->mtx_lock, MTX_UNOWNED);
	m->mtx_lock++;
}

extern "C" void
__mtx_unlock_sleep(volatile uintptr_t *c, int opts, const char *file,
	    int line)
{
	struct mtx *m;

	m = mtxlock2mtx(c);

	ASSERT_NE(m->mtx_lock, MTX_UNOWNED);
	m->mtx_lock--;
}
