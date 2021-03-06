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

#include "pktgen/PacketPayload.h"

#include "pktgen/Ipv4.h"
#include "pktgen/Ipv6.h"
#include "pktgen/Tcp.h"

#include "sysunit/TestSuite.h"

#include <gtest/gtest.h>

#include <stubs/sysctl.h>
#include <stubs/uio.h>

using namespace PktGen;
using internal::GetMbufHeader;
using internal::ntoh;

class PacketPayloadTestSuite : public SysUnit::TestSuite
{
};

TEST_F(PacketPayloadTestSuite, TestVectorPayload)
{
	auto p = PacketTemplate(PacketPayload().With(payload({1, 2, 3})));

	MbufUniquePtr m = p.Generate();

	ASSERT_EQ(m->m_pkthdr.len, 3);
	EXPECT_EQ(m->m_data[0], 1);
	EXPECT_EQ(m->m_data[1], 2);
	EXPECT_EQ(m->m_data[2], 3);
}

TEST_F(PacketPayloadTestSuite, TestEmptyPayload)
{
	auto p = PacketTemplate(PacketPayload());

	MbufUniquePtr m = p.Generate();
	ASSERT_EQ(m->m_pkthdr.len, 0);

	auto p2 = p.WithHeader(Layer::PAYLOAD).Fields(payload());

	m = p2.Generate();
	ASSERT_EQ(m->m_pkthdr.len, 0);
}

TEST_F(PacketPayloadTestSuite, TestBytePayload)
{
	auto p1 = PacketTemplate(PacketPayload().With(payload(0x05, 4)));

	MbufUniquePtr m = p1.Generate();
	ASSERT_EQ(m->m_pkthdr.len, 4);
	for (int i = 0; i < m->m_pkthdr.len; ++i)
		EXPECT_EQ(m->m_data[i], 0x05);

	auto p2 = p1.With(payload(0x0a));

	m = p2.Generate();
	ASSERT_EQ(m->m_pkthdr.len, 1);
	EXPECT_EQ(m->m_data[0], 0x0a);
}

TEST_F(PacketPayloadTestSuite, TestStringPayload)
{
	auto p1 = PacketTemplate(PacketPayload().With(payload("abcde", 7)));

	MbufUniquePtr m = p1.Generate();
	ASSERT_EQ(m->m_pkthdr.len, 7);
	EXPECT_EQ(m->m_data[0], 'a');
	EXPECT_EQ(m->m_data[1], 'b');
	EXPECT_EQ(m->m_data[2], 'c');
	EXPECT_EQ(m->m_data[3], 'd');
	EXPECT_EQ(m->m_data[4], 'e');
	EXPECT_EQ(m->m_data[5], 'a');
	EXPECT_EQ(m->m_data[6], 'b');

	auto p2 = p1.WithHeader(Layer::PAYLOAD).Fields(payload("bsd"));

	m = p2.Generate();
	ASSERT_EQ(m->m_pkthdr.len, 3);
	EXPECT_EQ(m->m_data[0], 'b');
	EXPECT_EQ(m->m_data[1], 's');
	EXPECT_EQ(m->m_data[2], 'd');
}

TEST_F(PacketPayloadTestSuite, TestAppendVector)
{
	auto p1 = PacketTemplate(PacketPayload().With(payload("daemon")));

	auto p2 = p1.WithHeader(Layer::PAYLOAD).Fields(appendPayload({7, 8, 9}));

	MbufUniquePtr m = p2.Generate();
	ASSERT_EQ(m->m_pkthdr.len, 9);
	EXPECT_EQ(m->m_data[0], 'd');
	EXPECT_EQ(m->m_data[1], 'a');
	EXPECT_EQ(m->m_data[2], 'e');
	EXPECT_EQ(m->m_data[3], 'm');
	EXPECT_EQ(m->m_data[4], 'o');
	EXPECT_EQ(m->m_data[5], 'n');
	EXPECT_EQ(m->m_data[6], 7);
	EXPECT_EQ(m->m_data[7], 8);
	EXPECT_EQ(m->m_data[8], 9);
}

TEST_F(PacketPayloadTestSuite, TestAppendByte)
{
	auto p1 = PacketTemplate(PacketPayload());
	auto p2 = p1.With(appendPayload(0x10));

	MbufUniquePtr m = p2.Generate();
	ASSERT_EQ(m->m_pkthdr.len, 1);
	EXPECT_EQ(m->m_data[0], 0x10);

	auto p3 = p2.WithHeader(Layer::PAYLOAD).Fields(appendPayload(0x65, 9));

	m = p3.Generate();
	ASSERT_EQ(m->m_pkthdr.len, 10);
	EXPECT_EQ(m->m_data[0], 0x10);
	for (int i = 1; i < m->m_pkthdr.len; ++i)
		EXPECT_EQ(m->m_data[i], 0x65);
}

