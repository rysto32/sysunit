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

#ifndef PKTGEN_IPV4_EXPECTATION_H
#define PKTGEN_IPV4_EXPECTATION_H

#include "fake/mbuf.h"

extern "C" {
#include <kern_include/sys/types.h>
#include <kern_include/netinet/in.h>
#include <kern_include/netinet/ip.h>
}

#include <gmock/gmock-matchers.h>

namespace PktGen
{
	class Ipv4Flow;

	class Ipv4Matcher : public testing::MatcherInterface<mbuf*>
	{
	private:
		size_t headerOffset;
		uint8_t header_len;
		uint8_t tos;
		uint16_t len;
		uint16_t id;
		uint16_t off;
		uint8_t ttl;
		uint8_t proto;
		uint16_t sum;
		struct in_addr src;
		struct in_addr dst;

	public:
		Ipv4Matcher(const Ipv4Flow & flow, uint8_t proto, uint16_t ip_len, size_t offset);

		virtual bool MatchAndExplain(mbuf*,
                    testing::MatchResultListener* listener) const override;

		virtual void DescribeTo(::std::ostream* os) const override;
	};

	inline testing::Matcher<mbuf*> Ipv4Header(const Ipv4Flow & flow, uint8_t proto, uint16_t ip_len, size_t offset)
	{
		return testing::MakeMatcher(new Ipv4Matcher(flow, proto, ip_len, offset));
	}
}

#endif
