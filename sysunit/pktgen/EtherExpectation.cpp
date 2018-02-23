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

#define _KERNEL_UT_NO_USERLAND_CONFLICTS 1

#include "pktgen/EtherExpectation.h"

#include "pktgen/EtherFlow.h"

#include <netinet/in.h>

namespace PktGen
{
	EtherExpectation::EtherExpectation(const EtherFlow& flow, uint16_t etype)
	  : dst(flow.GetDst()),
	    src(flow.GetSrc()),
	    ethertype(etype)
	{
	}

	void EtherExpectation::TestExpectations(mbuf *m) const
	{
		auto * eh = mtod(m, struct ether_header*);
		EXPECT_EQ(dst, EtherAddr(eh->ether_dhost));
		EXPECT_EQ(src, EtherAddr(eh->ether_shost));
		EXPECT_EQ(ethertype, ntohs(eh->ether_type));
	}

	size_t EtherExpectation::GetHeaderLen(mbuf *m) const
	{
		return sizeof(struct ether_header);
	}
}
