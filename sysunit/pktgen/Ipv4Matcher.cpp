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

#include "pktgen/Ipv4Matcher.h"

#include "pktgen/Ipv4Flow.h"

using testing::MatchResultListener;

bool operator!=(in_addr a, in_addr b)
{
	return a.s_addr != b.s_addr;
}

std::ostream & operator<<(std::ostream & os, in_addr a)
{
	os << (a.s_addr & 0xff) << "."
	   << ((a.s_addr >> 8) & 0xff) << "."
	   << ((a.s_addr >> 16) & 0xff) << "."
	   << ((a.s_addr >> 24) & 0xff);

	return os;
}

namespace PktGen
{
	Ipv4Matcher::Ipv4Matcher(const Ipv4Flow & flow, uint8_t proto, uint16_t ip_len, size_t offset)
	  : headerOffset(offset),
	    header_len(flow.GetHeaderSize() / sizeof(uint32_t)),
	    tos(flow.GetTos()),
	    len(ip_len),
	    id(flow.GetLastId()),
	    off(0),
	    ttl(flow.GetTtl()),
	    proto(proto),
	    sum(0),
	    src(flow.GetSrc()),
	    dst(flow.GetDst())
	{
	}

	#define	CheckField(field, expect) do { \
		if (ntoh(field) != (expect)) { \
			*listener << "IPv4: " << #expect << " field is " << ntoh(field) << " (expected " << expect << ")";; \
			return false; \
		} \
	} while (0)

	bool Ipv4Matcher::MatchAndExplain(mbuf* m, MatchResultListener* listener) const
	{
		auto * ip = GetMbufHeader<struct ip>(m, headerOffset);

		CheckField(ip->ip_hl, header_len);
		CheckField(ip->ip_v, static_cast<uint8_t>(4));
		CheckField(ip->ip_tos, tos);
		CheckField(ip->ip_len, len);
		CheckField(ip->ip_id, id);
		CheckField(ip->ip_off, off);
		CheckField(ip->ip_ttl, ttl);
		CheckField(ip->ip_p, proto);

		// XXX tcp_lro always updates this...
		if (0)
			CheckField(ip->ip_sum, sum);

		// The IPs are stored in network byte order so
		// there is no need to byte-swap them before comparing
		if (ip->ip_src != src) {
			*listener << "srcip field is " << ip->ip_src << " (expected " << src << ")";
			return false;
		}

		if (ip->ip_dst != dst) {
			*listener << "dstip field is " << ip->ip_dst << " (expected " << dst << ")";
			return false;
		}

		return true;
	}

	void Ipv4Matcher::DescribeTo(::std::ostream* os) const
	{
		*os << "IPv4 header";
	}
}
