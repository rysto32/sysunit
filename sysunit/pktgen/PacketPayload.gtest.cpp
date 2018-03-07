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

#include "pktgen/Ipv4Header.h"
#include "pktgen/Ipv6Header.h"
#include "pktgen/Packet.h"

#include "sysunit/TestSuite.h"

#include <gtest/gtest.h>

#include <stubs/sysctl.h>
#include <stubs/uio.h>

using namespace PktGen;

class PacketPayloadTestSuite : public SysUnit::TestSuite
{
};

TEST_F(PacketPayloadTestSuite, TestVectorPayload)
{
	auto p = PacketTemplate(PacketPayload().With(payload({1, 2, 3})));

	MbufPtr m = p.Generate();

	ASSERT_EQ(m->m_pkthdr.len, 3);
	EXPECT_EQ(m->m_data[0], 1);
	EXPECT_EQ(m->m_data[1], 2);
	EXPECT_EQ(m->m_data[2], 3);
}

TEST_F(PacketPayloadTestSuite, TestEmptyPayload)
{
	auto p = PacketTemplate(PacketPayload());

	MbufPtr m = p.Generate();
	ASSERT_EQ(m->m_pkthdr.len, 0);

	auto p2 = p.WithHeader(Layer::PAYLOAD).Fields(payload());

	m = p2.Generate();
	ASSERT_EQ(m->m_pkthdr.len, 0);
}

TEST_F(PacketPayloadTestSuite, TestBytePayload)
{
	auto p1 = PacketTemplate(PacketPayload().With(payload(0x05, 4)));

	MbufPtr m = p1.Generate();
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

	MbufPtr m = p1.Generate();
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

	MbufPtr m = p2.Generate();
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

	MbufPtr m = p2.Generate();
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

	MbufPtr m = p2.Generate();
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

	MbufPtr m = p2.Generate();
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

	MbufPtr m = p.Generate();
	this->CheckLengthField(m.get(), 4);

	auto p2 = p.WithHeader(Layer::PAYLOAD).Fields(appendPayload({1,2,3,4,5}));

	m = p2.Generate();
	this->CheckLengthField(m.get(), 9);

	auto p3 = p.With(payload({1, 2, 3}));

	m = p3.Generate();
	this->CheckLengthField(m.get(), 3);
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

	MbufPtr m = p.Generate();
	this->CheckLengthField(m.get(), 1);

	auto p2 = p.With(appendPayload(0x77, 3));

	m = p2.Generate();
	this->CheckLengthField(m.get(), 4);

	auto p3 = p.WithHeader(Layer::PAYLOAD).Fields(payload(0x11, 10));

	m = p3.Generate();
	this->CheckLengthField(m.get(), 10);
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

	MbufPtr m = p.Generate();
	this->CheckLengthField(m.get(), 10);

	auto p2 = p.WithHeader(Layer::PAYLOAD).Fields(appendPayload("8596"));

	m = p2.Generate();
	this->CheckLengthField(m.get(), 14);

	auto p3 = p.With(payload("fmjdlkjfdklfj", 39));

	m = p3.Generate();
	this->CheckLengthField(m.get(), 39);
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

	MbufPtr m = p2.Generate();
	this->CheckLengthField(m.get(), 6);

	auto p3 = p.With(length(0));

	m = p3.Generate();
	this->CheckLengthField(m.get(), 0);
}
