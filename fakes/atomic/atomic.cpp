
extern "C" {
#include <sys/types.h>
#include <kern_include/machine/atomic.h>
}

void
atomic_add_int(volatile u_int *p, u_int v)
{
	*p += v;
}
