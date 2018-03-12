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

#include "pktgen/Tcp.h"

#include "pktgen/Packet.h"
#include "pktgen/PacketPayload.h"
#include "pktgen/PacketParsing.h"

#include "sysunit/TestSuite.h"

#include <gtest/gtest.h>

#include <stubs/sysctl.h>
#include <stubs/uio.h>

using namespace PktGen;
using internal::GetMbufHeader;
using internal::hton;
using internal::ntoh;

class TcpHeaderTestSuite : public SysUnit::TestSuite
{
public:
	auto BuildHeader(struct tcphdr & tcp)
	{
		return PacketTemplate(TcpHeader()
			.With(
				src(ntoh(tcp.th_sport)),
				dst(ntoh(tcp.th_dport)),
				seq(ntoh(tcp.th_seq)),
				ack(ntoh(tcp.th_ack)),
				flags(ntoh(tcp.th_flags)),
				window(ntoh(tcp.th_win)),
				checksum(ntoh(tcp.th_sum)),
				urp(ntoh(tcp.th_urp))
			 ));
	}

	template <typename Header>
	void ExpectHeader(const Header & header, struct tcphdr *expected, size_t paylen = 0)
	{
		MbufPtr m = header.Generate();
		auto * tcp = GetMbufHeader<tcphdr>(m);

		EXPECT_EQ(m->m_pkthdr.len, sizeof(struct tcphdr) + paylen);

		EXPECT_EQ(tcp->th_sport, expected->th_sport);
		EXPECT_EQ(tcp->th_dport, expected->th_dport);
		EXPECT_EQ(tcp->th_seq, expected->th_seq);
		EXPECT_EQ(tcp->th_ack, expected->th_ack);
		EXPECT_EQ(tcp->th_x2, expected->th_x2);
		EXPECT_EQ(tcp->th_off, expected->th_off);
		EXPECT_EQ(tcp->th_flags, expected->th_flags);
		EXPECT_EQ(tcp->th_win, expected->th_win);
		EXPECT_EQ(tcp->th_sum, expected->th_sum);
		EXPECT_EQ(tcp->th_urp, expected->th_urp);
	}

	template <typename Header>
	void VerifyMbufChainLen(const Header & header, size_t expectedPayload)
	{
		MbufPtr m = header.Generate();
		uint16_t totalLen = sizeof(struct tcphdr) + expectedPayload;

		EXPECT_EQ(m->m_pkthdr.len, totalLen);
	}

	template <typename Header>
	void VerifyMbufPayload(const Header & header, char expectedByte, size_t expectedPayload)
	{
		MbufPtr m = header.Generate();
		size_t totalLen = sizeof(struct tcphdr) + expectedPayload;

		ASSERT_EQ(m->m_pkthdr.len, totalLen);

		char * payload = GetMbufHeader<char>(m, sizeof(struct tcphdr));
		for (size_t i = 0; i < expectedPayload; ++i) {
			ASSERT_EQ(payload[i], expectedByte);
		}
	}
};

// Define a TCP header template and generate a packet based on the template.
// Verify that the packet has all TCP header fields set to those in the
// template.
TEST_F(TcpHeaderTestSuite, TestGenerate)
{
	struct tcphdr expected = {
		.th_sport = htons(39965),
		.th_dport = htons(5268),
		.th_seq = htonl(0x188ef528),
		.th_ack = htonl(0xf1987989),
		.th_x2 = 0,
		.th_off = sizeof(struct tcphdr) / sizeof(uint32_t),
		.th_flags = TH_SYN | TH_FIN,
		.th_win = htons(48542),
		.th_sum = htons(0xffed),
		.th_urp = htons(0xb908),
	};

	auto p = BuildHeader(expected);
	ExpectHeader(p, &expected);
}

// Verify the default values of fields that aren't set in a template.
// Tests will be depending on this values.
TEST_F(TcpHeaderTestSuite, TestDefaults)
{
	struct tcphdr expected = {
		.th_sport = htons(0),
		.th_dport = htons(0),
		.th_seq = htonl(0),
		.th_ack = htonl(0),
		.th_x2 = 0,
		.th_off = sizeof(struct tcphdr) / sizeof(uint32_t),
		.th_flags = TH_ACK,
		.th_win = htons(0),
		.th_sum = htons(0),
		.th_urp = htons(0),
	};

	auto p = PacketTemplate(TcpHeader());
	ExpectHeader(p, &expected);
}

