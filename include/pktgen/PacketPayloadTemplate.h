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

#ifndef PKTGEN_PACKET_PAYLOAD_TEMPLATE_H
#define PKTGEN_PACKET_PAYLOAD_TEMPLATE_H

#include "fake/mbuf.h"

#include "pktgen/EncapsulatableHeader.h"
#include "pktgen/Layer.h"
#include "pktgen/PacketTemplates.h"
#include "pktgen/PayloadLength.h"

#include <stdint.h>
#include <vector>

namespace PktGen
{
	typedef std::vector<uint8_t> PayloadVector;

	template <typename Nesting>
	class PayloadTemplate
	{
	private:
		PayloadVector payload;

	public:
		typedef typename Nesting::NextPayload NESTING_LEVEL;
		typedef typename NESTING_LEVEL::PAYLOAD LAYER;
		typedef PayloadTemplate<Nesting> SelfType;

		typedef DefaultEncapFieldSetter EncapFieldSetter;

		PayloadTemplate() = default;

		template <typename U>
		explicit PayloadTemplate(const PayloadTemplate<U> & h)
		  : payload(h.GetPayload())
		{
		}

		void SetPayload(const PayloadVector & p)
		{
			payload = p;
		}

		void AppendPayload(const PayloadVector & p)
		{
			for (auto ch : p)
				payload.push_back(ch);
		}

		void FillPacket(mbuf * m, size_t & offset) const
		{
			auto * pl = GetMbufHeader<uint8_t>(m, offset);

			memcpy(pl, &payload[0], GetLen());

			offset += GetLen();
		}

		size_t GetLen() const
		{
			return payload.size();
		}

		const PayloadVector & GetPayload() const
		{
			return payload;
		}

		PayloadTemplate Next() const
		{
			return *this;
		}

		PayloadTemplate Retransmission() const
		{
			return *this;
		}

		template <typename NestingLevel>
		static auto MakeNested(const SelfType & up)
		{
			return PayloadTemplate<NestingLevel>(up);
		}

		UnnestedPayloadTemplate StripNesting() const
		{
			return UnnestedPayloadTemplate(*this);
		}

		static PayloadVector FillPayload(const std::string & str, size_t count)
		{
			PayloadVector p;

			while (count > 0) {
				for (auto ch : str) {
					p.push_back(ch);
					count--;
					if (count == 0)
						break;
				}
			}

			return p;
		}

		void print(int depth)
		{
			PrintIndent(depth, "Payload = {");
			PrintIndent(depth + 1, "len = %zd", GetLen());
			PrintIndent(depth, "}");
		}
	};

	auto inline PacketPayload()
	{
		return EncapsulatableHeader(UnnestedPayloadTemplate());
	}

	auto inline payload(PayloadVector && p)
	{
		return PayloadField(std::move(p),
		    [](auto & h, const PayloadVector & p) { h.SetPayload(p); });
	}

	auto inline payload()
	{
		return payload(PayloadVector());
	}

	auto inline payload(uint8_t byte, size_t count = 1)
	{
		return payload(PayloadVector(count, byte));
	}

	auto inline payload(const std::string & str, size_t count)
	{
		return payload(UnnestedPayloadTemplate::FillPayload(str, count));
	}

	auto inline payload(const std::string & p)
	{
		return payload(p, p.size());
	}

	auto inline appendPayload(PayloadVector && p)
	{
		return PayloadField(std::move(p),
		    [](auto & h, const PayloadVector & p) { h.AppendPayload(p); });
	}

	auto inline appendPayload(uint8_t byte, size_t count = 1)
	{
		return appendPayload(PayloadVector(count, byte));
	}

	auto inline appendPayload(const std::string & str, size_t count)
	{
		return appendPayload(UnnestedPayloadTemplate::FillPayload(str, count));
	}

	auto inline appendPayload(const std::string & p)
	{
		return appendPayload(p, p.size());
	}
}

#endif
