
#include <sys/types.h>

extern "C" {
#define _KERNEL 1
#include <kern_include/vm/uma.h>
}

struct uma_zone
{
	size_t size;
	uma_ctor ctor;
	uma_dtor dtor;
	uma_init init;
	uma_fini fini;
};

extern "C" void *
uma_zalloc_arg(uma_zone_t zone, void * arg, int flags)
{
	void * mem = ::operator new(zone->size);
	if (mem == NULL)
		return (NULL);

	if (zone->init != NULL)
		zone->init(mem, zone->size, flags);
	
	if (zone->ctor != NULL)
		zone->ctor(mem, zone->size, arg, flags);

	return (mem);
}

extern "C" void
uma_zfree_arg(uma_zone_t zone, void *mem, void *arg)
{

	if (zone->dtor != NULL)
		zone->dtor(mem, zone->size, arg);

	if (zone->fini != NULL)
		zone->fini(mem, zone->size);

	::operator delete(mem);
}