// Define a TCP template and then create another by applying mutators to the
// original.   Verify that the original template is unchanged and that the
// second template generates packets with fields set to the values specified by
// the mutators.
TEST_F(TcpHeaderTestSuite, TestMutate)
{
	struct tcphdr expected1 = {
		.th_sport = htons(2841),
		.th_dport = htons(55841),
		.th_seq = htonl(0x295415),
		.th_ack = htonl(0x996),
		.th_x2 = 0,
		.th_off = sizeof(struct tcphdr) / sizeof(uint32_t),
		.th_flags = TH_RST,
		.th_win = htons(0xffff),
		.th_sum = htons(0x28),
		.th_urp = htons(0x1478),
	};
	auto p1 = BuildHeader(expected1);

	struct tcphdr expected2 = {
		.th_sport = htons(25),
		.th_dport = htons(28921),
		.th_seq = htonl(100),
		.th_ack = htonl(223),
		.th_x2 = 0,
		.th_off = sizeof(struct tcphdr) / sizeof(uint32_t),
		.th_flags = TH_ACK | TH_ECE,
		.th_win = htons(16),
		.th_sum = htons(286),
		.th_urp = htons(288),
	};

	auto p2 = p1.WithHeader(Layer::L4).Fields(
		src(25),
		dst(28921),
		seq(100),
		ack(223),
		flags(TH_ACK | TH_ECE),
		window(16),
		checksum(286),
		urp(288)
	);

	ExpectHeader(p1, &expected1);
	ExpectHeader(p2, &expected2);
}

// Create a packet template and then create a template that is next in the
// connection with Next().  Verify that the sequence number has advanced while
// all other fields in the header are the same.
TEST_F(TcpHeaderTestSuite, TestNext)
{
	uint32_t firstSeq = 123;
	int payloadLen = 16;
	auto p1 = PacketTemplate(
		TcpHeader()
			.With(
				src(2155),
				dst(258),
				seq(firstSeq),
				ack(321),
				flags(TH_SYN | TH_ACK),
				window(10),
				checksum(386),
				urp(486)
			),
		PacketPayload()
			.With(payload(0x00, payloadLen))
	);

	auto p2 = p1.Next();

	MbufPtr m = p1.Generate();
	struct tcphdr * expected = GetMbufHeader<struct tcphdr>(m);

	expected->th_seq = hton(firstSeq + payloadLen);

	ExpectHeader(p2, expected);
}

// Create a packet template and then create a template that is a TCP
// retransmission.  Verify that all fields in the header are the same.
TEST_F(TcpHeaderTestSuite, TestRetransmission)
{
	const char * testPayload = "testpayload";
	size_t payloadLen = 300;

	auto p1 = PacketTemplate(
		TcpHeader()
			.With(
				src(2155),
				dst(258),
				seq(123),
				ack(321),
				flags(TH_SYN | TH_ACK),
				window(10),
				checksum(386),
				urp(486)
			),
		PacketPayload()
		   .With(payload(testPayload, payloadLen))
	);

	auto p2 = p1.Retransmission();

	MbufPtr m = p1.Generate();
	struct tcphdr * expected = GetMbufHeader<struct tcphdr>(m);

	ExpectHeader(p2, expected, payloadLen);

	m = p2.Generate();
	uint8_t * payload = GetMbufHeader<uint8_t>(m, sizeof(struct tcphdr));

	ASSERT_EQ(memcmp(testPayload, payload, strlen(testPayload)), 0);
}

// Create packet templates with various sized payloads in them and verify that
// the size of the mbuf reflects the payload size.
TEST_F(TcpHeaderTestSuite, TestPayload)
{
	auto p1 = PacketTemplate(
	    TcpHeader(),
	    PacketPayload()
	);

	VerifyMbufChainLen(p1, 0);

	auto p2 = p1.WithHeader(Layer::PAYLOAD).Fields(payload("abc", 6));
	VerifyMbufChainLen(p2, 6);

	auto p3 = p1.WithHeader(Layer::PAYLOAD).Fields(payload("ipv4", 6018));
	VerifyMbufChainLen(p3, 6018);
}

TEST_F(TcpHeaderTestSuite, TestSegmentation)
{
	size_t mss = 1448;
	size_t maxPayload = mss - sizeof(struct tcphdr);
	size_t lastSegLen = 500;
	size_t paylen = 2 * maxPayload + lastSegLen;

	auto p1 = PacketTemplate(
	    TcpHeader().With(mtu(mss)),
	    PacketPayload().With(payload('a', paylen))
	);

	auto p2 = p1.Next();
	auto p3 = p2.Next();
	auto p4 = p3.Next();

	VerifyMbufPayload(p1, 'a', maxPayload);
	VerifyMbufPayload(p2, 'a', maxPayload);
	VerifyMbufPayload(p3, 'a', lastSegLen);
	VerifyMbufChainLen(p4, 0);
}
