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

namespace PktGen
{
	template <typename Lower, typename Upper>
	class EncapsulatedHeader;

	template <typename Lower, typename Upper>
	EncapsulatedHeader<Lower, Upper> Encapsulate(const Lower & low, const Upper & up);

	template <typename Lower, typename Upper>
	class EncapsulatedHeader
	{
	private:
		Lower lower;
		Upper upper;

		typedef EncapsulatedHeader<Lower, Upper> SelfType;
	public:
		static const auto LAYER = Upper::LAYER;
		typedef typename Lower::EncapFieldSetter EncapFieldSetter;

		EncapsulatedHeader(const Lower & low, const Upper & up)
			: lower(low), upper(up)
		{
		}

		template <typename... Fields>
		SelfType With(Fields... f)
		{
			return SelfType(lower, upper.With(f...));
		}

		template <Layer layer, typename ... Fields>
		typename std::enable_if<layer == LAYER, SelfType>::type
		WithHeaders(Fields... f)
		{
			return With(f...);
		}

		template <Layer layer, typename ... Fields>
		typename std::enable_if<layer != LAYER, SelfType>::type
		WithHeaders(Fields... f)
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
		auto encapIn(Deepest origDeepest)
		{
			return Encapsulate(lower.encapIn(origDeepest), upper);
		}

		template <typename Deepest>
		friend auto operator|(SelfType lhs, Deepest rhs)
		{
			return lhs.encapIn(rhs);
		}

		void print(int depth)
		{
			PrintIndent(depth, "Encapped %s/%s : {", LayerStr(Upper::LAYER), LayerStr(Lower::LAYER));
			upper.print(depth + 1);
			lower.print(depth + 1);
			PrintIndent(depth, "}");
		}
	};

	template <typename Lower, typename Upper>
	EncapsulatedHeader<Lower, Upper> Encapsulate(const Lower & low, const Upper & up)
	{
		return EncapsulatedHeader<Lower, Upper>(low, up);
	}
}

#endif