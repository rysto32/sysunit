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
#include "pktgen/Ipv6.h"
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

using namespace PktGen;
using namespace testing;

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

	StrictMock<MockIfnet> mockIfp("mock", 0);

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
	int ret = tcp_lro_rx(&lc, pktTemplate.Generate(), 0);
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
	            src("05:58::87:32:44"),
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

	StrictMock<MockIfnet> mockIfp("mock", 0);

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
	int ret = tcp_lro_rx(&lc, pktTemplate.Generate(), 0);
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

	StrictMock<MockIfnet> mockIfp("mock", 0);

	// Begin the testcase.
	struct lro_ctrl lc;
	tcp_lro_init(&lc);
	lc.ifp = mockIfp.GetIfp();

	// Send the invalid frame to tcp_lro_rx() and confirm that it is rejected
	struct mbuf * m = pkt.Generate();
	int ret = tcp_lro_rx(&lc, m, 0);
	ASSERT_EQ(ret, TCP_LRO_CANNOT);

	// When tcp_lro_rx() rejects a frame the caller retains ownership of the
	// mbuf, so free it now that we're done with it.
	m_freem(m);

	tcp_lro_free(&lc);
}

template <typename NetworkLayerTemplate>
class TcpLroTestSuite : public SysUnit::TestSuite
{
public:
	struct lro_ctrl lc;
	std::unique_ptr<StrictMock<MockIfnet>> mockIfp;

	void TestCaseSetUp() override
	{
		// A MockIfnet instance mocks the APIs provided by callbacks in struct ifnet.
		// This ifnet will be named mock0
		mockIfp = std::make_unique<StrictMock<MockIfnet>>("mock", 0);

		tcp_lro_init(&lc);
		lc.ifp = mockIfp->GetIfp();
	}

	void TestCaseTearDown() override
	{
		tcp_lro_free(&lc);
		mockIfp.reset();
	}

	// Generate a template for the network (L3) layer header.  This will
	// have specializations for IPv4 and IPv6 tests to generate the correct
	// header.
	static NetworkLayerTemplate GetNetworkLayerTemplate();

	// For convenience, this function can be called to create a
	// packet template for a pure TCP packet (no payload)
	static auto GetTcpTemplate()
	{
		return PacketTemplate(
		    EthernetHeader()
			.With(
			src("02:f0:e0:d0:c0:b0"),
			dst("02:05:04:0c:02:01")
			),
		    GetNetworkLayerTemplate(),
		    TcpHeader()
			.With(
			src(6995),
			dst(123),
			checksumVerified(),
			checksumPassed()
		    )
		);
	}

	// This generates a packet template of a TCP/IP packet with a payload
	static auto GetPayloadTemplate()
	{
		return PacketTemplate(
			GetTcpTemplate(),
			PacketPayload()
		);

	}

	// Run a test case that sends pkt1 and pkt2 to tcp_lro_rx() in that order, and expects the first
	// packet to be accepted but the second to be rejected with the given error code.
	template <typename PktTemplate>
	void TestRejectSecond(const PktTemplate & pkt1, const PktTemplate & pkt2, int failCode = TCP_LRO_CANNOT);
};

typedef std::invoke_result<decltype(Ipv4Header)>::type Ipv4TemplateType;
typedef std::invoke_result<decltype(Ipv6Header)>::type Ipv6TemplateType;

// Generate an IPv4 header template.
template <>
Ipv4TemplateType TcpLroTestSuite<Ipv4TemplateType>::GetNetworkLayerTemplate()
{
	return Ipv4Header()
	    .With(
	        src("1.25.37.187"),
	        dst("57.8.9.63"),
	        checksumVerified(),
	        checksumPassed()
	    );
}

// Generate a IPv6 header template.
template <>
Ipv6TemplateType TcpLroTestSuite<Ipv6TemplateType>::GetNetworkLayerTemplate()
{
	return Ipv6Header()
	    .With(
	        src("05:58::87:32:44"),
	        dst("30::01")
	    );
}

