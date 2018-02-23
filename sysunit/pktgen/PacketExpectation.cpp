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

#include "pktgen/PacketExpectation.h"

#include <kern_include/sys/types.h>
#include <kern_include/netinet/in.h>

namespace PktGen
{
	void PacketExpectation::operator()(mbuf * m) const
	{
		TestExpectations(m);
		ASSERT_TRUE(m->m_flags & M_PKTHDR);

		auto hdrlen = GetHeaderLen(m);

		ASSERT_GE(m->m_pkthdr.len, hdrlen);
		m_adj(m, hdrlen);
	}

	uint8_t PacketExpectation::ntoh(uint8_t x)
	{
		return x;
	}

	uint16_t PacketExpectation::ntoh(uint16_t x)
	{
		return ntohs(x);
	}

	uint32_t PacketExpectation::ntoh(uint32_t x)
	{
		return ntohl(x);
	}
}