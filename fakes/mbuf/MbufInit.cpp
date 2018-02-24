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

extern "C" {
#define _KERNEL_UT 1

#include <kern_include/vm/uma.h>
#include <kern_include/sys/param.h>
#include <kern_include/sys/lock.h>
#include <kern_include/sys/mbuf.h>
#include <kern_include/vm/uma_dbg.h>
}

#include "MbufInit.h"

#include <stdexcept>

uma_zone_t zone_mbuf;
uma_zone_t zone_pack;

/*
 * Constructor for Mbuf master zone.
 *
 * The 'arg' pointer points to a mb_args structure which
 * contains call-specific information required to support the
 * mbuf allocation API.  See mbuf.h.
 */
static int
mb_ctor_mbuf(void *mem, int size, void *arg, int how)
{
	struct mbuf *m;
	struct mb_args *args;
	int error;
	int flags;
	short type;

	trash_ctor(mem, size, arg, how);
	args = (struct mb_args *)arg;
	type = args->type;

	/*
	 * The mbuf is initialized later.  The caller has the
	 * responsibility to set up any MAC labels too.
	 */
	if (type == MT_NOINIT)
		return (0);

	m = (struct mbuf *)mem;
	flags = args->flags;
	MPASS((flags & M_NOFREE) == 0);

	error = m_init(m, how, type, flags);

	return (error);
}

/*
 * The Mbuf master zone destructor.
 */
static void
mb_dtor_mbuf(void *mem, int size, void *arg)
{
	struct mbuf *m;
	unsigned long flags;

	m = (struct mbuf *)mem;
	flags = (unsigned long)arg;

	KASSERT((m->m_flags & M_NOFREE) == 0, ("%s: M_NOFREE set", __func__));
	if (!(flags & MB_DTOR_SKIP) && (m->m_flags & M_PKTHDR) && !SLIST_EMPTY(&m->m_pkthdr.tags))
		m_tag_delete_chain(m, NULL);
	trash_dtor(mem, size, arg);
}

void
MbufInit::SetUp()
{
	if (zone_mbuf != NULL)
		throw std::runtime_error("Previous testcase did not TearDown fake_mbuf");

	zone_mbuf = uma_zcreate(MBUF_MEM_NAME, MSIZE,
	    mb_ctor_mbuf, mb_dtor_mbuf,
	    trash_init, trash_fini,
	    MSIZE - 1, UMA_ZONE_MAXBUCKET);
}

void
MbufInit::TearDown()
{
	uma_zdestroy(zone_mbuf);
	zone_mbuf = NULL;
}

static MbufInit mbufInit;