typedef ::testing::Types<Ipv4TemplateType, Ipv6TemplateType> NetworkTypes;
TYPED_TEST_CASE(TcpLroTestSuite, NetworkTypes);

// Create two packets from the same TCP/IPv4 flow in sequence and send them
// into tcp_lro.  Verify that that LRO merges the frames into a single larger
// frame with the headers of the first packet and a payload containing the
// combined payloads of both frames.
TYPED_TEST(TcpLroTestSuite, TestMerge2Tcp4)
{
	// Create a packet template.  This template describes a TCP/IP packet
	// with a payload of 5 bytes (the characters "12345").  The mbuf will have
	// flags set to indicate that hardware checksum offload verified the
	// L3/L4 and the checksums passed the check.
	auto pktTemplate1 = this->GetPayloadTemplate()
	    .WithHeader(Layer::L4).Fields(seq(258958))
	    .WithHeader(Layer::PAYLOAD).Fields(payload("12345"));

	// Generate a second template that is the next in the TCP sequence.
	// This will have a 3-byte payload ("678")
	auto pktTemplate2 = pktTemplate1.Next()
	    .WithHeader(Layer::PAYLOAD).Fields(payload("678"));

	// Generate a template representing what we expect to be sent to
	// if_input().  The packet should have the same headers as the
	// first template, but with the 3-byte payload of the second
	// packet appended.
	auto expected = pktTemplate1.With(appendPayload("678"));

	// getmicrotime() is called once per packet, so set it to expect to be
	// called twice.  The second call will look like it happend 250us after
	// the first.
	MockTime::ExpectGetMicrotime({.tv_sec = 1, .tv_usec = 500});
	MockTime::ExpectGetMicrotime({.tv_sec = 1, .tv_usec = 750});

	// Tell the mockIfp to expect the single merged packet
	EXPECT_CALL(*this->mockIfp, if_input(PacketMatcher(expected)))
	    .Times(1);

	// Begin the testcase

	// Send the two frames in sequence to tcp_lro_rx() and verify the
	// return value from each call.
	int ret = tcp_lro_rx(&this->lc, pktTemplate1.Generate(), 0);
	ASSERT_EQ(ret, 0);

	ret = tcp_lro_rx(&this->lc, pktTemplate2.Generate(), 0);
	ASSERT_EQ(ret, 0);

	// Flush tcp_lro.  If LRO is working then if_input() will receive a
	// single merged packet, which it will validate against the template
	// we provided.
	tcp_lro_flush_all(&this->lc);
}

// Send a packet to be queued in tcp_lro, then call tcp_lro_flush_inactive
// before the timeout has expired, and verify that tcp_lro continues to
// hold the packet.  Then call tcp_lro_flush after the timeout has expired
// and verify that the packet is sent to if_input at that time.
TYPED_TEST(TcpLroTestSuite, TestFlushInactive)
{
	auto pkt1 = this->GetPayloadTemplate()
	    .WithHeader(Layer::PAYLOAD).Fields(payload("abcd", 100));

	struct timeval timeout = { .tv_sec = 0, .tv_usec = 150 };

	{
		InSequence seq;

		// This call will be from tcp_lro_rx() when it timestamps the
		// arrival of the packet.  Note that the time that the LRO
		// code sees is completely controlled by the mock.  Any amount
		// of time can pass between getmicrotime() calls and the LRO
		// code will see the time delta between calls as always being
		// consistent.  This is the advantage of mocking out calls to
		// get the current time.  If you tried to force the test to take
		// exactly 100us of real time between each call, a fast machine
		// would waste time and a slow machine might take too long
		// between calls and have the test fail intermittently.
		MockTime::ExpectGetMicrotime({.tv_sec = 127, .tv_usec = 100});

		// This call will be from the first call to tcp_lro_flush_inactive()
		// Because the time difference between the arrival time and the
		// flush time is less than the timeout value, the packet will
		// not be flushed.
		MockTime::ExpectGetMicrotime({.tv_sec = 127, .tv_usec = 200});

		// This call will be from the second call to tcp_lro_flush_inactive()
		// Now enough time will have been elapsed for the timeout to have
		// expired, and the packet will be sent to if_input
		MockTime::ExpectGetMicrotime({.tv_sec = 127, .tv_usec = 300});

		// We require that the call to if_input() must happen after the
		// timeout period has passed.  This enforced through the use of
		// the InSequence sequence.  The test will fail if the if_input()
		// call preceeds any other mock call in the sequence.
		EXPECT_CALL(*this->mockIfp, if_input(PacketMatcher(pkt1)))
		    .Times(1);
	}

	// Begin the testcase.

	int ret = tcp_lro_rx(&this->lc, pkt1.Generate(), 0);
	ASSERT_EQ(ret, 0);

	// This call will occur before the timeout and will not flush the packet.
	tcp_lro_flush_inactive(&this->lc, &timeout);

	// This call will flush the packet.
	tcp_lro_flush_inactive(&this->lc, &timeout);
}

