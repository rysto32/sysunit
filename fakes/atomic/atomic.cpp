
extern "C" {
#include <sys/types.h>
#include <kern_include/machine/atomic.h>
}

void
atomic_add_int(volatile u_int *p, u_int v)
{
	*p += v;
}

u_int
atomic_fetchadd_int(volatile u_int *p, u_int v)
{
	u_int val = *p;
	*p += v;
	return val;
}
