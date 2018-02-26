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
	template <typename Header>
	class PayloadLengthSetter
	{
	public:
		void operator()(Header & h, size_t length) const
		{
			h.SetPayloadLength(length);
		}
	};

	class PayloadLengthGenerator
	{
	private:
		size_t length;
	public:
		PayloadLengthGenerator(size_t l = 0)
		  : length(l)
		{
		}

		template <typename Header>
		Header Apply(Header h)
		{
			PayloadLengthSetter<Header> setter;

			setter(h, length);

			length += h.GetLen();
			return h;
		}
	};

	template <typename Setter>
	class PayloadField
	{
		std::vector<uint8_t> payload;
		Setter setter;

	public:
		PayloadField(std::vector<uint8_t> && p, Setter s)
		  : payload(p),
		    setter(s)
		{
		}

		typedef PayloadLengthGenerator DownwardFieldGenerator;
		typedef DownwardFieldGenerator ApplyReturn;

		template <typename Header>
		ApplyReturn operator()(Header & h) const
		{
			setter(h, payload);
			return DownwardFieldGenerator(h.GetLen());
		}
	};

	class PayloadSizeField
	{
		size_t size;

	public:
		PayloadSizeField(size_t s)
		  : size(s)
		{
		}

		typedef PayloadLengthGenerator DownwardFieldGenerator;
		typedef DownwardFieldGenerator ApplyReturn;

		template <typename Header>
		ApplyReturn operator()(Header & h) const
		{
			h.SetPayloadLength(size);
			return DownwardFieldGenerator(h.GetLen() + size);
		}
	};

	class PayloadTemplate
	{
	public:
		typedef std::vector<uint8_t> PayloadVector;

	private:
		PayloadVector payload;

	public:
		static const auto LAYER = Layer::PAYLOAD;

		struct EncapFieldSetter
		{
			template <typename Header>
			Header operator()(const Header & h, const PayloadTemplate & t) const
			{
				return h.template WithHeaderFields<Layer::L4>(
				    PayloadSizeField(t.GetLen())
				);
			}
		};

		PayloadTemplate() = default;

		void SetPayload(const PayloadVector & p)
		{
			payload = p;
		}

		void AppendPayload(const PayloadVector & p)
		{
			for (auto ch : p)
				payload.push_back(ch);
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

		const PayloadVector & GetPayload() const
		{
			return payload;
		}

		PayloadTemplate Next() const
		{
			return *this;
		}

		void print(int depth)
		{
			PrintIndent(depth, "Payload = {");
			PrintIndent(depth + 1, "len = %zd", GetLen());
			PrintIndent(depth, "}");
		}

		static PayloadVector FillPayload(const std::string & str, size_t count)
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

			return p;
		}
	};

	auto inline PacketPayload()
	{
		return EncapsulatableHeader(PayloadTemplate());
	}

	auto inline payload(PayloadTemplate::PayloadVector && p)
	{
		return PayloadField(std::move(p),
		    [](auto & h, const PayloadTemplate::PayloadVector & p) { h.SetPayload(p); });
	}

	auto inline payload()
	{
		return payload(PayloadTemplate::PayloadVector());
	}

	auto inline payload(uint8_t byte, size_t count = 1)
	{
		return payload(PayloadTemplate::PayloadVector(count, byte));
	}

	auto inline payload(const std::string & str, size_t count)
	{
		return payload(PayloadTemplate::FillPayload(str, count));
	}

	auto inline payload(const std::string & p)
	{
		return payload(p, p.size());
	}

	auto inline appendPayload(PayloadTemplate::PayloadVector && p)
	{
		return PayloadField(std::move(p),
		    [](auto & h, const PayloadTemplate::PayloadVector & p) { h.AppendPayload(p); });
	}

	auto inline appendPayload(uint8_t byte, size_t count = 1)
	{
		return appendPayload(PayloadTemplate::PayloadVector(count, byte));
	}

	auto inline appendPayload(const std::string & str, size_t count)
	{
		return appendPayload(PayloadTemplate::FillPayload(str, count));
	}

	auto inline appendPayload(const std::string & p)
	{
		return appendPayload(p, p.size());
	}
}

#endif
