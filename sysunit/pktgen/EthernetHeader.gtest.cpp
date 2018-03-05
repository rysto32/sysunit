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

#include "pktgen/EthernetHeader.h"
#include "pktgen/Packet.h"
#include "pktgen/PacketPayload.h"

#include "sysunit/TestSuite.h"

#include <gtest/gtest.h>

#include <stubs/sysctl.h>
#include <stubs/uio.h>

using namespace PktGen;

class EthernetHeaderTestSuite : public SysUnit::TestSuite
{
public:
	void CheckMbufMatches(struct mbuf * m, const struct ether_header * expected)
	{
		auto * eh = GetMbufHeader<struct ether_header>(m, 0);
		EXPECT_EQ(eh->ether_dhost[0], expected->ether_dhost[0]);
		EXPECT_EQ(eh->ether_dhost[1], expected->ether_dhost[1]);
		EXPECT_EQ(eh->ether_dhost[2], expected->ether_dhost[2]);
		EXPECT_EQ(eh->ether_dhost[3], expected->ether_dhost[3]);
		EXPECT_EQ(eh->ether_dhost[4], expected->ether_dhost[4]);
		EXPECT_EQ(eh->ether_dhost[5], expected->ether_dhost[5]);

		EXPECT_EQ(eh->ether_shost[0], expected->ether_shost[0]);
		EXPECT_EQ(eh->ether_shost[1], expected->ether_shost[1]);
		EXPECT_EQ(eh->ether_shost[2], expected->ether_shost[2]);
		EXPECT_EQ(eh->ether_shost[3], expected->ether_shost[3]);
		EXPECT_EQ(eh->ether_shost[4], expected->ether_shost[4]);
		EXPECT_EQ(eh->ether_shost[5], expected->ether_shost[5]);

		EXPECT_EQ(eh->ether_type, expected->ether_type);
	}

	template <typename Header>
	void ExpectTemplateMatches(const Header & header,
	    const struct ether_header * expected, uint16_t vlan = 0)
	{
		struct mbuf * m = header.Generate();

		EXPECT_EQ(m->m_flags & M_PKTHDR, M_PKTHDR);
		EXPECT_EQ(m->m_len, sizeof(struct ether_header));
		EXPECT_EQ(m->m_pkthdr.len, sizeof(struct ether_header));

		CheckMbufMatches(m, expected);

		if (vlan == 0) {
			EXPECT_EQ(m->m_flags & M_VLANTAG, 0);
		} else {
			EXPECT_EQ(m->m_flags & M_VLANTAG, M_VLANTAG);
			EXPECT_EQ(m->m_pkthdr.ether_vtag, vlan);
		}

		m_freem(m);
	}

	template <typename Header>
	void VerifyMbufChainLen(const Header & header, size_t expectedPayload)
	{
		struct mbuf * m = header.Generate();

		EXPECT_EQ(m->m_pkthdr.len,
		    sizeof(struct ether_header) + expectedPayload);
		m_freem(m);
	}
};

// Generate an ethernet packet based on a template and check that the packet's
// fields are set to the specified values.
TEST_F(EthernetHeaderTestSuite, GenerateEthernet)
{
	auto p = PacketTemplate(EthernetHeader()
	    .With(dst("6f:5e:4d:3c:2b:1a"),
		  src("01:02:03:04:05:06"),
		  ethertype(0x9147)
	     ));

	struct ether_header expected = {
		.ether_dhost = {0x6f, 0x5e, 0x4d, 0x3c, 0x2b, 0x1a},
		.ether_shost = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06},
		.ether_type = ntohs(0x9147)
	};

	ExpectTemplateMatches(p, &expected);
}

// Take an ethernet template and apply all supported mutators to it to generate
// a new template.  Verify that the original template is unmodified, and that
// the new template generates packets with the new values.
TEST_F(EthernetHeaderTestSuite, MutateHeader)
{
	auto p1 = PacketTemplate(EthernetHeader()
	    .With(dst("6f:5e:4d:3c:2b:1a"),
		  src("01:02:03:04:05:06"),
		  ethertype(0x9147),
		  mbufVlan(2)
	     ));

	auto p2 = p1.WithHeader(Layer::L2).Fields(
	    dst("25:28:96:78:48:84"),
	    src("64:74:35:15:22:55"),
	    ethertype(0x1258),
	    mbufVlan(0)
	);

	struct ether_header expected1 = {
		.ether_dhost = {0x6f, 0x5e, 0x4d, 0x3c, 0x2b, 0x1a},
		.ether_shost = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06},
		.ether_type = ntohs(0x9147)
	};

	struct ether_header expected2 = {
		.ether_dhost = {0x25, 0x28, 0x96, 0x78, 0x48, 0x84},
		.ether_shost = {0x64, 0x74, 0x35, 0x15, 0x22, 0x55},
		.ether_type = ntohs(0x1258)
	};

	ExpectTemplateMatches(p1, &expected1, 2);
	ExpectTemplateMatches(p2, &expected2);
}

// Test that the Next() and Retransmission() methods on an Ethernet template
// generate exactly the same header as the original template.  Ethernet
// headers have no fields that vary when retransmitting or from packet to
// packet in a connection.
TEST_F(EthernetHeaderTestSuite, TestNext)
{
	auto p1 = PacketTemplate(EthernetHeader()
	    .With(dst("5c:3a:47:20:08:13"),
		  src("1f:1e:61:5c:51:21"),
		  ethertype(0x1acd),
		  mbufVlan(56)
	     ));

	auto p2 = p1.Next();
	auto p3 = p1.Retransmission();

	struct mbuf *m = p1.Generate();
	struct ether_header *expected = GetMbufHeader<ether_header>(m, 0);

	ExpectTemplateMatches(p2, expected, 56);
	ExpectTemplateMatches(p3, expected, 56);
	m_freem(m);
}

// Generate an ethernet packet with a large payload embedded in it.  Verify
// that the mbuf chain is long enough to accomodate the payload.
TEST_F(EthernetHeaderTestSuite, TestPayloadSize)
{
	auto p1 = PacketTemplate(EthernetHeader(), PacketPayload());
	VerifyMbufChainLen(p1, 0);

	auto p2 = p1.WithHeader(Layer::PAYLOAD).Fields(payload("12345"));
	VerifyMbufChainLen(p2, 5);

	auto p3 = p1.WithHeader(Layer::PAYLOAD).Fields(payload(0, 8000));
	VerifyMbufChainLen(p3, 8000);
}
