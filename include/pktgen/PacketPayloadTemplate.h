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

#include "pktgen/CommonFields.h"
#include "pktgen/FieldPropagator.h"
#include "pktgen/Layer.h"
#include "pktgen/PacketParsing.h"
#include "pktgen/PayloadLength.h"
#include "pktgen/PrintIndent.h"

#include <stdint.h>
#include <vector>

namespace PktGen::internal
{
	typedef std::vector<uint8_t> PayloadVector;

	class PayloadTemplate
	{
	private:
		PayloadVector payload;
		size_t outerMtu;
		size_t localMtu;
		size_t payloadIndex;

		typedef PayloadTemplate SelfType;

	public:
		static const auto LAYER = LayerVal::PAYLOAD;

		typedef DefaultOutwardFieldSetter OutwardFieldSetter;

		PayloadTemplate()
		  : outerMtu(DEFAULT_MTU),
		    localMtu(DEFAULT_MTU),
		    payloadIndex(0)
		{
		}

		void SetPayload(const PayloadVector & p)
		{
			payload = p;
			payloadIndex = 0;
		}

		void AppendPayload(const PayloadVector & p)
		{
			for (auto ch : p)
				payload.push_back(ch);
		}

		void SetLength(size_t s)
		{
			if (s <= payload.size()) {
				payload.resize(s);

				if (s < payloadIndex)
					payloadIndex = s;
			} else {
				throw std::runtime_error("Increasing length not supported yet");
			}
		}

		void FillPacket(mbuf * m, size_t offset) const
		{
			auto * pl = GetMbufHeader<uint8_t>(m, offset);

			if (GetLen() == 0)
				return;

			size_t len = std::min(GetLen(), GetMtu());
			memcpy(pl, &payload[payloadIndex], len);
		}

		void SetPayloadLength(size_t len)
		{
			if (len != 0)
				throw std::runtime_error("Payload does not support encapsulated headers");
		}

		// This returns the length of the payload encapsulated in this
		// header.  As a Payload does not allow anything to be
		// encapsulated within it this is always 0.
		size_t GetPayloadLength() const
		{
			return 0;
		}

		size_t GetMtu() const
		{
			return std::min(localMtu, outerMtu);
		}

		void SetMtu(size_t x)
		{
			localMtu = x;
		}

		void SetOuterMtu(size_t x)
		{
			outerMtu = x;
		}

		size_t GetLen() const
		{
			return std::min(payload.size() - payloadIndex, localMtu);
		}

		size_t GetFillLen() const
		{
			return std::min(GetLen(), outerMtu);
		}

		const PayloadVector & GetPayload() const
		{
			return payload;
		}

		size_t GetStartIndex() const
		{
			return payloadIndex;
		}

		PayloadTemplate Next() const
		{
			PayloadTemplate copy(*this);

			size_t len = GetLen();
			if (len <= GetMtu())
				copy.payloadIndex += len;
			else
				copy.payloadIndex += GetMtu();

			return copy;
		}

		PayloadTemplate Retransmission() const
		{
			return *this;
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

		void print(int depth) const
		{
			PrintIndent(depth, "Payload = {");
			PrintIndent(depth + 1, "len = %zd", GetLen());
			PrintIndent(depth + 1, "off = %zd", payloadIndex);
			PrintIndent(depth + 1, "mtu = %zd", GetMtu());
			PrintIndent(depth, "}");
		}
	};
}

namespace PktGen
{
	auto inline payload(internal::PayloadVector && p)
	{
		return internal::PayloadField([p](auto & h) { h.SetPayload(p); });
	}

	auto inline payload()
	{
		return payload(internal::PayloadVector());
	}

	auto inline payload(uint8_t byte, size_t count = 1)
	{
		return payload(internal::PayloadVector(count, byte));
	}

	auto inline payload(const std::string & str, size_t count)
	{
		return payload(internal::PayloadTemplate::FillPayload(str, count));
	}

	auto inline payload(const char * p)
	{
		std::string str(p);
		return payload(str, str.size());
	}

	auto inline appendPayload(internal::PayloadVector && p)
	{
		return internal::PayloadField([p](auto & h) { h.AppendPayload(p); });
	}

	auto inline appendPayload(uint8_t byte, size_t count = 1)
	{
		return appendPayload(internal::PayloadVector(count, byte));
	}

	auto inline appendPayload(const std::string & str, size_t count)
	{
		return appendPayload(internal::PayloadTemplate::FillPayload(str, count));
	}

	auto inline appendPayload(const char * p)
	{
		std::string str(p);
		return appendPayload(str, str.size());
	}

	auto inline length(size_t size)
	{
		return internal::PayloadField([size] (auto & h) { h.SetLength(size); });
	}
}

#endif
