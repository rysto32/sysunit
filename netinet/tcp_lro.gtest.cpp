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

#define _KERNEL_UT 1

#include <stdarg.h>

#include "pktgen/Ethernet.h"
#include "pktgen/Ipv4.h"
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

#include "mock/ifnet.h"
#include "mock/time.h"

// Debug printing
void PrintIndent(int depth, const char * fmt, ...)
{
	for (int i = 0; i < depth; ++i)
		printf("    ");

	va_list args;
	va_start (args, fmt);
	vprintf(fmt, args);
	va_end(args);

	printf("\n");
}


const char * LayerStr(PktGen::Layer l)
{
	switch (l) {
		case PktGen::Layer::L2:
			return "L2";
		case PktGen::Layer::L3:
			return "L3";
		case PktGen::Layer::L4:
			return "L4";
		case PktGen::Layer::PAYLOAD:
			return "PAYLOAD";
		default:
			return "UNKNOWN";
	}
}

using namespace PktGen;
using namespace testing;

int ipforwarding;
int ip6_forwarding;

class TcpLroTestSuite : public SysUnit::TestSuite
{
};

// Generate a single TCP/IPv4 packet and send it through tcp_lro_rx().  Verify
// that the packet is passed to if_input(), and that it's unmodified.
TEST_F(TcpLroTestSuite, TestSingleTcp4)
{
	// Create a packet template.  This template describes a TCP/IP packet
	// with a payload of 7 nul (0x00) bytes.  The mbuf will have flags set
	// to indicate that hardware checksum offload verified the L3/L4
	// and the checksums passed the check.
	auto pktTemplate = PacketTemplate(
	    EthernetHeader()
	        .With(
		    srcMac("02:f0:e0:d0:c0:b0"),
		    dstMac("02:05:04:0c:02:01")
		),
	     Ipv4Header()
	        .With(
	            srcIp("192.168.1.1"),
		    dstIp("192.168.1.10"),
		    checksumVerified(),
		    checksumPassed()
		 ),
	    TcpHeader()
	        .With(
		    srcPort(11965),
		    dstPort(54321),
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
	// The MockTime interface is used to implement time-based APIs.  In this
	// case it's used for its implementation of getmicrotime().
	GlobalMockTime mockTv;

	// A MockIfnet instance mocks the APIs provided by callbacks in struct ifnet.
	// This ifnet will be named mock0
	StrictMock<MockIfnet> mockIfp("mock", 0);

	// Inform the mock to expect a single call to getmicrotime(), and specify the
	// value that will be returned to the code under test when it's called.
	mockTv.ExpectGetMicrotime({.tv_sec = 1, .tv_usec = 500});

	// Inform the mock that we expect tcp_lro to pass in a packet matching out
	// template exactly once.  The test will fail if if_input() is not called
	// exactly 1 time, or if it is called but with a packet that does not match
	// the template.
	EXPECT_CALL(mockIfp, if_input(PacketMatcher(pktTemplate)))
	    .Times(1);

	// Initialize tcp_lro's state
	struct lro_ctrl lc;
	tcp_lro_init(&lc);
	lc.ifp = mockIfp.GetIfp();

	// Pass an mbuf based on our template to tcp_lro_rx(), and test the return
	// value to confirm that tcp_lro_rx() accepted the mbuf.
	int ret = tcp_lro_rx(&lc, pktTemplate.Generate(), 0);
	ASSERT_EQ(ret, 0);

	// Flush tcp_lro.  This should cause the mbuf generated above to be flushed
	// and sent to if_input(), at which point the mockIfp will see it and verify
	// that the packet made it through tcp_lro without being modified.
	tcp_lro_flush_all(&lc);
}

// Create two packets from the same TCP/IPv4 flow in sequence and send them
// into tcp_lro.  Verify that that LRO merges the frames into a single larger
// frame with the headers of the first packet and a payload containing the
// combined payloads of both frames.
TEST_F(TcpLroTestSuite, TestMerge2Tcp4)
{
	// Create a packet template.  This template describes a TCP/IP packet
	// with a payload of 5 bytes (the characters "12345").  The mbuf will have
	// flags set to indicate that hardware checksum offload verified the
	// L3/L4 and the checksums passed the check.
	auto pktTemplate1 = PacketTemplate(
	    EthernetHeader()
	        .With(
		    srcMac("02:f0:e0:d0:c0:b0"),
		    dstMac("02:05:04:0c:02:01")
		),
	     Ipv4Header()
	        .With(
	            srcIp("192.168.1.1"),
		    dstIp("192.168.1.10"),
		    checksumVerified(),
		    checksumPassed()
		 ),
	    TcpHeader()
	        .With(
		    srcPort(11965),
		    dstPort(54321),
		    seq(258958),
		    checksumVerified(),
		    checksumPassed()
		),
	    PacketPayload()
	        .With(
		    payload("12345")
		 )
	);

	// Generate a second template that is the next in the TCP sequence.
	// This will have a 3-byte payload ("678")
	auto pktTemplate2 = pktTemplate1.Next()
	    .WithHeaders<Layer::PAYLOAD>(
		payload("678")
	    )
	;

	// Generate a template representing what we expect to be sent to
	// if_input().  The packet should have the same headers as the
	// first template, but with the 3-byte payload of the second
	// packet appended.
	auto expected = pktTemplate1.With(appendPayload("678"));

	// Initialize the same mocks as last time.
	GlobalMockTime mockTv;
	StrictMock<MockIfnet> mockIfp("mock", 0);

	// getmicrotime() is called once per packet, so set it to expect to be
	// called twice.  The second call will look like it happend 250us after
	// the first.
	mockTv.ExpectGetMicrotime({.tv_sec = 1, .tv_usec = 500});
	mockTv.ExpectGetMicrotime({.tv_sec = 1, .tv_usec = 750});

	// Tell the mockIfp to expect the single merged packet
	EXPECT_CALL(mockIfp, if_input(PacketMatcher(expected)))
	    .Times(1);

	// Initialize the code that we're testing.
	struct lro_ctrl lc;
	tcp_lro_init(&lc);
	lc.ifp = mockIfp.GetIfp();

	// Send the two frames in sequence to tcp_lro_rx().  Test the return
	// value from each call.
	int ret = tcp_lro_rx(&lc, pktTemplate1.Generate(), 0);
	ASSERT_EQ(ret, 0);

	ret = tcp_lro_rx(&lc, pktTemplate2.Generate(), 0);
	ASSERT_EQ(ret, 0);

	// Flush tcp_lro.  If LRO is working then if_input() will receive a
	// single merged packet, which it will validate against the template
	// we provided.
	tcp_lro_flush_all(&lc);
}