TEST_F(PacketPayloadTestSuite, TestAppendString)
{
	auto p1 = PacketTemplate(PacketPayload().With(payload({9, 6, 3})));
	auto p2 = p1.With(appendPayload("741"));

	MbufUniquePtr m = p2.Generate();
	ASSERT_EQ(m->m_pkthdr.len, 6);
	EXPECT_EQ(m->m_data[0], 9);
	EXPECT_EQ(m->m_data[1], 6);
	EXPECT_EQ(m->m_data[2], 3);
	EXPECT_EQ(m->m_data[3], '7');
	EXPECT_EQ(m->m_data[4], '4');
	EXPECT_EQ(m->m_data[5], '1');

	auto p3 = p1.WithHeader(Layer::PAYLOAD).Fields(appendPayload("1478963"));

	m = p3.Generate();
	ASSERT_EQ(m->m_pkthdr.len, 10);
	EXPECT_EQ(m->m_data[0], 9);
	EXPECT_EQ(m->m_data[1], 6);
	EXPECT_EQ(m->m_data[2], 3);
	EXPECT_EQ(m->m_data[3], '1');
	EXPECT_EQ(m->m_data[4], '4');
	EXPECT_EQ(m->m_data[5], '7');
	EXPECT_EQ(m->m_data[6], '8');
	EXPECT_EQ(m->m_data[7], '9');
	EXPECT_EQ(m->m_data[8], '6');
	EXPECT_EQ(m->m_data[9], '3');
}

TEST_F(PacketPayloadTestSuite, TestReduceLength)
{
	auto p1 = PacketTemplate(PacketPayload().With(payload("1234567890", 21)));
	auto p2 = p1.With(length(6));

	MbufUniquePtr m = p2.Generate();
	ASSERT_EQ(m->m_pkthdr.len, 6);
	EXPECT_EQ(m->m_data[0], '1');
	EXPECT_EQ(m->m_data[1], '2');
	EXPECT_EQ(m->m_data[2], '3');
	EXPECT_EQ(m->m_data[3], '4');
	EXPECT_EQ(m->m_data[4], '5');
	EXPECT_EQ(m->m_data[5], '6');

	auto p3 = p1.WithHeader(Layer::PAYLOAD).Fields(length(0));

	m = p3.Generate();
	ASSERT_EQ(m->m_pkthdr.len, 0);
}

template <typename L3Proto>
class EncapsulatedPayloadTestSuite : public SysUnit::TestSuite
{
public:
	void CheckLengthField(struct mbuf * m, uint16_t payloadLen);
	auto GetL3Header();
	size_t GetL3HeaderLen();

	uint8_t * GetL3Payload(struct mbuf * m)
	{
		return GetMbufHeader<uint8_t>(m, GetL3HeaderLen());
	}
};

struct IPv4 {};

template <>
void EncapsulatedPayloadTestSuite<IPv4>::CheckLengthField(struct mbuf * m, uint16_t payloadLen)
{
	uint16_t expectedLen;

	expectedLen = sizeof(struct ip) + payloadLen;

	ASSERT_EQ(m->m_pkthdr.len, expectedLen);

	struct ip * ip = GetMbufHeader<struct ip>(m);
	EXPECT_EQ(ntoh(ip->ip_len), expectedLen);
}

template <>
auto EncapsulatedPayloadTestSuite<IPv4>::GetL3Header()
{
	return Ipv4Header();
}

template<>
size_t EncapsulatedPayloadTestSuite<IPv4>::GetL3HeaderLen()
{
	return sizeof(struct ip);
}

struct IPv6 {};

template <>
void EncapsulatedPayloadTestSuite<IPv6>::CheckLengthField(struct mbuf * m, uint16_t payloadLen)
{
	uint16_t expectedLen;

	expectedLen = sizeof(struct ip6_hdr) + payloadLen;

	ASSERT_EQ(m->m_pkthdr.len, expectedLen);

	struct ip6_hdr * ip = GetMbufHeader<struct ip6_hdr>(m);
	EXPECT_EQ(ntoh(ip->ip6_plen), payloadLen);
}

template <>
auto EncapsulatedPayloadTestSuite<IPv6>::GetL3Header()
{
	return Ipv6Header();
}

template<>
size_t EncapsulatedPayloadTestSuite<IPv6>::GetL3HeaderLen()
{
	return sizeof(struct ip6_hdr);
}

typedef ::testing::Types<IPv4, IPv6> NetworkTypes;
TYPED_TEST_CASE(EncapsulatedPayloadTestSuite, NetworkTypes);

