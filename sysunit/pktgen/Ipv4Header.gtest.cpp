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

#include "pktgen/Ipv4Header.h"

#include "pktgen/EthernetHeader.h"
#include "pktgen/Packet.h"
#include "pktgen/PacketPayload.h"

#include "sysunit/TestSuite.h"

#include <gtest/gtest.h>

#include <stubs/sysctl.h>
#include <stubs/uio.h>

using namespace PktGen;

class Ipv4HeaderTestSuite : public SysUnit::TestSuite
{
public:
	template <typename Header>
	void ExpectHeader(const Header & h, struct ip * expected)
	{
		MbufPtr m = h.Generate();
		auto * ip = GetMbufHeader<struct ip>(m.get(), 0);

		EXPECT_EQ(ip->ip_v, expected->ip_v);
		EXPECT_EQ(ip->ip_hl, expected->ip_hl);
		EXPECT_EQ(ip->ip_tos, expected->ip_tos);
		EXPECT_EQ(ip->ip_len, expected->ip_len);
		EXPECT_EQ(ip->ip_id, expected->ip_id);
		EXPECT_EQ(ip->ip_off, expected->ip_off);
		EXPECT_EQ(ip->ip_ttl, expected->ip_ttl);
		EXPECT_EQ(ip->ip_p, expected->ip_p);
		EXPECT_EQ(ip->ip_sum, expected->ip_sum);
		EXPECT_EQ(ip->ip_src.s_addr, expected->ip_src.s_addr);
		EXPECT_EQ(ip->ip_dst.s_addr, expected->ip_dst.s_addr);
	}

	auto GenerateHeader(struct ip & header)
	{
		char srcIp[INET_ADDRSTRLEN];
		char dstIp[INET_ADDRSTRLEN];

		inet_ntop(AF_INET, &header.ip_src, srcIp, sizeof(srcIp));
		inet_ntop(AF_INET, &header.ip_dst, dstIp, sizeof(dstIp));

		return PacketTemplate(Ipv4Header()
			.With(
				ipVersion(ntoh(header.ip_v)),
				headerLength(ntoh(header.ip_hl)),
				tos(ntoh(header.ip_tos)),
				id(ntoh(header.ip_id)),
				fragOffset(ntoh(header.ip_off)),
				ttl(ntoh(header.ip_ttl)),
				proto(ntoh(header.ip_p)),
				checksum(ntoh(header.ip_sum)),
				src(srcIp),
				dst(dstIp)
			));
	}

	template <typename Header>
	void VerifyMbufChainLen(const Header & header, size_t expectedPayload)
	{
		MbufPtr m = header.Generate();
		uint16_t totalLen = sizeof(struct ip) + expectedPayload;

		EXPECT_EQ(m->m_pkthdr.len, totalLen);

		struct ip * ip = GetMbufHeader<struct ip>(m.get(), 0);
		EXPECT_EQ(ntoh(ip->ip_len), totalLen);
	}
};

// Generate a packet from a template and verify that the packet has all fields
// set to the specified values.
TEST_F(Ipv4HeaderTestSuite, TestGenerate)
{
	struct ip expected = {
		.ip_v = 3,
		.ip_hl = 2,
		.ip_tos = 247,
		.ip_len = htons(sizeof(struct ip)),
		.ip_id = htons(62874),
		.ip_off = htons(58741),
		.ip_ttl = 128,
		.ip_p = 200,
		.ip_sum = htons(0xd874),
		.ip_src.s_addr = htonl(0xc0000201),
		.ip_dst.s_addr = htonl(0xc0000202),
	};

	auto p = GenerateHeader(expected);

	ExpectHeader(p, &expected);
}

