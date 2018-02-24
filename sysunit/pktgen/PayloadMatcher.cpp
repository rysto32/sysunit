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

#include "pktgen/PayloadMatcher.h"

#include "pktgen/Packet.h"

#include <algorithm>

using testing::MatchResultListener;

namespace PktGen
{
	PayloadMatcher::PayloadMatcher(const char * pattern, size_t len, size_t offset)
	  : pattern(pattern),
	    len(len),
	    headerOffset(offset)
	{
	}

	PayloadMatcher::PayloadMatcher(size_t len, size_t offset)
	  : pattern(nullptr),
	    len(len),
	    headerOffset(offset)
	{
	}

	bool PayloadMatcher::TestNullPattern(mbuf* m, size_t off, size_t mbufIndex, size_t payloadIndex,
	    size_t & remaining, MatchResultListener* listener) const
	{
		auto * ptr = GetMbufHeader<uint8_t>(m, off);
		size_t i;

		for (i = 0; i < m->m_len && i < remaining; ++i) {
			if (ptr[i] != 0) {
				*listener << "payload at mbuf " << mbufIndex <<
				    " index " << off + i << " (payload index " << payloadIndex + i <<
				    ") is 0x" << std::hex << (u_int)ptr[i] << " (expected 0x" << 0 << ")";
				return false;
			}
		}

		remaining -= i;
		return true;
	}

	bool PayloadMatcher::TestPattern(mbuf *m, size_t hdroff, size_t mbufIndex, size_t payloadIndex,
	    size_t & patOff, size_t & remaining, MatchResultListener* listener) const
	{
		auto * ptr = GetMbufHeader<uint8_t>(m, 0);
		auto patlen = strlen(pattern);
		size_t left, last_offset;

		left = std::min(static_cast<size_t>(m->m_len - hdroff), remaining);
		remaining -= left;

		last_offset = patOff;
		while (left > 0) {
			auto iterlen = patlen - last_offset;
			auto cmplen = std::min(left, iterlen);

			size_t i = 0;
			for (i = 0; i < cmplen; ++i) {
				if (ptr[hdroff + i] != pattern[last_offset + i]) {
					*listener << "payload at mbuf " << mbufIndex <<
					    " index " << hdroff + i <<
					    " (payload index " << payloadIndex + i << ")" <<
					     " is 0x" << std::hex << (u_int)ptr[hdroff + i] <<
					     " (expected 0x" << (u_int)pattern[last_offset + i] << ")";
					return false;
				}
			}
			ptr += cmplen;
			left -= cmplen;
			last_offset = cmplen;
		}
		patOff = last_offset;
		return true;
	}

	bool PayloadMatcher::MatchAndExplain(mbuf* m, MatchResultListener* listener) const
	{
		if (m->m_pkthdr.len < len) {
			*listener << "packet len is " << m->m_pkthdr.len;
			return false;
		}

		if (pattern == nullptr) {
			auto remaining = len;
			auto offset = headerOffset;
			size_t mbufIndex = 0;
			size_t payloadIndex = 0;

			while (m != nullptr && remaining > 0) {
				bool matched = TestNullPattern(m, offset, mbufIndex, payloadIndex,
				    remaining, listener);
				if (!matched)
					return false;

				mbufIndex++;
				payloadIndex += m->m_len - offset;
				offset = 0;
				m = m->m_next;
			}

// 			ASSERT_EQ(remaining, 0);
		} else {
			auto remaining = len;
			size_t patOff = 0;
			auto hdroff = headerOffset;
			size_t mbufIndex = 0;
			size_t payloadIndex = 0;

			while (m != nullptr && remaining > 0) {
				bool matched = TestPattern(m, hdroff, mbufIndex, payloadIndex,
				    patOff, remaining, listener);
				if (!matched)
					return false;

				mbufIndex++;
				payloadIndex += m->m_len - hdroff;
				hdroff = 0;
				m = m->m_next;
			}

// 			ASSERT_EQ(remaining, 0);
		}

		return true;
	}

	void PayloadMatcher::DescribeTo(::std::ostream* os) const
	{
		*os << "payload";
	}
}