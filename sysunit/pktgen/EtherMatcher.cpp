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

#include "pktgen/EtherMatcher.h"

#include "pktgen/EtherFlow.h"

#include <netinet/in.h>

using testing::MatchResultListener;

namespace PktGen
{
	EtherMatcher::EtherMatcher(const EtherFlow& flow, uint16_t etype,
	    size_t offset)
	  : dst(flow.GetDst()),
	    src(flow.GetSrc()),
	    ethertype(etype),
	    headerOffset(offset)
	{
	}

	bool EtherMatcher::MatchAndExplain(mbuf* m,
	    MatchResultListener* listener) const
	{
		auto * eh = GetMbufHeader<ether_header>(m, headerOffset);

		EtherAddr pktDst(eh->ether_dhost);
		if (dst != pktDst) {
			*listener << "dst mac is " << pktDst << " (expected " << dst << ")";
			return false;
		}

		EtherAddr pktSrc(eh->ether_shost);
		if (src != pktSrc) {
			*listener << "src mac is " << pktSrc << " (expected " << src << ")";
			return false;
		}

		if (ethertype != ntohs(eh->ether_type)) {
			*listener << "ethertype is " << ntohs(eh->ether_type)
			    << " (expected " << ethertype << ")";
			return false;
		}

		return true;
	}

	void EtherMatcher::DescribeTo(::std::ostream* os) const
	{
		*os << "Ethernet header";
	}
}
