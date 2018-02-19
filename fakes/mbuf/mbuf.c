
#define _KERNEL 1

#include <sys/types.h>
#include <sys/systm.h>
#include <sys/lock.h>
#include <kern_include/sys/mbuf.h>
#include <kern_include/vm/uma.h>

uma_zone_t zone_mbuf;
uma_zone_t zone_pack;

struct mbuf *
m_getm2(struct mbuf *m, int len, int how, short type, int flags)
{
	MPASS (false);
	return NULL;
}

int
m_tag_copy_chain(struct mbuf *to, const struct mbuf *from, int how)
{
	MPASS (SLIST_EMPTY(&from->m_pkthdr.tags));
}

void
m_tag_delete_chain(struct mbuf *m, struct m_tag *t)
{
	MPASS (t == NULL);
	MPASS (SLIST_EMPTY(&m->m_pkthdr.tags));
}

void
m_freem(struct mbuf *m)
{
	while (m != NULL)
		m = m_free(m);
}

void
mb_free_ext(struct mbuf *m)
{
	MPASS(false);
}
