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

#ifndef PKTGEN_TCP_EXPECTATION_H
#define PKTGEN_TCP_EXPECTATION_H

#include "fake/mbuf.h"
#include <gmock/gmock-matchers.h>

namespace PktGen
{
	class TcpFlow;

	class TcpMatcher : public testing::MatcherInterface<mbuf*>
	{
	private:
		size_t headerOffset;
		uint16_t th_sport;
		uint16_t th_dport;
		uint32_t th_seq;
		uint32_t th_ack;
		uint8_t th_off;
		uint8_t th_x2;
		uint8_t th_flags;
		uint16_t th_win;
		uint16_t th_sum;
		uint16_t th_urp;

	public:
		TcpMatcher(const TcpFlow &, size_t offset);

		virtual bool MatchAndExplain(mbuf*,
                    testing::MatchResultListener* listener) const override;

		virtual void DescribeTo(::std::ostream* os) const override;
	};

	inline testing::Matcher<mbuf*> TcpHeader(const TcpFlow &flow, size_t offset)
	{
		return testing::MakeMatcher(new TcpMatcher(flow, offset));
	}
}

#endif
