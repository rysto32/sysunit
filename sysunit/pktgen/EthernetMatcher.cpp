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

#include "fake/mbuf.h"

#include "pktgen/Ethernet.h"

#include <netinet/in.h>

using testing::MatchResultListener;

namespace PktGen::internal
{
	EthernetMatcher::EthernetMatcher(const EthernetTemplate & h, size_t off)
	  : header(h),
	    headerOffset(off)
	{
	}

	bool EthernetMatcher::MatchAndExplain(mbuf* m,
	    MatchResultListener* listener) const
	{
		auto * eh = GetMbufHeader<ether_header>(m, headerOffset);

		EtherAddr pktDst(eh->ether_dhost);
		if (header.GetDst() != pktDst) {
			*listener << "Ethernet: dst mac is " << pktDst
			    << " (expected " << header.GetDst() << ")";
			return false;
		}

		EtherAddr pktSrc(eh->ether_shost);
		if (header.GetSrc() != pktSrc) {
			*listener << "Ethernet: src mac is " << pktSrc
			    << " (expected " << header.GetSrc() << ")";
			return false;
		}

		if (header.GetEthertype() != ntohs(eh->ether_type)) {
			*listener << "Ethernet: ethertype is " << std::hex << ntohs(eh->ether_type)
			    << " (expected " << header.GetEthertype() << ")";
			return false;
		}

		uint16_t tag = header.GetMbufVlan();
		if (tag == 0) {
			if (m->m_flags & M_VLANTAG) {
				*listener << "Ethernet: M_VLANTAG is set on mbuf (expected no tag)";
				return false;
			}
		} else {
			if (!(m->m_flags & M_VLANTAG)) {
				*listener << "Ethernet: M_VLANTAG is not set on mbuf"
				    << " (expected tag " << tag << ")";
				return false;
			}

			if (tag != m->m_pkthdr.ether_vtag) {
				*listener << "Ethernet: mbuf ether_vlan is " << m->m_pkthdr.ether_vtag
				    << " (expected tag " << tag << ")";
				return false;
			}
		}

		return true;
	}

	void EthernetMatcher::DescribeTo(::std::ostream* os) const
	{
		*os << "Ethernet";
	}
}