// Verify the default values of fields that aren't set in a IPv4 template.
// Tests will be depending on this values.
TEST_F(Ipv4HeaderTestSuite, TestDefaults)
{
	struct ip defaults = {
		.ip_v = 4,
		.ip_hl = sizeof(struct ip) / sizeof(uint32_t),
		.ip_tos = 0,
		.ip_len = htons(sizeof(struct ip)),
		.ip_id = 0,
		.ip_off = 0,
		.ip_ttl = 255,
		.ip_p = 0,
		.ip_sum = htons(0),
		.ip_src.s_addr = htonl(0),
		.ip_dst.s_addr = htonl(0),
	};

	auto p = PacketTemplate(Ipv4Header());

	ExpectHeader(p, &defaults);
}

// Create a packet template and then create another by applying mutators to
// the original.  Verify that the original template is unchanged and that the
// second template generates packets with fields set to the values specified by
// the mutators.
TEST_F(Ipv4HeaderTestSuite, TestMutate)
{
	struct ip expected1 = {
		.ip_v = 3,
		.ip_hl = 2,
		.ip_tos = 247,
		.ip_len = htons(sizeof(struct ip)),
		.ip_id = htons(62874),
		.ip_off = htons(58741),
		.ip_ttl = 128,
		.ip_p = 200,
		.ip_sum = htons(0xd874),
		.ip_src.s_addr = htonl(0xc0000201),
		.ip_dst.s_addr = htonl(0xc0000202),
	};

	auto p1 = GenerateHeader(expected1);

	struct ip expected2 = {
		.ip_v = 4,
		.ip_hl = 7,
		.ip_tos = 2,
		.ip_len = htons(sizeof(struct ip)),
		.ip_id = htons(85),
		.ip_off = htons(47),
		.ip_ttl = 64,
		.ip_p = 6,
		.ip_sum = htons(0x8848),
		.ip_src.s_addr = htonl(0x10111213),
		.ip_dst.s_addr = htonl(0x20212223),
	};

	auto p2 = p1.WithHeader(Layer::L3).Fields(
	        ipVersion(ntoh(expected2.ip_v)),
	        headerLength(ntoh(expected2.ip_hl)),
	        tos(ntoh(expected2.ip_tos)),
	        id(ntoh(expected2.ip_id)),
	        fragOffset(ntoh(expected2.ip_off)),
	        ttl(ntoh(expected2.ip_ttl)),
	        proto(ntoh(expected2.ip_p)),
	        checksum(ntoh(expected2.ip_sum)),
	        src("16.17.18.19"),
	        dst("32.33.34.35")
	);

	ExpectHeader(p1, &expected1);
	ExpectHeader(p2, &expected2);
}

// Create a packet template and then create a template that is next in the
// packet stream.  Verify that the ip_id fields has incremented but nothing
// else in the header has changed.  Create a template that is a retransmission
// in the packet stream, and verify again that only the ip_id has incremented.
TEST_F(Ipv4HeaderTestSuite, TestNext)
{
	uint16_t firstId = 9822;
	auto p1 = PacketTemplate(
	    Ipv4Header().With(
	        id(firstId),
	        src("10.1.66.198"),
	        dst("10.1.66.197")
	     ));

	auto p2 = p1.Next();
	auto p3 = p1.Retransmission();

	MbufPtr m = p1.Generate();
	struct ip * expected = GetMbufHeader<ip>(m.get(), 0);

	expected->ip_id = htons(firstId + 1);

	ExpectHeader(p2, expected);
	ExpectHeader(p3, expected);
}

// Create packet templates with various sized payloads in them and verify that
// the size of the mbuf and the ip_len field both reflect the payload size.
TEST_F(Ipv4HeaderTestSuite, TestPayload)
{
	auto p1 = PacketTemplate(
	    Ipv4Header(),
	    PacketPayload()
	);

	VerifyMbufChainLen(p1, 0);

	auto p2 = p1.WithHeader(Layer::PAYLOAD).Fields(payload("abc", 6));
	VerifyMbufChainLen(p2, 6);

	auto p3 = p1.WithHeader(Layer::PAYLOAD).Fields(payload("ipv4", 6018));
	VerifyMbufChainLen(p3, 6018);
}
