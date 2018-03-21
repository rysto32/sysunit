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

#include "pktgen/Ipv6.h"

#include "pktgen/Packet.h"
#include "pktgen/PacketPayload.h"

#include "sysunit/TestSuite.h"

#include <gtest/gtest.h>

#include <stubs/sysctl.h>
#include <stubs/uio.h>

using namespace PktGen;
using internal::GetMbufHeader;
using internal::Ipv6Addr;
using internal::ntoh;

class Ipv6HeaderTestSuite : public SysUnit::TestSuite
{
public:
	template <typename Header>
	void ExpectHeader(const Header & h, struct ip6_hdr * expected)
	{
		MbufUniquePtr m = h.Generate();
		auto * ip = GetMbufHeader<struct ip6_hdr>(m);

		EXPECT_EQ(ip->ip6_vfc, expected->ip6_vfc);
		EXPECT_EQ(ip->ip6_flow & IPV6_FLOWLABEL_MASK, expected->ip6_flow & IPV6_FLOWLABEL_MASK);
		EXPECT_EQ(ip->ip6_plen, expected->ip6_plen);
		EXPECT_EQ(ip->ip6_nxt, expected->ip6_nxt);
		EXPECT_EQ(ip->ip6_hlim, expected->ip6_hlim);
		EXPECT_EQ(Ipv6Addr(ip->ip6_src), Ipv6Addr(expected->ip6_src));
		EXPECT_EQ(Ipv6Addr(ip->ip6_dst), Ipv6Addr(expected->ip6_dst));
	}

	auto GenerateHeader(struct ip6_hdr & header)
	{
		char srcIp[INET6_ADDRSTRLEN];
		char dstIp[INET6_ADDRSTRLEN];

		inet_ntop(AF_INET6, &header.ip6_src, srcIp, sizeof(srcIp));
		inet_ntop(AF_INET6, &header.ip6_dst, dstIp, sizeof(dstIp));

		return PacketTemplate(Ipv6Header()
			.With(
				ipVersion(header.ip6_vfc >> 4),
				trafficClass((ntoh(header.ip6_flow) >> 20) & 0xff),
				flow(ntoh(header.ip6_flow) & 0xfffff),
				hopLimit(ntoh(header.ip6_hlim)),
				proto(ntoh(header.ip6_nxt)),
				src(srcIp),
				dst(dstIp)
			));
	}

	template <typename Header>
	void VerifyMbufChainLen(const Header & header, uint16_t expectedPayload)
	{
		MbufUniquePtr m = header.Generate();
		uint16_t totalLen = sizeof(struct ip6_hdr) + expectedPayload;

		EXPECT_EQ(m->m_pkthdr.len, totalLen);

		struct ip6_hdr * ip = GetMbufHeader<struct ip6_hdr>(m);
		EXPECT_EQ(ntoh(ip->ip6_plen), expectedPayload);
	}
};

// Generate a packet from a template and verify that the packet has all fields
// set to the specified values.
TEST_F(Ipv6HeaderTestSuite, TestGenerate)
{
	struct ip6_hdr expected = {
		.ip6_flow = htonl(0x5e00b99e),
		.ip6_plen = htons(0),
		.ip6_hlim = 128,
		.ip6_nxt = 157,
		.ip6_src.s6_addr = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 6},
		.ip6_dst.s6_addr = { 60, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 5, 56},
	};

	auto p = GenerateHeader(expected);

	ExpectHeader(p, &expected);
}

// Verify the default values of fields that aren't set in a IPv4 template.
// Tests will be depending on this values.
TEST_F(Ipv6HeaderTestSuite, TestDefaults)
{
	struct ip6_hdr defaults = {
		.ip6_flow = htonl(0),
		.ip6_plen = htons(0),
		.ip6_hlim = 255,
		.ip6_nxt = 0,
		.ip6_src.s6_addr = {},
		.ip6_dst.s6_addr = {},
	};

	defaults.ip6_vfc = IPV6_VERSION;

	auto p = PacketTemplate(Ipv6Header());

	ExpectHeader(p, &defaults);
}

// Create a packet template and then create another by applying mutators to
// the original.  Verify that the original template is unchanged and that the
// second template generates packets with fields set to the values specified by
// the mutators.
TEST_F(Ipv6HeaderTestSuite, TestMutate)
{
	struct ip6_hdr expected1 = {
		.ip6_flow = htonl(0x60000000),
		.ip6_plen = htons(0),
		.ip6_hlim = 96,
		.ip6_nxt = 6,
		.ip6_src.s6_addr = {},
		.ip6_dst.s6_addr = {},
	};

	auto p1 = GenerateHeader(expected1);

	struct ip6_hdr expected2 = {
		.ip6_flow = htonl(0x49600055),
		.ip6_plen = htons(0),
		.ip6_hlim = 251,
		.ip6_nxt = 10,
		.ip6_src.s6_addr = {0xef, 0x34},
		.ip6_dst.s6_addr = {0x89, 0x48},
	};

	auto p2 = p1.WithHeader(Layer::L3).Fields(
	        ipVersion(4),
	        trafficClass(0x96),
	        flow(0x55),
	        hopLimit(251),
	        proto(10),
	        src("ef34::"),
	        dst("8948::")
	);

	ExpectHeader(p1, &expected1);
	ExpectHeader(p2, &expected2);
}

// Create a packet template and then create a template that is next in the
// packet stream.  Verify that nothing in the IPv6 header has changed.  Create
// a template that is a retransmission in the packet stream, and verify again
// that the header is identical.
TEST_F(Ipv6HeaderTestSuite, TestNext)
{
	auto p1 = PacketTemplate(
	    Ipv6Header().With(
	        src("2515:2599::3e"),
	        dst("9678:1258::fd")
	     ));

	auto p2 = p1.Next();
	auto p3 = p1.Retransmission();

	MbufUniquePtr m = p1.Generate();
	struct ip6_hdr * expected = GetMbufHeader<struct ip6_hdr>(m);

	ExpectHeader(p2, expected);
	ExpectHeader(p3, expected);
}

// Create packet templates with various sized payloads in them and verify that
// the size of the mbuf and the ip_len field both reflect the payload size.
TEST_F(Ipv6HeaderTestSuite, TestPayload)
{
	auto p1 = PacketTemplate(
	    Ipv6Header(),
	    PacketPayload()
	);

	VerifyMbufChainLen(p1, 0);

	auto p2 = p1.WithHeader(Layer::PAYLOAD).Fields(payload("abc", 14));
	VerifyMbufChainLen(p2, 14);

	auto p3 = p1.WithHeader(Layer::PAYLOAD).Fields(payload("inet6", 8952));
	VerifyMbufChainLen(p3, 8952);
}
