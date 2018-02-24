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
#define _KERNEL_UT_NO_USERLAND_CONFLICTS 1

#include "pktgen/TcpExpectation.h"

#include "pktgen/TcpFlow.h"

#include <gtest/gtest.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

namespace PktGen
{
	TcpExpectation::TcpExpectation(const TcpFlow &flow)
	  : th_sport(flow.GetSrcPort()),
	    th_dport(flow.GetDstPort()),
	    th_seq(flow.GetLastSeq()),
	    th_ack(flow.GetLastAck()),
	    th_off(flow.GetHeaderLen() / sizeof(uint32_t)),
	    th_x2(0),
	    th_flags(flow.GetDefaultFlags()),
	    th_win(flow.GetWindow()),
	    th_sum(0),
	    th_urp(0)
	{
	}

	#define	CheckField(th, field) \
		ASSERT_EQ(ntoh((th)->field), field)

	void TcpExpectation::TestExpectations(mbuf *m) const
	{
		auto * tcp = mtod(m, struct tcphdr*);

		CheckField(tcp, th_sport);
		CheckField(tcp, th_dport);
		CheckField(tcp, th_seq);
		CheckField(tcp, th_ack);
		CheckField(tcp, th_off);
		CheckField(tcp, th_x2);
		CheckField(tcp, th_flags);
		CheckField(tcp, th_win);
		CheckField(tcp, th_sum);
		CheckField(tcp, th_urp);
	}

	size_t TcpExpectation::GetHeaderLen(mbuf *m) const
	{
		return th_off * sizeof(uint32_t);
	}
}