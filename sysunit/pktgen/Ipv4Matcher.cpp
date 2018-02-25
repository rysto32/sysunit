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

#include "pktgen/Ipv4Matcher.h"
#include "pktgen/Ipv4Header.h"

#include "pktgen/Ipv4Flow.h"

extern "C" {
#include "kern_include/sys/types.h"
#include "kern_include/netinet/in.h"
#include "kern_include/netinet/ip.h"
}

using testing::MatchResultListener;

bool operator!=(in_addr a, in_addr b)
{
	return a.s_addr != b.s_addr;
}

namespace PktGen
{
	Ipv4Matcher::Ipv4Matcher(const Ipv4Template & h, size_t offset)
	  : header(h),
	    headerOffset(offset)
	{
	}

	#define	CheckField(hdr, field, expect) do { \
		if (ntoh((hdr)->field) != (expect)) { \
			*listener << "IPv4: " << #field << " field is " << ntoh((hdr)->field) \
			    << " (expected " << expect << ")";; \
			return false; \
		} \
	} while (0)

	bool Ipv4Matcher::MatchAndExplain(mbuf* m, MatchResultListener* listener) const
	{
		auto * ip = GetMbufHeader<struct ip>(m, headerOffset);

		CheckField(ip, ip_hl, header.GetHeaderLen());
		CheckField(ip, ip_v, 4);
		CheckField(ip, ip_tos, header.GetTos());
		CheckField(ip, ip_len, header.GetIpLen());
		CheckField(ip, ip_id, header.GetId());
		CheckField(ip, ip_off, header.GetOff());
		CheckField(ip, ip_ttl, header.GetTtl());
		CheckField(ip, ip_p, header.GetProto());

		// XXX tcp_lro always updates this...
		if (0)
			CheckField(ip, ip_sum, header.GetChecksum());

		// The IPs are stored in network byte order so
		// there is no need to byte-swap them before comparing
		if (header.GetSrc() != ip->ip_src) {
			*listener << "IPv4: srcip field is " << ip->ip_src
			    << " (expected " << header.GetSrc() << ")";
			return false;
		}

		if (header.GetDst() != ip->ip_dst) {
			*listener << "IPv4: dstip field is " << ip->ip_dst
			    << " (expected " << header.GetDst() << ")";
			return false;
		}

		uint32_t expectedFlag = 0;
		if (header.GetChecksumVerified())
			expectedFlag = CSUM_L3_CALC;

		auto actualFlag = m->m_pkthdr.csum_flags & CSUM_L3_CALC;
		if (actualFlag != expectedFlag) {
			*listener << "IPv4: l3 csum calc flag is " << actualFlag
			    << " (expected " << expectedFlag << ")";
			return false;
		}

		expectedFlag = 0;
		if (header.GetChecksumPassed())
			expectedFlag = CSUM_L3_VALID;

		actualFlag = m->m_pkthdr.csum_flags & CSUM_L3_VALID;
		if (actualFlag != expectedFlag) {
			*listener << "IPv4: l3 csum valid flag is " << actualFlag
			    << " (expected " << expectedFlag << ")";
			return false;
		}

		return true;
	}

	void Ipv4Matcher::DescribeTo(::std::ostream* os) const
	{
		*os << "IPv4";
	}
}
