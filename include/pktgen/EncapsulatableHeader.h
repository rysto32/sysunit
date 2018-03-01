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

#ifndef PKTGEN_ENCAPSULATABLE_HEADER_H
#define PKTGEN_ENCAPSULATABLE_HEADER_H

#include "pktgen/EncapsulatedHeader.h"
#include "pktgen/PayloadLength.h"

namespace PktGen
{
	struct DefaultEncapFieldSetter
	{
		template <typename Lower, typename Upper>
		Lower operator()(const Lower & l, const Upper & u) const
		{
			return l.With(
			    PayloadLengthField(u.GetLen() + u.GetPayloadLength())
			);
		}
	};

	template <typename Header>
	class EncapsulatableHeader
	{
	private:
		Header header;

		typedef EncapsulatableHeader<Header> SelfType;

	public:
		EncapsulatableHeader() = default;

		EncapsulatableHeader(const Header &h)
		: header(h)
		{
		}

		typedef typename Header::NESTING_LEVEL NESTING_LEVEL;
		typedef typename Header::LAYER LAYER;
		typedef Header UnderlyingHeader;

		struct EncapFieldSetter
		{
			template <typename Lower>
			Lower operator()(const Lower & l, const SelfType & t) const
			{
				typename Header::EncapFieldSetter setter;
				return setter(l, t.header);
			}
		};

		auto With() const
		{
			return *this;
		}

		template <typename Generator>
		SelfType ApplyGenerator(Generator & gen) const
		{
			return SelfType(gen.Apply(header));
		}

		template <typename Field>
		static SelfType Apply(Field f, SelfType h, typename Field::ApplyReturn & ret)
		{
			ret = f(h.header);
			return EncapsulatableHeader<Header>(h);
		}

		template <typename Field>
		static SelfType Apply(Field f, SelfType h)
		{
			f(h.header);
			return EncapsulatableHeader<Header>(h);
		}

		template <typename Field, typename... Args>
		SelfType With(Field f, Args... a) const
		{
			return Apply(f, With(a...));
		}

		template <LayerVal layer, int Nesting, typename ... Fields>
		typename std::enable_if<std::is_same<LayerImpl<layer, Nesting>, LAYER>::value, SelfType>::type
		WithHeaderFieldsImpl(Fields... f) const
		{
			return With(f...);
		}

		template <typename Layer, typename ... Fields>
		SelfType WithHeaderFields(Fields... f) const
		{
			constexpr int nestDepth = NESTING_LEVEL::template ConvertInnerDepth<Layer>();

			return WithHeaderFieldsImpl<Layer::IMPL::LAYER, nestDepth>(f...);
		}

		template <typename Lower>
		auto EncapIn(Lower lower) const
		{
			return MakeEncapsulation(lower, *this);
		}

		template <typename Upper>
		auto Encapsulate(Upper upper) const
		{
			return upper.EncapIn(*this);
		}

		mbuf *Generate() const
		{
			size_t parentOff;
			return Generate(parentOff);
		}

		mbuf *Generate(size_t & offset) const
		{
			size_t mb_len = header.GetLen() + header.GetPayloadLength();
			mbuf * m = alloc_mbuf(mb_len);
			m->m_pkthdr.len = mb_len;
			m->m_len = mb_len;

			offset = 0;
			FillPacket(m, offset);
			return m;
		}

		void FillPacket(mbuf * m, size_t & offset) const
		{
			header.FillPacket(m, offset);
		}

		size_t GetLen() const
		{
			return header.GetLen();
		}

		const auto & GetHeader() const
		{
			return header;
		}

		SelfType Next() const
		{
			return SelfType(header.Next());
		}

		SelfType Retransmission() const
		{
			return SelfType(header.Retransmission());
		}

		template <typename NestingLevel>
		static auto MakeNested(const SelfType & up)
		{
			auto newHeader(Header::template MakeNested<NestingLevel>(up.header));
			return EncapsulatableHeader<decltype(newHeader)>(newHeader);
		}

		void print(int depth) const
		{
			std::string headerName(LAYER::Name());
			PrintIndent(depth, "Encapable %s : {", headerName.c_str());
			header.print(depth + 1);
			PrintIndent(depth, "}");
		}
	};

	template <typename Header>
	auto PacketMatcher(const EncapsulatableHeader<Header> & header, size_t hdrOff)
	{
		return PacketMatcher(header.GetHeader(), hdrOff);
	}
}

#endif
