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

#ifndef PKTGEN_PACKET_MATCHER_H
#define PKTGEN_PACKET_MATCHER_H

#include "fake/mbuf.h"

#include "pktgen/Packet.h"

#include <gmock/gmock.h>

#include <tuple>

namespace PktGen
{
	template <typename... Matchers>
	class PacketTemplateMatcher : public testing::MatcherInterface<mbuf *>
	{
	private:
		std::tuple<Matchers...> matchers;

		template <typename Matcher>
		static void DescribeMatcher(std::ostream* os, const char *sep, const Matcher & matcher)
		{
			*os << sep;
			matcher.DescribeTo(os);
		}

	public:
		PacketTemplateMatcher(const std::tuple<Matchers...> & m)
		  : matchers(m)
		{
		}

		virtual bool MatchAndExplain(mbuf* m,
                    testing::MatchResultListener* listener) const override
		{
			return std::apply([m, listener] (const auto &... matcher)
				{
					return ((matcher.MatchAndExplain(m, listener)) && ...);
				}, matchers);
		}

		virtual void DescribeTo(std::ostream* os) const override
		{
			std::apply([os] (const auto &... matcher)
				{
					const char * sep = "";

					((DescribeMatcher(os, sep, matcher), sep = "/"), ...);
				}, matchers);
		}
	};

	auto MakePacketMatcherTupleImpl(size_t off)
	{
		return std::tuple<>();
	}

	template <typename First, typename... Rest>
	auto MakePacketMatcherTupleImpl(size_t off, const First & first, const Rest &... rest)
	{
		auto firstTuple = std::make_tuple(PacketMatcher(first, off));
		off += first.GetLen();

		return std::tuple_cat(firstTuple, MakePacketMatcherTupleImpl(off, rest...));
	}

	template <typename... Headers>
	auto MakePacketMatcherTuple(const Headers &... headers)
	{
		return MakePacketMatcherTupleImpl(0, headers...);
	}

	template <typename... Headers>
	auto PacketMatcher(const PacketTemplateWrapper<Headers...> wrapper)
	{
		return std::apply( [] (const auto &... header)
			{
				return testing::MakeMatcher(
				    new PacketTemplateMatcher(MakePacketMatcherTuple(header...)));
			}, wrapper.Unwrap());
	}
}

#endif