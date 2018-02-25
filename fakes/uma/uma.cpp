
extern "C" {
#define _KERNEL_UT 1
#include <kern_include/sys/types.h>
#include <kern_include/sys/systm.h>
#include <kern_include/vm/uma.h>
#include <kern_include/vm/uma_dbg.h>
}

#include <gtest/gtest.h>

struct uma_zone
{
	const char *name;
	size_t size;
	uma_ctor ctor;
	uma_dtor dtor;
	uma_init init;
	uma_fini fini;
	size_t alloced;
};

uma_zone_t
uma_zcreate(const char *name, size_t size, uma_ctor ctor,
		    uma_dtor dtor, uma_init uminit, uma_fini fini,
		    int align, uint32_t flags)
{
	struct uma_zone * zone = new uma_zone;

	zone->name = name;
	zone->size = size;
	zone->ctor = ctor;
	zone->dtor = dtor;
	zone->init = uminit;
	zone->fini = fini;
	zone->alloced = 0;

	return (zone);
}

void
uma_zdestroy(uma_zone_t zone)
{
	EXPECT_EQ(zone->alloced, 0) << "Leaked memory from uma zone " << zone->name;
	delete zone;
}

void *
uma_zalloc_arg(uma_zone_t zone, void * arg, int flags)
{
	void * mem = ::operator new(zone->size);
	if (mem == NULL)
		return (NULL);

	if (zone->init != NULL)
		zone->init(mem, zone->size, flags);
	
	if (zone->ctor != NULL)
		zone->ctor(mem, zone->size, arg, flags);

	zone->alloced++;

	return (mem);
}

void
uma_zfree_arg(uma_zone_t zone, void *mem, void *arg)
{
	EXPECT_GT(zone->alloced, 0) << "Unexpected uma_zfree_arg call on uma zone "
	   << zone-> name << " (possibly due to double free)";
	zone->alloced--;

	if (zone->dtor != NULL)
		zone->dtor(mem, zone->size, arg);

	if (zone->fini != NULL)
		zone->fini(mem, zone->size);

	::operator delete(mem);
}

static const uint32_t uma_junk = 0xdeadc0de;

/*
 * Checks an item to make sure it hasn't been overwritten since it was freed,
 * prior to subsequent reallocation.
 *
 * Complies with standard ctor arg/return
 */
int
trash_ctor(void *mem, int size, void *arg, int flags)
{
	int cnt;
	uint32_t *p;

	cnt = size / sizeof(uma_junk);

	for (p = static_cast<uint32_t*>(mem); cnt > 0; cnt--, p++)
		if (*p != uma_junk) {
			panic("Memory modified after free %p(%d) val=%x @ %p\n",
			    mem, size, *p, p);
			return (0);
		}
	return (0);
}

/*
 * Fills an item with predictable garbage
 *
 * Complies with standard dtor arg/return
 *
 */
void
trash_dtor(void *mem, int size, void *arg)
{
	int cnt;
	uint32_t *p;

	cnt = size / sizeof(uma_junk);

	for (p = static_cast<uint32_t*>(mem); cnt > 0; cnt--, p++)
		*p = uma_junk;
}

/*
 * Fills an item with predictable garbage
 *
 * Complies with standard init arg/return
 *
 */
int
trash_init(void *mem, int size, int flags)
{
	trash_dtor(mem, size, NULL);
	return (0);
}

/*
 * Checks an item to make sure it hasn't been overwritten since it was freed.
 *
 * Complies with standard fini arg/return
 *
 */
void
trash_fini(void *mem, int size)
{
	(void)trash_ctor(mem, size, NULL, 0);
}

