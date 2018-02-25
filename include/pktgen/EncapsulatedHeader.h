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

#ifndef PKTGEN_ENCAPSULATED_HEADER
#define PKTGEN_ENCAPSULATED_HEADER

#include "pktgen/Layer.h"
#include "pktgen/Packet.h"

#include <gmock/gmock-matchers.h>

struct mbuf;

namespace PktGen
{
	template <typename Lower, typename Upper>
	class EncapsulatedHeader;

	template <typename Lower, typename Upper>
	EncapsulatedHeader<Lower, Upper> MakeEncapsulation(const Lower & low, const Upper & up);

	template <typename Lower, typename Upper>
	class EncapsulatedHeader
	{
	private:
		Lower lower;
		Upper upper;

		typedef EncapsulatedHeader<Lower, Upper> SelfType;
	public:
		static const auto LAYER = Upper::LAYER;
		typedef typename Upper::EncapFieldSetter EncapFieldSetter;

		EncapsulatedHeader(const Lower & low, const Upper & up)
			: lower(low), upper(up)
		{
		}

		template <typename... Fields>
		SelfType With(Fields... f) const
		{
			return SelfType(lower, upper.With(f...));
		}

		template <Layer layer, typename ... Fields>
		typename std::enable_if<layer == LAYER, SelfType>::type
		WithHeaders(Fields... f) const
		{
			return With(f...);
		}

		template <Layer layer, typename ... Fields>
		typename std::enable_if<layer != LAYER, SelfType>::type
		WithHeaders(Fields... f) const
		{
			auto newLower(lower.template WithHeaders<layer>(f...));

			return SelfType(newLower, upper);
		}

		mbuf *Generate() const
		{
			size_t parentOff;
			return Generate(parentOff, 0);
		}

		mbuf *Generate(size_t & offset, size_t parentLen) const
		{
			mbuf * m = lower.Generate(offset, parentLen + upper.GetLen());
			upper.FillPacket(m, parentLen, offset);

			return m;
		}

		void FillPacket(mbuf * m, size_t parentLen, size_t & offset) const
		{
			lower.FillPacket(m, parentLen + upper.GetLen(), offset);
			upper.FillPacket(m, parentLen, offset);
		}

		size_t GetLen() const
		{
			return lower.GetLen() + upper.GetLen();
		}

		// We reformat the parse tree to ensure that the template is
		// always in canonical form, with the top level template of
		// type EncapsulatedHeader<EncapsulatedHeader<...>, UpperMostProtocol>,
		// rather than EncapsulatedHeader<LowestProtocol, EncapsulatedHeader<...>>
		// The algorithm in WithHeaders() depends on this.
		template <typename Deepest>
		auto EncapIn(Deepest origDeepest) const
		{
			return MakeEncapsulation(lower.EncapIn(origDeepest), upper);
		}

		template <typename Upmost>
		auto  Encapsulate(Upmost upmost) const
		{
			return MakeEncapsulation(*this, upmost);
		}

		template <typename Deepest>
		friend auto operator|(SelfType lhs, Deepest rhs)
		{
			return lhs.EncapIn(rhs);
		}

		void print(int depth)
		{
			PrintIndent(depth, "Encapped %s/%s : {", LayerStr(Upper::LAYER), LayerStr(Lower::LAYER));
			upper.print(depth + 1);
			lower.print(depth + 1);
			PrintIndent(depth, "}");
		}

		const auto & GetLower() const
		{
			return lower;
		}

		const auto & GetUpper() const
		{
			return upper;
		}
	};

	template <typename Lower, typename Upper>
	EncapsulatedHeader<Lower, Upper> MakeEncapsulation(const Lower & low, const Upper & up)
	{
		typename Upper::EncapFieldSetter setter;
		return EncapsulatedHeader<Lower, Upper>(low.With(setter), up);
	}

	template <typename LowerMatcher, typename UpperMatcher>
	class EncapMatcher : public testing::MatcherInterface<mbuf*>
	{
		LowerMatcher lowMatch;
		UpperMatcher upperMatch;

	public:
		EncapMatcher(LowerMatcher l, UpperMatcher u)
		: lowMatch(l), upperMatch(u)
		{}

		virtual bool MatchAndExplain(mbuf* m,
                    testing::MatchResultListener* listener) const override
		{
			bool matched = lowMatch.MatchAndExplain(m, listener);
			if (!matched)
				return false;

			return upperMatch.MatchAndExplain(m, listener);
		}

		virtual void DescribeTo(::std::ostream* os) const override
		{
			lowMatch.DescribeTo(os);
			*os << '/';
			upperMatch.DescribeTo(os);
		}
	};

	template <typename Lower, typename Upper>
	auto PacketMatcher(const EncapsulatedHeader<Lower, Upper> & pktTemplate, size_t hdrOff)
	{
		return EncapMatcher(PacketMatcher(pktTemplate.GetLower(), hdrOff),
				PacketMatcher(pktTemplate.GetUpper(), hdrOff + pktTemplate.GetLower().GetLen()));
	}

	template <typename Lower, typename Upper>
	auto MakePacketMatcher(const EncapsulatedHeader<Lower, Upper> & header)
	{
		return new EncapMatcher(PacketMatcher(header.GetLower(), 0),
				PacketMatcher(header.GetUpper(), 0 + header.GetLower().GetLen()));
	}

	template <typename Header>
	auto PacketMatcher(const Header & h)
	{
		return testing::MakeMatcher(MakePacketMatcher(h));
	}
}

#endif