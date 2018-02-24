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

#define GTEST_HAS_EXCEPTIONS 1
#define _KERNEL_UT 1
#define _KERNEL_UT_NO_USERLAND_CONFLICTS 1

#include "pktgen/EtherFlow.h"
#include "pktgen/Ipv4Flow.h"
#include "pktgen/TcpFlow.h"

#include "pktgen/EtherExpectation.h"
#include "pktgen/Ipv4Expectation.h"
#include "pktgen/PacketExpectationList.h"
#include "pktgen/PayloadExpectation.h"
#include "pktgen/TcpExpectation.h"

extern "C" {
#include <kern_include/net/if.h>
#include <kern_include/net/if_var.h>
#include <kern_include/netinet/in.h>
#include <kern_include/netinet/tcp_lro.h>
}

#include <stubs/sysctl.h>
#include <stubs/uio.h>

#include <gtest/gtest.h>

#include "sysunit/TestSuite.h"

#include "mock/ifnet.h"
#include "mock/timeval.h"

using namespace PktGen;
using namespace testing;

int ipforwarding;
int ip6_forwarding;

class TcpLroTestSuite : public SysUnit::TestSuite
{
};

TEST_F(TcpLroTestSuite, TestSingleTcp4)
{
	GlobalMockTimeval mockTv;
	StrictMock<MockIfnet> mockIfp("test", 0);
	struct lro_ctrl lc;

	tcp_lro_init(&lc);
	lc.ifp = mockIfp.GetIfp();

	mockTv.ExpectGetMicrotime({.tv_sec = 1, .tv_usec = 500});

	EtherFlow ether("02:01:02:03:04:05", "02:05:04:03:02:01");
	Ipv4Flow ip(ether, "192.168.1.1", "192.168.1.10");
	TcpFlow tcp(ip, 11965, 54321, 0, 0);

	Packet m = tcp.GetNextPacket(0, 5, "12345");

	EXPECT_CALL(mockIfp, if_input(_))
	  .Times(1)
	  .WillOnce(Invoke(PacketExpectationList({
	    new EtherExpectation(ether, ip.GetEtherType()),
	    new Ipv4Expectation(ip, tcp.GetIpProto(), m.GetL3Len()),
	    new TcpExpectation(tcp),
	    new PayloadExpectation("12345", 5)
	})));

	m->m_pkthdr.csum_flags |= CSUM_IP_CHECKED | CSUM_IP_VALID | CSUM_DATA_VALID | CSUM_PSEUDO_HDR;

	int ret = tcp_lro_rx(&lc, m.release(), 0);
	ASSERT_EQ(ret, 0);

	tcp_lro_flush_all(&lc);
}

TEST_F(TcpLroTestSuite, TestMergeTcp4)
{
	GlobalMockTimeval mockTv;
	StrictMock<MockIfnet> mockIfp("test", 0);
	struct lro_ctrl lc;

	tcp_lro_init(&lc);
	lc.ifp = mockIfp.GetIfp();

	mockTv.ExpectGetMicrotime({.tv_sec = 1, .tv_usec = 500});
	mockTv.ExpectGetMicrotime({.tv_sec = 1, .tv_usec = 700});

	EtherFlow ether("02:01:02:03:04:05", "02:05:04:03:02:01");
	Ipv4Flow ip(ether, "192.168.1.1", "192.168.1.10");
	TcpFlow tcp(ip, 11965, 54321, 0, 0);

	const auto csum_flags = CSUM_IP_CHECKED | CSUM_IP_VALID | CSUM_DATA_VALID | CSUM_PSEUDO_HDR;

	const auto first_payload_len = 2*5;

	Packet m = tcp.GetNextPacket(0, first_payload_len, "12345");
	m->m_pkthdr.csum_flags |= csum_flags;

	const auto second_payload_len = 2*4;

	EXPECT_CALL(mockIfp, if_input(_))
	  .Times(1)
	  .WillOnce(Invoke(PacketExpectationList({
	    new EtherExpectation(ether, ip.GetEtherType()),
	    new Ipv4Expectation(ip, tcp.GetIpProto(), m.GetL3Len() + second_payload_len),
	    new TcpExpectation(tcp),
	    new PayloadExpectation("123451234567896789", first_payload_len + second_payload_len)
	})));

	Packet m2 = tcp.GetNextPacket(0, second_payload_len, "6789");
	m2->m_pkthdr.csum_flags |= csum_flags;

	int ret = tcp_lro_rx(&lc, m.release(), 0);
	ASSERT_EQ(ret, 0);

	ret = tcp_lro_rx(&lc, m2.release(), 0);
	ASSERT_EQ(ret, 0);

	tcp_lro_flush_all(&lc);
}

