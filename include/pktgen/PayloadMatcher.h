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

#ifndef PKTGEN_PAYLOAD_EXPECTATION_H
#define PKTGEN_PAYLOAD_EXPECTATION_H

#include <gmock/gmock-matchers.h>

#include "pktgen/PacketPayloadTemplate.h"

struct mbuf;

namespace PktGen::internal
{
	class PayloadMatcher : public testing::MatcherInterface<mbuf*>
	{
	private:
		PayloadTemplate payload;
		size_t headerOffset;

		bool TestPattern(mbuf *m, int hdroff, size_t mbufNumber,
		    size_t & payloadIndex, const std::vector<uint8_t> & payloadBytes,
		    size_t payloadLen, testing::MatchResultListener* listener) const;

	public:
		PayloadMatcher(const PayloadTemplate & p, size_t off);

		virtual bool MatchAndExplain(mbuf*,
                    testing::MatchResultListener* listener) const override;

		virtual void DescribeTo(::std::ostream* os) const override;
	};

	auto inline PacketMatcher(const PayloadTemplate & pkt, size_t off)
	{
		return PayloadMatcher(pkt, off);
	}
}

#endif
