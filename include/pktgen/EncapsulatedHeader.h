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
	auto MakeEncapsulation(const Lower & low, const Upper & up);

	template <typename Lower, typename Upper>
	class EncapsulatedHeader
	{
	private:
		Lower lower;
		Upper upper;

		typedef EncapsulatedHeader<Lower, Upper> SelfType;

	public:
		typedef typename Upper::LAYER LAYER;
		typedef typename Upper::NESTING_LEVEL NESTING_LEVEL;
		typedef typename Upper::EncapFieldSetter EncapFieldSetter;

		EncapsulatedHeader(const Lower & low, const Upper & up)
			: lower(low), upper(up)
		{
		}

		template <typename Generator>
		SelfType ApplyGenerator(Generator & gen) const
		{
			auto newUpper(upper.ApplyGenerator(gen));
			auto newLower(lower.ApplyGenerator(gen));
			return SelfType(newLower, newUpper);
		}

		template <typename Field>
		static typename std::enable_if<!std::is_void<typename Field::DownwardFieldGenerator>::value, SelfType>::type
		Apply(Field f, SelfType u)
		{
			typename Field::DownwardFieldGenerator ret;
			Upper newUpper(Upper::Apply(f, u.upper, ret));
			return SelfType(u.lower.ApplyGenerator(ret), newUpper);
		}

		template <typename Field>
		static typename std::enable_if<std::is_void<typename std::invoke_result<Field, typename Upper::UnderlyingHeader &>::type>::value, SelfType>::type
		Apply(Field f, SelfType u)
		{
			SelfType header (u.lower, u.upper.With(f));
			return header;
		}

		SelfType With() const
		{
			return *this;
		}

		template <typename Field, typename... Args>
		SelfType With(Field f, Args... args) const
		{
			return SelfType::Apply(f, With(args...));
		}

		template <LayerVal layer, int Nesting, typename ... Fields>
		typename std::enable_if<std::is_same<LayerImpl<layer, Nesting>, LAYER>::value, SelfType>::type
		WithHeaderFieldsImpl(Fields... f) const
		{
			return With(f...);
		}

		template <LayerVal layer, int Nesting, typename ... Fields>
		typename std::enable_if<!std::is_same<LayerImpl<layer, Nesting>, LAYER>::value, SelfType>::type
		WithHeaderFieldsImpl(Fields... f) const
		{
			auto newLower(lower.template WithHeaderFieldsImpl<layer, Nesting>(f...));

			return SelfType(newLower, upper);
		}

		template <typename Layer, typename ... Fields>
		SelfType WithHeaderFields(Fields... f) const
		{
			constexpr int nestDepth = NESTING_LEVEL::template ConvertInnerDepth<Layer>();

			return WithHeaderFieldsImpl<Layer::LAYER, nestDepth>(f...);
		}

		mbuf *Generate() const
		{
			size_t parentOff;
			return Generate(parentOff);
		}

		mbuf *Generate(size_t & offset) const
		{
			mbuf * m = lower.Generate(offset);
			upper.FillPacket(m, offset);

			return m;
		}

		void FillPacket(mbuf * m, size_t & offset) const
		{
			lower.FillPacket(m, offset);
			upper.FillPacket(m, offset);
		}

		size_t GetLen() const
		{
			return lower.GetLen() + upper.GetLen();
		}

		// We reformat the parse tree to ensure that the template is
		// always in canonical form, with the top level template of
		// type EncapsulatedHeader<EncapsulatedHeader<...>, UpperMostProtocol>,
		// rather than EncapsulatedHeader<LowestProtocol, EncapsulatedHeader<...>>
		// The algorithm in WithHeaderFields() depends on this.
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

		const auto & GetLower() const
		{
			return lower;
		}

		const auto & GetUpper() const
		{
			return upper;
		}

		SelfType Next() const
		{
			return SelfType(lower.Next(), upper.Next());
		}

		void print(int depth) const
		{
			std::string innerName(Lower::LAYER::Name());
			std::string outerName(Upper::LAYER::Name());
			PrintIndent(depth, "Encapped %s/%s : {", innerName.c_str(), outerName.c_str());
			lower.print(depth + 1);
			upper.print(depth + 1);
			PrintIndent(depth, "}");
		}
	};

	template <typename Lower, typename Upper>
	auto MakeEncapsulation(const Lower & low, const Upper & up)
	{
		auto newUpper = Upper::template MakeNested<typename Lower::NESTING_LEVEL>(up);
		typename decltype(newUpper)::EncapFieldSetter setter;
		auto newLower = setter(low, newUpper);
		return EncapsulatedHeader(newLower, newUpper);
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
		return testing::MakeMatcher(MakePacketMatcher(UnwrapTemplate(h)));
	}
}

#endif