// Generate a packet with an L3 header and a payload.  Verify that the L3
// header's length field properly reflects the size of the payload after
// applying a variety of mutators that accept a vector of bytes.
TYPED_TEST(EncapsulatedPayloadTestSuite, TestVectorPayload)
{
	auto p = PacketTemplate(
		this->GetL3Header(),
		PacketPayload().With(payload({9, 8, 7, 6}))
	);

	MbufUniquePtr m = p.Generate();
	this->CheckLengthField(m.get(), 4);

	uint8_t * pktPayload = this->GetL3Payload(m.get());
	EXPECT_EQ(pktPayload[0], 9);
	EXPECT_EQ(pktPayload[1], 8);
	EXPECT_EQ(pktPayload[2], 7);
	EXPECT_EQ(pktPayload[3], 6);

	auto p2 = p.WithHeader(Layer::PAYLOAD).Fields(appendPayload({1,2,3,4,5}));

	m = p2.Generate();
	this->CheckLengthField(m.get(), 9);

	pktPayload = this->GetL3Payload(m.get());
	EXPECT_EQ(pktPayload[0], 9);
	EXPECT_EQ(pktPayload[1], 8);
	EXPECT_EQ(pktPayload[2], 7);
	EXPECT_EQ(pktPayload[3], 6);
	EXPECT_EQ(pktPayload[4], 1);
	EXPECT_EQ(pktPayload[5], 2);
	EXPECT_EQ(pktPayload[6], 3);
	EXPECT_EQ(pktPayload[7], 4);
	EXPECT_EQ(pktPayload[8], 5);

	auto p3 = p.With(payload({1, 2, 3}));

	m = p3.Generate();
	this->CheckLengthField(m.get(), 3);

	pktPayload = this->GetL3Payload(m.get());
	EXPECT_EQ(pktPayload[0], 1);
	EXPECT_EQ(pktPayload[1], 2);
	EXPECT_EQ(pktPayload[2], 3);
}

// Generate a packet with an L3 header and a payload.  Verify that the L3
// header's length field properly reflects the size of the payload after
// applying a variety of mutators that accept a byte and an option repeat count.
TYPED_TEST(EncapsulatedPayloadTestSuite, TestBytePayload)
{
	auto p = PacketTemplate(
		this->GetL3Header(),
		PacketPayload().With(payload(0x67))
	);

	MbufUniquePtr m = p.Generate();
	this->CheckLengthField(m.get(), 1);

	uint8_t * pktPayload = this->GetL3Payload(m.get());
	EXPECT_EQ(pktPayload[0], 0x67);

	auto p2 = p.With(appendPayload(0x77, 3));

	m = p2.Generate();
	this->CheckLengthField(m.get(), 4);

	pktPayload = this->GetL3Payload(m.get());
	EXPECT_EQ(pktPayload[0], 0x67);
	EXPECT_EQ(pktPayload[1], 0x77);
	EXPECT_EQ(pktPayload[2], 0x77);
	EXPECT_EQ(pktPayload[3], 0x77);

	auto p3 = p.WithHeader(Layer::PAYLOAD).Fields(payload(0x11, 10));

	m = p3.Generate();
	this->CheckLengthField(m.get(), 10);

	pktPayload = this->GetL3Payload(m.get());
	for (int i = 0; i < 10; ++i)
		EXPECT_EQ(pktPayload[i], 0x11);
}

// Generate a packet with an L3 header and a payload.  Verify that the L3
// header's length field properly reflects the size of the payload after
// applying a variety of mutators that accept a string
TYPED_TEST(EncapsulatedPayloadTestSuite, TestStringPayload)
{
	auto p = PacketTemplate(
		this->GetL3Header(),
		PacketPayload().With(payload("kfjuw", 10))
	);

	MbufUniquePtr m = p.Generate();
	this->CheckLengthField(m.get(), 10);

	uint8_t * pktPayload = this->GetL3Payload(m.get());
	EXPECT_EQ(pktPayload[0], 'k');
	EXPECT_EQ(pktPayload[1], 'f');
	EXPECT_EQ(pktPayload[2], 'j');
	EXPECT_EQ(pktPayload[3], 'u');
	EXPECT_EQ(pktPayload[4], 'w');
	EXPECT_EQ(pktPayload[5], 'k');
	EXPECT_EQ(pktPayload[6], 'f');
	EXPECT_EQ(pktPayload[7], 'j');
	EXPECT_EQ(pktPayload[8], 'u');
	EXPECT_EQ(pktPayload[9], 'w');

	auto p2 = p.WithHeader(Layer::PAYLOAD).Fields(appendPayload("8596"));

	m = p2.Generate();
	this->CheckLengthField(m.get(), 14);

	pktPayload = this->GetL3Payload(m.get());
	EXPECT_EQ(pktPayload[0], 'k');
	EXPECT_EQ(pktPayload[1], 'f');
	EXPECT_EQ(pktPayload[2], 'j');
	EXPECT_EQ(pktPayload[3], 'u');
	EXPECT_EQ(pktPayload[4], 'w');
	EXPECT_EQ(pktPayload[5], 'k');
	EXPECT_EQ(pktPayload[6], 'f');
	EXPECT_EQ(pktPayload[7], 'j');
	EXPECT_EQ(pktPayload[8], 'u');
	EXPECT_EQ(pktPayload[9], 'w');
	EXPECT_EQ(pktPayload[10], '8');
	EXPECT_EQ(pktPayload[11], '5');
	EXPECT_EQ(pktPayload[12], '9');
	EXPECT_EQ(pktPayload[13], '6');

	int len = 39;
	auto p3 = p.With(payload("0123456789", len));

	m = p3.Generate();
	this->CheckLengthField(m.get(), len);

	pktPayload = this->GetL3Payload(m.get());
	for (int i = 0; i < roundup(len, 10); ++i) {
		int count = std::min(10, len - i * 10);
		for (int j = 0; j < count; ++j)
			EXPECT_EQ(pktPayload[i * 10 + j], '0' + j);
	}
}