// Send a pure ACK from a TCP/IPv4 flow followed by a data frame.  Verify that
// tcp_lro merges the two packets into a a single frame.
TYPED_TEST(TcpLroTestSuite, TestMergeAckData)
{
	// This template represents a pure  ACK packet.
	auto pktTemplate1 = this->GetTcpTemplate()
	    .WithHeader(Layer::L3).Fields()
	    .WithHeader(Layer::L4).Fields(ack(889925), flags(TH_ACK));

	// Generate a second template that is the next in the TCP sequence.
	// This will have a 20-byte payload
	auto payloadTemplate = PacketPayload().With(payload("9581", 20));
	auto pktTemplate2 = PacketTemplate(
	    pktTemplate1.Next(),
	    payloadTemplate
	);

	// Generate a template representing what we expect to be sent to
	// if_input().  It will have all of the headers from the first
	// packet but the "9581" payload of the second one
	// (Note that the difference between this template and pktTemplate2
	// is that pktTemplate2 was generated with pktTemplate1.Next(),
	// which incremented the IP id.  The merged packet that will be
	// sent to if_input() will have the id of the original ACK.)
	auto expected = PacketTemplate(
	    pktTemplate1,
	    payloadTemplate
	);

	// getmicrotime() is called once per packet
	MockTime::ExpectGetMicrotime({.tv_sec = 1, .tv_usec = 500});
	MockTime::ExpectGetMicrotime({.tv_sec = 1, .tv_usec = 750});

	// Tell the this->mockIfp to expect the single merged packet
	EXPECT_CALL(*this->mockIfp, if_input(PacketMatcher(expected)))
	    .Times(1);

	// Begin the testcase.

	// Send the two frames in sequence to tcp_lro_rx().  Verify the return
	// value from each call.
	int ret = tcp_lro_rx(&this->lc, pktTemplate1.Generate(), 0);
	ASSERT_EQ(ret, 0);

	ret = tcp_lro_rx(&this->lc, pktTemplate2.Generate(), 0);
	ASSERT_EQ(ret, 0);

	// Flush tcp_lro.  If LRO is working then if_input() will receive a
	// single merged packet, which it will validate against the template
	// we provided.
	tcp_lro_flush_all(&this->lc);
}

