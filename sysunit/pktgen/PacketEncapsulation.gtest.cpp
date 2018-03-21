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

#include "pktgen/Packet.h"

#include "pktgen/Ethernet.h"
#include "pktgen/Ipv4.h"
#include "pktgen/Ipv6.h"
#include "pktgen/PacketPayload.h"
#include "pktgen/Tcp.h"

#include "sysunit/TestSuite.h"

#include <gtest/gtest.h>

#include <stubs/sysctl.h>
#include <stubs/uio.h>

using namespace PktGen;
using internal::GetMbufHeader;

class PacketEncapsulationTestSuite : public SysUnit::TestSuite
{
public:
	template <typename Template, typename Expected>
	void TestPayload(const Template & p, const Expected & expectedHeaders)
	{
		MbufUniquePtr m = p.Generate();

		ASSERT_EQ(m->m_pkthdr.len, sizeof(Expected));

		uint8_t * pkt = GetMbufHeader<uint8_t>(m);
		const uint8_t * expected = reinterpret_cast<const uint8_t*>(&expectedHeaders);
		for (size_t i = 0; i < sizeof(Expected); ++i) {
			ASSERT_EQ(pkt[i], expected[i]) << "Mismatch at offset " << i;
		}
	}
};

TEST_F(PacketEncapsulationTestSuite, TestTcpIpv4)
{
	auto p = PacketTemplate(
		EthernetHeader(),
		Ipv4Header(),
		TcpHeader(),
		PacketPayload()
	);

	auto p2 = p
		.WithHeader(Layer::L2).Fields(
			src("00:01:02:03:04:05"),
			dst("ff:ff:ff:ff:ff:ff")
		).WithHeader(Layer::L3).Fields(
			src("10.1.2.3"),
			dst("11.3.2.1"),
			ttl(255)
		).WithHeader(Layer::L4).Fields(
			src(11965),
			dst(80),
			seq(2568),
			ack(69541),
			flags(TH_ACK),
			window(4)
		).WithHeader(Layer::PAYLOAD).Fields(
			payload("12345")
		);

	struct {
		struct ether_header eh;
		struct ip ip;
		struct tcphdr tcp;
		char payload[5];
	} __packed expectedHeaders  = {
		.eh = {
			.ether_shost = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 },
			.ether_dhost = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
			.ether_type = ntohs(ETHERTYPE_IP),
		},
		.ip = {
			.ip_v = 4,
			.ip_hl = sizeof(struct ip) / sizeof(uint32_t),
			.ip_len = htons(sizeof(expectedHeaders) - sizeof(struct ether_header)),
			.ip_src.s_addr = htonl(0x0a010203),
			.ip_dst.s_addr = htonl(0x0b030201),
			.ip_ttl = 255,
			.ip_p = IPPROTO_TCP,
		},
		.tcp = {
			.th_sport = htons(11965),
			.th_dport = htons(80),
			.th_off = sizeof(struct tcphdr) / sizeof(uint32_t),
			.th_seq = htonl(2568),
			.th_ack = htonl(69541),
			.th_flags = TH_ACK,
			.th_win = htons(4)
		},
		.payload = { '1', '2', '3', '4', '5' }
	};

	TestPayload(p2, expectedHeaders);
}


template <typename IP>
struct Headers {
	struct ether_header eh;
	IP ip;
	struct tcphdr tcp;
} __packed;

