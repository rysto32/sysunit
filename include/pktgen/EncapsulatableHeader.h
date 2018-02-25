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

namespace PktGen
{
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

		static const auto LAYER = Header::LAYER;
		typedef typename Header::EncapFieldSetter EncapFieldSetter;

		auto With() const
		{
			return *this;
		}

		template <typename Field>
		SelfType Apply(Field f, SelfType h) const
		{
			f(h.header);
			return EncapsulatableHeader<Header>(h);
		}

		template <typename Field, typename... Args>
		SelfType With(Field f, Args... a) const
		{
			return Apply(f, With(a...));
		}

		template <Layer layer, typename ... Fields>
		typename std::enable_if<layer == LAYER, SelfType>::type
		WithHeaders(Fields... f) const
		{
			return With(f...);
		}

		template <typename Lower>
		EncapsulatedHeader<Lower, SelfType> EncapIn(Lower lower) const
		{
			return MakeEncapsulation(lower, *this);
		}

		template <typename Upper>
		auto Encapsulate(Upper upper) const
		{
			return upper.EncapIn(*this);
		}

		template <typename Lower>
		friend auto operator|(SelfType lhs, Lower rhs)
		{
			return lhs.EncapIn(rhs);
		}

		mbuf *Generate() const
		{
			size_t parentOff;
			return Generate(parentOff, 0);
		}

		mbuf *Generate(size_t & offset, size_t parentLen) const
		{
			size_t mb_len = GetLen() + parentLen;
			mbuf * m = alloc_mbuf(mb_len);
			m->m_pkthdr.len = mb_len;
			m->m_len = mb_len;

			offset = 0;
			FillPacket(m, parentLen, offset);
			return m;
		}

		void FillPacket(mbuf * m, size_t parentLen, size_t & offset) const
		{
			header.FillPacket(m, parentLen, offset);
		}

		size_t GetLen() const
		{
			return header.GetLen();
		}

		const auto & GetHeader() const
		{
			return header;
		}

		void print(int depth)
		{
			PrintIndent(depth, "Encapable %s : {", LayerStr(LAYER));
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