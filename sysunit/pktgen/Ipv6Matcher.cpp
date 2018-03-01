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

#include "pktgen/Ipv6.h"

extern "C" {
#include "kern_include/sys/types.h"
#include "kern_include/netinet/in.h"
#include "kern_include/netinet/ip6.h"
}

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

#include "pktgen/Ipv6.h"

extern "C" {
#include "kern_include/sys/types.h"
#include "kern_include/netinet/in.h"
#include "kern_include/netinet/ip.h"
}

using testing::MatchResultListener;

namespace PktGen
{
	Ipv6Matcher::Ipv6Matcher(UnnestedIpv6Template && h, size_t offset)
	  : header(h),
	    headerOffset(offset)
	{
	}

	#define	CheckField(field, expect, name) do { \
		if ((field) != (expect)) { \
			*listener << "IPv6: " name " field is " << (int)(field) \
			    << " (expected " << (int)expect << ")";; \
			return false; \
		} \
	} while (0)

	bool Ipv6Matcher::MatchAndExplain(mbuf* m, MatchResultListener* listener) const
	{
		auto * ip6 = GetMbufHeader<ip6_hdr>(m, headerOffset);

		CheckField(ip6->ip6_vfc & 0xfU, header.GetClass(), "ip6_class");
		CheckField((ip6->ip6_vfc & uint32_t(IPV6_VERSION_MASK)) >> 4, header.GetVersion(), "ip6_version");
		CheckField(ip6->ip6_flow & uint32_t(IPV6_FLOWINFO_MASK), header.GetFlow(), "ip6_flow");
		CheckField(ntoh(ip6->ip6_plen), header.GetPayloadLength(), "ip6_plen");
		CheckField(ntoh(ip6->ip6_nxt), header.GetProto(), "ip6_nxt");
		CheckField(ntoh(ip6->ip6_hlim), header.GetHopLimit(), "ip6_hlim");

		// The IPs are stored in network byte order so
		// there is no need to byte-swap them before comparing
		if (header.GetSrc() != ip6->ip6_src) {
			*listener << "IPv6: src field is " << ip6->ip6_src
			    << " (expected " << header.GetSrc() << ")";
			return false;
		}

		if (header.GetDst() != ip6->ip6_dst) {
			*listener << "IPv6: dst field is " << ip6->ip6_dst
			    << " (expected " << header.GetDst() << ")";
			return false;
		}

		return true;
	}

	void Ipv6Matcher::DescribeTo(::std::ostream* os) const
	{
		*os << "IPv6";
	}
}