// Create a packet that is TCP/IP in TCP/IP in TCP/IPv6, and verify that
// we can set fields in the template via Layer::L*, Layer::OUTER_*,
// LAYER::INNER* and NestedLayer::*.
TEST_F(PacketEncapsulationTestSuite, TestDoubleInner)
{
	auto p1 = PacketTemplate(
		EthernetHeader(),
		Ipv6Header(),
		TcpHeader(),
		EthernetHeader(),
		Ipv4Header(),
		TcpHeader(),
		EthernetHeader(),
		Ipv4Header(),
		TcpHeader(),
		PacketPayload()
	);

	struct ExpectedHeaders {
		struct Headers<ip6_hdr> outer;
		struct Headers<ip> middle;
		struct Headers<ip> inner;
		char payload[8];
	} __packed expected  = {
		.outer = {
			.eh = {
				.ether_shost = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 },
				.ether_dhost = { 0x04, 0x09, 0x16, 0x25, 0x36, 0x49 },
				.ether_type = ntohs(ETHERTYPE_IPV6),
			},
			.ip = {
				.ip6_flow = htonl(0x60000000),
				.ip6_plen = htons(sizeof(ExpectedHeaders) - sizeof(struct ether_header) - sizeof(struct ip6_hdr)),
				.ip6_hlim = 255,
				.ip6_nxt = IPPROTO_TCP,
				.ip6_src.s6_addr = {0xde, 0xad, 0xc0, 0xde},
				.ip6_dst.s6_addr = {0xca, 0xfe, 0xba, 0xbe},
			},
			.tcp = {
				.th_sport = htons(5891),
				.th_dport = htons(23),
				.th_off = sizeof(struct tcphdr) / sizeof(uint32_t),
				.th_seq = htonl(2652),
				.th_ack = htonl(89247),
				.th_flags = TH_ACK | TH_PUSH,
				.th_win = htons(16),
			},
		},
		.middle = {
			.eh = {
				.ether_shost = { 0x01, 0x01, 0x02, 0x03, 0x05, 0x08 },
				.ether_dhost = { 0x02, 0x01, 0x03, 0x04, 0x07, 0x0b },
				.ether_type = ntohs(ETHERTYPE_IP),
			},
			.ip = {
				.ip_v = 4,
				.ip_hl = sizeof(struct ip) / sizeof(uint32_t),
				.ip_len = htons(sizeof(ExpectedHeaders) - offsetof(struct ExpectedHeaders, middle.ip)),
				.ip_src.s_addr = htonl(0x7f000001),
				.ip_dst.s_addr = htonl(0x7f000001),
				.ip_ttl = 36,
				.ip_p = IPPROTO_TCP,
			},
			.tcp = {
				.th_sport = htons(21524),
				.th_dport = htons(285),
				.th_off = sizeof(struct tcphdr) / sizeof(uint32_t),
				.th_seq = htonl(324),
				.th_ack = htonl(211861),
				.th_flags = 0,
				.th_win = htons(1)
			},
		},
		.inner = {
			.eh = {
				.ether_shost = { 0x03, 0x14, 0x15, 0x92, 0x65, 0x35 },
				.ether_dhost = { 0x02, 0x71, 0x82, 0x81, 0x82, 0x84 },
				.ether_type = ntohs(ETHERTYPE_IP),
			},
			.ip = {
				.ip_v = 4,
				.ip_hl = sizeof(struct ip) / sizeof(uint32_t),
				.ip_len = htons(sizeof(ExpectedHeaders) - offsetof(struct ExpectedHeaders, inner.ip)),
				.ip_src.s_addr = htonl(0x0f0f0f0f),
				.ip_dst.s_addr = htonl(0x80808080),
				.ip_ttl = 128,
				.ip_p = IPPROTO_TCP,
			},
			.tcp = {
				.th_sport = htons(258),
				.th_dport = htons(639),
				.th_off = sizeof(struct tcphdr) / sizeof(uint32_t),
				.th_seq = htonl(14896),
				.th_ack = htonl(398),
				.th_flags = TH_ACK,
				.th_win = htons(4)
			},
		},
		.payload = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' }
	};

	auto p2 = p1
		.WithHeader(Layer::OUTER_L2).Fields(
			src("00:01:02:03:04:05"),
			dst("04:09:16:25:36:49")
		).WithHeader(Layer::L3).Fields(
			src("dead:c0de::"),
			dst("cafe:babe::"),
			hopLimit(255)
		).WithHeader(NestedLayer::L4<1>).Fields(
			src(5891),
			dst(23),
			seq(2652),
			ack(89247),
			flags(TH_ACK | TH_PUSH),
			window(16)
		).WithHeader(NestedLayer::L2<2>).Fields(
			src("01:01:02:03:05:08"),
			dst("02:01:03:04:07:0b")
		).WithHeader(NestedLayer::L3<2>).Fields(
			src("127.0.0.1"),
			dst("127.0.0.1"),
			ttl(36)
		).WithHeader(NestedLayer::L4<-2>).Fields(
			src(21524),
			dst(285),
			seq(324),
			ack(211861),
			flags(0),
			window(1)
		).WithHeader(Layer::INNER_L2).Fields(
			src("03:14:15:92:65:35"),
			dst("02:71:82:81:82:84")
		).WithHeader(NestedLayer::L3<3>).Fields(
			src("15.15.15.15"),
			dst("128.128.128.128"),
			ttl(128)
		).WithHeader(NestedLayer::L4<-1>).Fields(
			src(258),
			dst(639),
			seq(14896),
			ack(398),
			flags(TH_ACK),
			window(4)
		).WithHeader(NestedLayer::PAYLOAD<-1>).Fields(
			payload("abcdefgh")
		);

	TestPayload(p2, expected);
}
