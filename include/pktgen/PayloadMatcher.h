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

#include "fake/mbuf.h"
#include <gmock/gmock-matchers.h>

namespace PktGen
{
	class PayloadMatcher : public testing::MatcherInterface<mbuf*>
	{
	private:
		const char *pattern;
		size_t len;
		size_t headerOffset;

		bool TestNullPattern(mbuf *m, size_t off, size_t mbufIndex, size_t payloadIndex,
		    size_t & remaining, testing::MatchResultListener *) const;

		bool TestPattern(mbuf *m, size_t hdroff, size_t mbufIndex, size_t payloadIndex,
		    size_t & patOff, size_t & remaining, testing::MatchResultListener *) const;

	public:
		PayloadMatcher(const char * pattern, size_t len, size_t offset);
		PayloadMatcher(size_t len, size_t offset);

		virtual bool MatchAndExplain(mbuf*,
                    testing::MatchResultListener* listener) const override;

		virtual void DescribeTo(::std::ostream* os) const override;
	};

	template <typename... Args>
	inline testing::Matcher<mbuf*> Payload(Args... args)
	{
		return testing::MakeMatcher(new PayloadMatcher(args...));
	}
}

#endif
