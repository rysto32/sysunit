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

#include "pktgen/PacketPayload.h"

#include "pktgen/Packet.h"

#include <algorithm>

using testing::MatchResultListener;

namespace PktGen::internal
{
	PayloadMatcher::PayloadMatcher(const PayloadTemplate & p, size_t off)
	  : payload(std::move(p)),
	    headerOffset(off)
	{
	}

	bool PayloadMatcher::TestPattern(mbuf *m, int hdroff, size_t mbufNumber,
	    size_t & payloadIndex, const std::vector<uint8_t> & payloadBytes,
	    size_t payloadLen, MatchResultListener* listener) const
	{
		auto * ptr = GetMbufHeader<uint8_t>(m, 0);

		int mbIndex = hdroff;
		size_t payIndex = payloadIndex;

		while (mbIndex < m->m_len && payIndex < payloadLen) {
			if (payloadBytes.at(payIndex) != ptr[mbIndex]) {
				*listener << "Payload incorrect at mbuf " <<
				    mbufNumber << " index " << mbIndex <<
				    " (payload index " << payIndex << ")" <<
				    " value " << std::hex << int(ptr[mbIndex]);
				if (std::isprint(ptr[mbIndex]))
					*listener << "('" << (char)ptr[mbIndex] << "')";

				*listener << " (expected " << int(payloadBytes.at(payIndex));
				if (std::isprint(payloadBytes.at(payIndex)))
					*listener <<  "('" << (char)payloadBytes.at(payIndex) << "'))";
				return false;
			}

			++mbIndex;
			++payIndex;
		}

		payloadIndex = payIndex;
		return true;
	}

	bool PayloadMatcher::MatchAndExplain(mbuf* m, MatchResultListener* listener) const
	{
		const auto & payloadBytes = payload.GetPayload();

		int hdroff = headerOffset;
		size_t mbufNumber = 0;
		size_t payloadIndex = payload.GetStartIndex();
		size_t payloadLen = payload.GetFillLen();

		if ((m->m_pkthdr.len - headerOffset) < payloadLen) {
			*listener << "payload len is " << m->m_pkthdr.len - headerOffset
			    << " (expected " << payloadLen << ")";
			return false;
		}

		while (m != nullptr && payloadIndex < payloadBytes.size()) {
			bool matched = TestPattern(m, hdroff, mbufNumber, payloadIndex,
				payloadBytes, payloadLen, listener);
			if (!matched)
				return false;

			mbufNumber++;
			hdroff = 0;
			m = m->m_next;
		}

		return true;
	}

	void PayloadMatcher::DescribeTo(::std::ostream* os) const
	{
		*os << "Payload";
	}
}