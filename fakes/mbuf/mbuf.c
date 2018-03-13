/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2018 Ryan Stone
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define _KERNEL_UT 1

#include <kern_include/sys/types.h>
#include <kern_include/sys/queue.h>
#include <kern_include/sys/kernel.h>
#include <kern_include/sys/systm.h>
#include <kern_include/sys/lock.h>
#include <kern_include/sys/malloc.h>
#include <kern_include/sys/mbuf.h>
#include <kern_include/vm/uma.h>
#include <kern_include/vm/uma_dbg.h>

#include <fake/mbuf.h>

uma_zone_t zone_mbuf;
uma_zone_t zone_pack;
uma_zone_t zone_clust;
uma_zone_t zone_jumbop;
uma_zone_t zone_jumbo9;
uma_zone_t zone_jumbo16;

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
	return (0);
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
	while (m != NULL) {
		m = m_free(m);
	}
}

void
mb_free_ext(struct mbuf *m)
{
	volatile u_int *refcnt;
	struct mbuf *mref;
	int freembuf;

	KASSERT(m->m_flags & M_EXT, ("%s: M_EXT not set on %p", __func__, m));

	/* See if this is the mbuf that holds the embedded refcount. */
	if (m->m_ext.ext_flags & EXT_FLAG_EMBREF) {
		refcnt = &m->m_ext.ext_count;
		mref = m;
	} else {
		KASSERT(m->m_ext.ext_cnt != NULL,
		    ("%s: no refcounting pointer on %p", __func__, m));
		refcnt = m->m_ext.ext_cnt;
		mref = __containerof(refcnt, struct mbuf, m_ext.ext_count);
	}

	/*
	 * Check if the header is embedded in the cluster.  It is
	 * important that we can't touch any of the mbuf fields
	 * after we have freed the external storage, since mbuf
	 * could have been embedded in it.  For now, the mbufs
	 * embedded into the cluster are always of type EXT_EXTREF,
	 * and for this type we won't free the mref.
	 */
	if (m->m_flags & M_NOFREE) {
		freembuf = 0;
		KASSERT(m->m_ext.ext_type == EXT_EXTREF,
		    ("%s: no-free mbuf %p has wrong type", __func__, m));
	} else
		freembuf = 1;

	/* Free attached storage if this mbuf is the only reference to it. */
	if (*refcnt == 1 || atomic_fetchadd_int(refcnt, -1) == 1) {
		switch (m->m_ext.ext_type) {
		case EXT_PACKET:
			/* The packet zone is special. */
			if (*refcnt == 0)
				*refcnt = 1;
			uma_zfree(zone_pack, mref);
			break;
		case EXT_CLUSTER:
			uma_zfree(zone_clust, m->m_ext.ext_buf);
			uma_zfree(zone_mbuf, mref);
			break;
		case EXT_JUMBOP:
			uma_zfree(zone_jumbop, m->m_ext.ext_buf);
			uma_zfree(zone_mbuf, mref);
			break;
		case EXT_JUMBO9:
			uma_zfree(zone_jumbo9, m->m_ext.ext_buf);
			uma_zfree(zone_mbuf, mref);
			break;
		case EXT_JUMBO16:
			uma_zfree(zone_jumbo16, m->m_ext.ext_buf);
			uma_zfree(zone_mbuf, mref);
			break;
		case EXT_SFBUF:
		case EXT_NET_DRV:
		case EXT_MOD_TYPE:
		case EXT_DISPOSABLE:
			KASSERT(mref->m_ext.ext_free != NULL,
				("%s: ext_free not set", __func__));
			mref->m_ext.ext_free(mref);
			uma_zfree(zone_mbuf, mref);
			break;
		case EXT_EXTREF:
			KASSERT(m->m_ext.ext_free != NULL,
				("%s: ext_free not set", __func__));
			m->m_ext.ext_free(m);
			break;
		default:
			KASSERT(m->m_ext.ext_type == 0,
				("%s: unknown ext_type", __func__));
		}
	}

	if (freembuf && m != mref)
		uma_zfree(zone_mbuf, m);
}

void
m_extadd(struct mbuf *mb, char *buf, u_int size, m_ext_free_t freef,
    void *arg1, void *arg2, int flags, int type)
{

	KASSERT(type != EXT_CLUSTER, ("%s: EXT_CLUSTER not allowed", __func__));

	mb->m_flags |= (M_EXT | flags);
	mb->m_ext.ext_buf = buf;
	mb->m_data = mb->m_ext.ext_buf;
	mb->m_ext.ext_size = size;
	mb->m_ext.ext_free = freef;
	mb->m_ext.ext_arg1 = arg1;
	mb->m_ext.ext_arg2 = arg2;
	mb->m_ext.ext_type = type;

	if (type != EXT_EXTREF) {
		mb->m_ext.ext_count = 1;
		mb->m_ext.ext_flags = EXT_FLAG_EMBREF;
	} else
		mb->m_ext.ext_flags = 0;
}

MALLOC_DEFINE(M_SYSUNIT_MBUF, "sysunit_mbuf", "sysunit mbuf ext_buf allocations");

static void m_ext_free_malloc(struct mbuf *m)
{
	free(m->m_ext.ext_buf, M_SYSUNIT_MBUF);
}

struct mbuf *
alloc_mbuf(size_t len)
{
	struct mbuf *m;
	void *ext;

	m = m_gethdr(M_WAITOK, MT_DATA);
	if (len <= MHLEN)
		return (m);

	ext = malloc(len, M_SYSUNIT_MBUF, M_WAITOK);

	// Fill the mbuf with junk
	trash_dtor(ext, len, NULL);

	m_extadd(m, ext, len, m_ext_free_malloc, NULL, NULL, 0, EXT_MOD_TYPE);
	return (m);
}
