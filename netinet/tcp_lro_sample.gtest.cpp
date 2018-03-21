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

#include "pktgen/Ethernet.h"
#include "pktgen/Ipv4.h"
#include "pktgen/Ipv6.h"
#include "pktgen/Packet.h"
#include "pktgen/PacketMatcher.h"
#include "pktgen/PacketPayload.h"
#include "pktgen/Tcp.h"

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

#include "mock/UpperIfnet.h"
#include "mock/time.h"

using namespace PktGen;
using namespace testing;
using SysUnit::MockTime;
using SysUnit::MockUpperIfnet;

int ipforwarding;
int ip6_forwarding;

class TcpLroSampleTestSuite : public SysUnit::TestSuite
{
};

// Generate a single TCP/IPv4 packet and send it through tcp_lro_rx().  Verify
// that the packet is passed to if_input(), and that it's unmodified.
TEST_F(TcpLroSampleTestSuite, TestSingleTcp4)
{
	// Create a packet template.  This template describes a TCP/IPv4 packet
	// with a payload of 7 nul (0x00) bytes.  The mbuf will have flags set
	// to indicate that hardware checksum offload verified the L3/L4
	// and the checksums passed the check.
	// The type is described as "auto" as writing out the actual type would
	// be infeasible due to the heavy use of C++ templates in the packet
	// template API.
	auto pktTemplate = PacketTemplate(
	    EthernetHeader()
	        .With(
		    src("00:35:59:25:ea:90"),
		    dst("25:36:49:49:36:25")
		),
	     Ipv4Header()
	        .With(
	            src("192.168.1.1"),
		    dst("192.168.1.10"),
		    checksumVerified(),
		    checksumPassed()
		 ),
	    TcpHeader()
	        .With(
		    src(11965),
		    dst(54321),
		    checksumVerified(),
		    checksumPassed()
		),
	    PacketPayload()
	        .With(
		    payload(0x00, 7)
		 )
	);

	// Initialize mocks.  Mocks are used to implement kernel APIs depended
	// on by the code being tested.

	StrictMock<MockUpperIfnet> mockIfp("mock", 0);

	// Inform the mock that we expect tcp_lro to pass in a packet matching out
	// template exactly once.  The test will fail if if_input() is not called
	// exactly 1 time, or if it is called but with a packet that does not match
	// the template.
	EXPECT_CALL(mockIfp, if_input(PacketMatcher(pktTemplate)))
	    .Times(1);

	// The MockTime interface is used to implement time-based APIs.  In this
	// case it's used for its implementation of getmicrotime().
	// Inform the mock to expect a single call to getmicrotime(), and specify the
	// value that will be returned to the code under test when it's called.
	MockTime::ExpectGetMicrotime({.tv_sec = 1, .tv_usec = 500});

	// Test setup is complete.  The code that follows is the actual test case.
	struct lro_ctrl lc;
	tcp_lro_init(&lc);
	lc.ifp = mockIfp.GetIfp();

	// Pass an mbuf based on our template to tcp_lro_rx(), and test the return
	// value to confirm that tcp_lro_rx() accepted the mbuf.
	int ret = tcp_lro_rx(&lc, pktTemplate.GenerateRawMbuf(), 0);
	ASSERT_EQ(ret, 0);

	// Flush tcp_lro.  This should cause the mbuf input above to be flushed
	// and sent to if_input(), at which point the mockIfp will see it and verify
	// that the packet made it through tcp_lro without being modified.
	tcp_lro_flush_all(&lc);

	tcp_lro_free(&lc);
}

// Generate a single TCP/IPv6 packet and send it through tcp_lro_rx().  Verify
// that the packet is passed to if_input(), and that it's unmodified.
TEST_F(TcpLroSampleTestSuite, TestSingleTcp6)
{
	// Create a packet template for a TCP/IPv6 packet
	auto pktTemplate = PacketTemplate(
	    EthernetHeader()
	        .With(
		    src("00:35:59:25:ea:90"),
		    dst("25:36:49:49:36:25")
		),
	     Ipv6Header()
	        .With(
	            src("0558::8732:44"),
		    dst("30::01")
		 ),
	    TcpHeader()
	        .With(
		    src(11965),
		    dst(54321),
		    checksumVerified(),
		    checksumPassed()
		),
	    PacketPayload()
	        .With(
		    payload(0x00, 7)
		 )
	);

	// Initialize mocks.

	StrictMock<MockUpperIfnet> mockIfp("mock", 0);

	// Inform the mock that we expect tcp_lro to pass in a packet matching out
	// template exactly once.  The test will fail if if_input() is not called
	// exactly 1 time, or if it is called but with a packet that does not match
	// the template.
	EXPECT_CALL(mockIfp, if_input(PacketMatcher(pktTemplate)))
	    .Times(1);

	// Inform the mock to expect a single call to getmicrotime(), and specify the
	// value that will be returned to the code under test when it's called.
	MockTime::ExpectGetMicrotime({.tv_sec = 1, .tv_usec = 500});

	// Test setup is complete.  The code that follows is the actual test case.
	struct lro_ctrl lc;
	tcp_lro_init(&lc);
	lc.ifp = mockIfp.GetIfp();

	// Pass an mbuf based on our template to tcp_lro_rx(), and test the return
	// value to confirm that tcp_lro_rx() accepted the mbuf.
	int ret = tcp_lro_rx(&lc, pktTemplate.GenerateRawMbuf(), 0);
	ASSERT_EQ(ret, 0);

	// Flush tcp_lro.  This should cause the mbuf input above to be flushed
	// and sent to if_input(), at which point the mockIfp will see it and verify
	// that the packet made it through tcp_lro without being modified.
	tcp_lro_flush_all(&lc);

	tcp_lro_free(&lc);
}

// Send a packet with an invalid IP header length field, and verify that tcp_lro
// rejects it.
TEST_F(TcpLroSampleTestSuite, TestBadIpHeaderLen)
{
	auto pkt = PacketTemplate(
	    EthernetHeader()
	        .With(
		    src("00:35:59:25:ea:90"),
		    dst("25:36:49:49:36:25")
		),
	     Ipv4Header()
	        .With(
	            src("192.168.1.1"),
		    dst("192.168.1.10"),
		    headerLength(3),
		    checksumVerified(),
		    checksumPassed()
		 ),
	    TcpHeader()
	        .With(
		    src(11965),
		    dst(54321),
		    checksumVerified(),
		    checksumPassed()
		),
	    PacketPayload()
	        .With(
		    payload("FreeBSD", 25)
		 )
	);

	StrictMock<MockUpperIfnet> mockIfp("mock", 0);

	// Begin the testcase.
	struct lro_ctrl lc;
	tcp_lro_init(&lc);
	lc.ifp = mockIfp.GetIfp();

	// Send the invalid frame to tcp_lro_rx() and confirm that it is rejected
	MbufUniquePtr m = pkt.Generate();
	int ret = tcp_lro_rx(&lc, m.get(), 0);
	ASSERT_EQ(ret, TCP_LRO_CANNOT);

	tcp_lro_free(&lc);
}
