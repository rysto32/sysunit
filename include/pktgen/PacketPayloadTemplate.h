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

#include <stdint.h>
#include <vector>

namespace PktGen
{
	class PayloadTemplate
	{
	public:
		typedef std::vector<uint8_t> PayloadVector;

	private:
		PayloadVector payload;

	public:
		PayloadTemplate() = default;

		static const auto LAYER = Layer::PAYLOAD;

		void SetPayload(const PayloadVector & p)
		{
			payload = p;
		}

		void FillPacket(mbuf * m, size_t parentLen, size_t & offset) const
		{
			auto * pl = GetMbufHeader<uint8_t>(m, offset);

			memcpy(pl, &payload[0], GetLen());

			offset += GetLen();
		}

		size_t GetLen() const
		{
			return payload.size();
		}

		// This is to appease EncapsulatableHeader
		struct EncapFieldSetter
		{
			template <typename T>
			void operator()(T &)
			{
			}
		};

		const PayloadVector & GetPayload() const
		{
			return payload;
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
		return EncapsulatableHeader<PayloadTemplate>();
	}

	auto inline payload(const PayloadTemplate::PayloadVector & p)
	{
		return [p] (auto & h) { h.SetPayload(p); };
	}

	auto inline payload(PayloadTemplate::PayloadVector && p)
	{
		return [p = std::move(p)] (auto & h) { h.SetPayload(p); };
	}

	auto inline payload(uint8_t byte, size_t count = 1)
	{
		PayloadTemplate::PayloadVector p(count, byte);
		return [p = std::move(p)] (auto & h) { h.SetPayload(p); };
	}

	auto inline payload(const std::string & str, size_t count)
	{
		PayloadTemplate::PayloadVector p;

		while (count > 0) {
			for (auto ch : str) {
				p.push_back(ch);
				count--;
				if (count == 0)
					break;
			}
		}

		return [p = std::move(p)] (auto & h) { h.SetPayload(p); };
	}

	auto inline payload(const std::string & p)
	{
		return payload(p, p.size());
	}
}

#endif
