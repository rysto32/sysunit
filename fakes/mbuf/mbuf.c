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
#include <kern_include/sys/systm.h>
#include <kern_include/sys/lock.h>
#include <kern_include/sys/mbuf.h>
#include <kern_include/vm/uma.h>

#include <fake/mbuf.h>

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
	MPASS(false);
}

struct mbuf *
alloc_mbuf(size_t len)
{
	MPASS (len < MHLEN);

	return m_gethdr(M_WAITOK, MT_DATA);
}
