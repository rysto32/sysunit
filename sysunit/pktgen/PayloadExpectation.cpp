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

#include "pktgen/PayloadExpectation.h"

#include <algorithm>

namespace PktGen
{
	PayloadExpectation::PayloadExpectation(const char * pattern, size_t len)
	  : pattern(pattern),
	    len(len)
	{
	}

	PayloadExpectation::PayloadExpectation(size_t len)
	  : pattern(nullptr),
	    len(len)
	{
	}

	void PayloadExpectation::TestNullPattern(mbuf* m, size_t & remaining) const
	{
		auto * ptr = mtod(m, uint8_t*);
		size_t i;

		for (i = 0; i < m->m_len && i < remaining; ++i) {
			EXPECT_EQ(ptr[i], 0);
		}

		remaining -= i;
	}

	void PayloadExpectation::TestPattern(mbuf *m, size_t & offset, size_t & remaining) const
	{
		auto * ptr = mtod(m, uint8_t*);
		auto patlen = strlen(pattern);
		size_t left, last_offset;

		left = std::min(static_cast<size_t>(m->m_len), remaining);
		remaining -= left;

		last_offset = offset;
		while (left > 0) {
			auto iterlen = patlen - last_offset;
			auto cmplen = std::min(left, iterlen);

			ASSERT_EQ(memcmp(ptr, pattern + last_offset, cmplen), 0);
			ptr += cmplen;
			left -= cmplen;
			last_offset = cmplen;
		}
		offset = last_offset;
	}

	void PayloadExpectation::TestExpectations(mbuf *m) const
	{
		ASSERT_GE(m->m_pkthdr.len, len);

		if (pattern == nullptr) {
			auto remaining = len;
			while (m != nullptr && remaining > 0) {
				PayloadExpectation::TestNullPattern(m, remaining);
				m = m->m_next;
			}

			ASSERT_EQ(remaining, 0);
		} else {
			auto remaining = len;
			size_t offset = 0;

			while (m != nullptr && remaining > 0) {
				TestPattern(m, offset, remaining);
				m = m->m_next;
			}

			ASSERT_EQ(remaining, 0);
		}
	}

	size_t PayloadExpectation::GetHeaderLen(mbuf *m) const
	{
		return len;
	}
}