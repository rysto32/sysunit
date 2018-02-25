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

#include "fake/mbuf.h"

#include "pktgen/TcpMatcher.h"
#include "pktgen/TcpHeader.h"

#include <gtest/gtest.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

namespace PktGen
{
	TcpMatcher::TcpMatcher(const TcpTemplate &header, size_t offset)
	  : header(header),
	    headerOffset(offset)
	{
	}

	#define	CheckField(th, field, expect) do { \
		if (ntoh((th)->field) != (expect)) { \
			*listener << "TCP: " << #field << " field is " << ntoh((th)->field) \
			    << " (expected " << (expect) << ")"; \
			return false; \
		} \
	} while (0)

	bool TcpMatcher::MatchAndExplain(mbuf* m,
                    testing::MatchResultListener* listener) const
	{
		auto * tcp = GetMbufHeader<tcphdr>(m, headerOffset);

		CheckField(tcp, th_sport, header.GetSrcPort());
		CheckField(tcp, th_dport, header.GetDstPort());
		CheckField(tcp, th_seq, header.GetSeq());
		CheckField(tcp, th_ack, header.GetAck());
		CheckField(tcp, th_off, header.GetOff());
		CheckField(tcp, th_x2, header.GetX2());
		CheckField(tcp, th_flags, header.GetFlags());
		CheckField(tcp, th_win, header.GetWindow());

		// XXX tcp_lro always updates this...
		if (0)
			CheckField(tcp, th_sum, header.GetChecksum());

		CheckField(tcp, th_urp, header.GetUrgentPointer());

		uint32_t expectedFlag = 0;
		if (header.GetChecksumVerified())
			expectedFlag = CSUM_L4_CALC;

		auto actualFlag = m->m_pkthdr.csum_flags & CSUM_L4_CALC;
		if (actualFlag != expectedFlag) {
			*listener << "TCP: csum calc flag is " << actualFlag
			    << " (expected " << expectedFlag << ")";
			return false;
		}

		expectedFlag = 0;
		if (header.GetChecksumPassed())
			expectedFlag = CSUM_L4_VALID;

		actualFlag = m->m_pkthdr.csum_flags & CSUM_L4_VALID;
		if (actualFlag != expectedFlag) {
			*listener << "TCP: csum valid flag is " << actualFlag
			    << " (expected " << expectedFlag << ")";
			return false;
		}

		return true;
	}

	void TcpMatcher::DescribeTo(::std::ostream* os) const
	{
		*os << "TCP";
	}
}