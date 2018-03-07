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