// Send two packets with TH_ACK set, and the second with a th_ack higher
// than the first.  Verify that the merged packet sent up the stack has the
// th_ack value from the second packet.
TYPED_TEST(TcpLroTestSuite, TestIncrAck)
{
	auto pkt1 = this->GetPayloadTemplate()
	    .WithHeaderFields(Layer::L4, ack(965), flags(TH_ACK))
	    .WithHeader(Layer::PAYLOAD).Fields(payload("abcd", 100));

	auto pkt2 = pkt1.Next().WithHeader(Layer::L4).Fields(incrAck(1000));

	auto expected = pkt1
	    .WithHeader(Layer::L4).Fields(incrAck(1000))
	    .WithHeader(Layer::PAYLOAD).Fields(appendPayload("abcd", 100));

	EXPECT_CALL(*this->mockIfp, if_input(PacketMatcher(expected)))
	    .Times(1);

	// getmicrotime() is called once per accepted packet
	MockTime::ExpectGetMicrotime({.tv_sec = 89952, .tv_usec = 17935});
	MockTime::ExpectGetMicrotime({.tv_sec = 89952, .tv_usec = 18932});

	// Begin the testcase.

	// Send the two frames in sequence to tcp_lro_rx().  Test the return
	// value from each call.
	int ret = tcp_lro_rx(&this->lc, pkt1.Generate(), 0);
	ASSERT_EQ(ret, 0);

	// tcp_lro_rx() will detect the dup ACK here
	ret = tcp_lro_rx(&this->lc, pkt2.Generate(), 0);
	ASSERT_EQ(ret, 0);

	tcp_lro_flush_all(&this->lc);
}

// Send a data packet followed by a pure ACK with a larger th_ack field.
// Verify that the merged frame sent up the stack has the th_ack field from
// the pure ACK.
TYPED_TEST(TcpLroTestSuite, TestIncrPureAck)
{
	auto pkt1 = this->GetPayloadTemplate()
	    .WithHeader(Layer::L4).Fields(ack(965), flags(TH_ACK))
	    .WithHeader(Layer::PAYLOAD).Fields(payload("abcd", 100));

	// Generate a pure ACK with a larger th_ack field
	auto pkt2 = pkt1.Next()
	    .WithHeader(Layer::PAYLOAD).Fields(payload())
	    .WithHeader(Layer::L4).Fields(incrAck(2889));

	// The merged packet will be identical to the original data packet
	// but will have the larger th_ack field from the pure ACK.
	auto expected = pkt1
	    .WithHeader(Layer::L4).Fields(incrAck(2889));

	EXPECT_CALL(*this->mockIfp, if_input(PacketMatcher(expected)))
	    .Times(1);

	// getmicrotime() is not called on a pure ACK
	MockTime::ExpectGetMicrotime({.tv_sec = 89952, .tv_usec = 17935});

	// Begin the testcase.

	// Send the two frames in sequence to tcp_lro_rx().  Test the return
	// value from each call.
	int ret = tcp_lro_rx(&this->lc, pkt1.Generate(), 0);
	ASSERT_EQ(ret, 0);

	ret = tcp_lro_rx(&this->lc, pkt2.Generate(), 0);
	ASSERT_EQ(ret, 0);

	tcp_lro_flush_all(&this->lc);
}

// Send a pure ACK followed by a duplicate ACK.  Confirm that the original
// ACK is sent up the second and the dup ACK is rejected by LRO (which would
// force a real Ethernet driver to send the dup ACK up the stack immediately).
TYPED_TEST(TcpLroTestSuite, TestDupAck)
{
	auto origAck = this->GetTcpTemplate()
	    .WithHeader(Layer::L4).Fields(flags(TH_ACK), ack(89772));

	// Generate a dup ACK.  This should be a replica of the original
	// ACK but with the IP id incremented.
	auto dupAck = origAck.Next();

	this->TestRejectSecond(origAck, dupAck);
}

// Test the reception of out-of-order packets where the TCP sequence
// number is observed to go backwards.  Verify that the queued packet
// is immediately flushed up the stack while the second, out-of-order
// packet is rejected by LRO.
TYPED_TEST(TcpLroTestSuite, TestOoOBackwards)
{
	// This is the second packet that will be sent to LRO
	auto pkt2 = this->GetPayloadTemplate()
	    .WithHeader(Layer::PAYLOAD).Fields(payload("FreeBSD", 25));

	// This is the first packet that will be sent to LRO.
	auto pkt1 = pkt2.Next();

	this->TestRejectSecond(pkt1, pkt2);
}