// Generate a packet with an L3 header and a payload.  Verify that the L3
// header's length field properly reflects the size of the payload after
// applying a variety of mutators that set the payload's length.
TYPED_TEST(EncapsulatedPayloadTestSuite, TestSetPayloadLength)
{
	auto p = PacketTemplate(
		this->GetL3Header(),
		PacketPayload().With(payload("kfjuw", 10))
	);

	auto p2 = p.WithHeader(Layer::PAYLOAD).Fields(length(6));

	MbufUniquePtr m = p2.Generate();
	this->CheckLengthField(m.get(), 6);

	uint8_t * pktPayload = this->GetL3Payload(m.get());
	EXPECT_EQ(pktPayload[0], 'k');
	EXPECT_EQ(pktPayload[1], 'f');
	EXPECT_EQ(pktPayload[2], 'j');
	EXPECT_EQ(pktPayload[3], 'u');
	EXPECT_EQ(pktPayload[4], 'w');
	EXPECT_EQ(pktPayload[5], 'k');

	auto p3 = p.With(length(0));

	m = p3.Generate();
	this->CheckLengthField(m.get(), 0);
}

template <typename PktTemplate>
static void
TestPayload(const PktTemplate & p, size_t pktNum, size_t headerLen, size_t payloadLen, char byte)
{
	MbufUniquePtr m = p.Generate();

	ASSERT_EQ(m->m_pkthdr.len, headerLen + payloadLen);

	char * payload = GetMbufHeader<char>(m, headerLen);
	for (size_t i = 0; i < payloadLen; ++i) {
		ASSERT_EQ(payload[i], byte) << "Mismatch at packet " <<
			pktNum << " index " << i;
	}
}

// Encapsulate a large payload in a IPvX/TCP header and set an MTU on the
// L3 header that will require the payload to be segmented.  Create a
// template for each segment and generate a packet for that template, and
// verify that the packet has the right length and payload.
TYPED_TEST(EncapsulatedPayloadTestSuite, TestPayloadSegmentation)
{
	size_t ifMtu = 1500;
	size_t headerLen = this->GetL3HeaderLen() + sizeof(struct tcphdr);
	size_t maxPayload = ifMtu - headerLen;

	auto pktTemplate = PacketTemplate(
		this->GetL3Header().With(mtu(ifMtu)),
		TcpHeader(),
		PacketPayload().With(payload('w', IP_MAXPACKET))
	);

	size_t packets = (IP_MAXPACKET / maxPayload);

	for (size_t i = 0; i < packets; ++i) {
		TestPayload(pktTemplate, i, headerLen, maxPayload, 'w');
		pktTemplate = pktTemplate.Next();
	}

	size_t left = IP_MAXPACKET - packets * maxPayload;
	TestPayload(pktTemplate, packets, headerLen, left, 'w');
}

// Confirm that if no MTU is configured, that the result of Next() is a
// template that specifies an empty payload.
TYPED_TEST(EncapsulatedPayloadTestSuite, TestNoMtu)
{
	size_t headerLen = this->GetL3HeaderLen() + sizeof(struct tcphdr);

	auto pkt1 = PacketTemplate(
		this->GetL3Header(),
		TcpHeader(),
		PacketPayload().With(payload('a', 10))
	);

	auto pkt2 = pkt1.Next();

	TestPayload(pkt1, 1, headerLen, 10, 'a');
	TestPayload(pkt2, 2, headerLen, 0, 'a');
}