// Test the reception of out-of-order packets where the TCP sequence
// number is observed to jump forward (due a packet being missing).
// Verify that the queued packet is immediately flushed up the stack
// while the second, out-of-order packet is rejected by LRO.
TYPED_TEST(TcpLroTestSuite, TestOoOSkipSeq)
{
	auto pkt1 = this->GetPayloadTemplate()
	    .WithHeader(Layer::PAYLOAD).Fields(payload("FreeBSD", 25));

	// We skip over one packet in the sequence by calling Next() twice.
	auto pkt2 = pkt1.Next().Next();

	this->TestRejectSecond(pkt1, pkt2);
}

// Test the reception of out-of-order packets where LRO sees the same sequence
// number twice.  Verify that the queued packet is immediately flushed up the
// stack while the second, out-of-order packet is rejected by LRO.
TYPED_TEST(TcpLroTestSuite, TestOoODupSeq)
{
	auto pkt1 = this->GetPayloadTemplate()
	    .WithHeader(Layer::PAYLOAD).Fields(payload("FreeBSD", 25));

	// Create a duplicate packet with the ip id incremented.
	auto pkt2 = pkt1.Retransmission();

	this->TestRejectSecond(pkt1, pkt2);
}

// Test the reception of out-of-order packets where LRO sees the a frame with
// a sequence number in the middle of the previous frame.  Verify that the
// queued packet is immediately flushed up the stack while the second, out-of-
// order packet is rejected by LRO.
TYPED_TEST(TcpLroTestSuite, TestOoOSeqInPrev)
{
	auto pkt1 = this->GetPayloadTemplate()
	    .WithHeader(Layer::PAYLOAD).Fields(payload("FreeBSD", 25));

	// Create a packet with a sequence number in the middle of the
	// range of the previous frame.
	auto pkt2 = pkt1.Next().WithHeader(Layer::L4).Fields(incrSeq(-1));

	this->TestRejectSecond(pkt1, pkt2);
}

// Test the reception of out-of-order packets where LRO sees the a frame with
// a sequence number that is one larger than the next in sequence.  Verify that the
// queued packet is immediately flushed up the stack while the second, out-of-
// order packet is rejected by LRO.
TYPED_TEST(TcpLroTestSuite, TestOoOSeqOffByOne)
{
	auto pkt1 = this->GetPayloadTemplate()
	    .WithHeader(Layer::PAYLOAD).Fields(payload("FreeBSD", 25));

	// Create a packet with a sequence number one past the end of
	// the previous frame.
	auto pkt2 = pkt1.Next().WithHeader(Layer::L4).Fields(incrSeq(+1));

	this->TestRejectSecond(pkt1, pkt2);
}

TYPED_TEST(TcpLroTestSuite, TestBadIpVersion)
{
	auto pkt1 = this->GetPayloadTemplate()
	    .WithHeader(Layer::PAYLOAD).Fields(payload("FreeBSD", 25));

	auto pkt2 = pkt1.Next().WithHeader(Layer::L3).Fields(ipVersion(5));

	this->TestRejectSecond(pkt1, pkt2, TCP_LRO_NOT_SUPPORTED);

	tcp_lro_flush_all(&this->lc);
}

template <typename NetworkLayerTemplate>
template <typename PktTemplate>
void
TcpLroTestSuite<NetworkLayerTemplate>::TestRejectSecond(const PktTemplate & pkt1, const PktTemplate & pkt2, int failCode)
{
	// The first packet should be flushed by LRO immediately when
	// it sees the second invalid packet.
	EXPECT_CALL(*this->mockIfp, if_input(PacketMatcher(pkt1)))
	    .Times(1);

	MockTime::ExpectGetMicrotime({.tv_sec = 5489, .tv_usec = 25847});

	// Begin the testcase.

	// Send the two frames in sequence to tcp_lro_rx().  Test the return
	// value from each call.
	int ret = tcp_lro_rx(&this->lc, pkt1.Generate(), 0);
	ASSERT_EQ(ret, 0);

	struct mbuf * m = pkt2.Generate();
	ret = tcp_lro_rx(&this->lc, m, 0);
	ASSERT_EQ(ret, failCode);
	m_freem(m);
}
