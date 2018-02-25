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

#ifndef ETHERNET_HEADER_H
#define ETHERNET_HEADER_H

#include "fake/mbuf.h"

#include "pktgen/EncapsulatableHeader.h"
#include "pktgen/EtherAddr.h"
#include "pktgen/Layer.h"
#include "pktgen/L2Fields.h"

namespace PktGen {

	class EthernetTemplate
	{
	private:
		EtherAddr dst;
		EtherAddr src;
		uint16_t ethertype;

	public:
		static const auto LAYER = Layer::L2;

		// This is to appease EncapsulatableHeader
		typedef NullEncapFieldSetter EncapFieldSetter;

		EthernetTemplate()
		  : ethertype(0)
		{
		}

		const EtherAddr & GetDst() const
		{
			return dst;
		}

		const EtherAddr & GetSrc() const
		{
			return src;
		}

		uint16_t GetEthertype() const
		{
			return ethertype;
		}

		void SetSrcAddr(const EtherAddr & a)
		{
			src = a;
		}

		void SetDstAddr(const EtherAddr & a)
		{
			dst = a;
		}

		void SetEthertype(uint16_t t)
		{
			ethertype = t;
		}

		void FillPacket(mbuf * m, size_t parentLen, size_t & offset) const
		{
			auto * eh = GetMbufHeader<ether_header>(m, offset);

			memcpy(eh->ether_dhost, dst.GetAddr(), ETHER_ADDR_LEN);
			memcpy(eh->ether_shost, src.GetAddr(), ETHER_ADDR_LEN);
			eh->ether_type = ntohs(ethertype);

			offset += GetLen();
		}

		size_t GetLen() const
		{
			return sizeof(struct ether_header);
		}

		void SetPayloadLength(size_t)
		{
		}

		EthernetTemplate Next() const
		{
			return *this;
		}

		void print(int depth)
		{
			PrintIndent(depth, "Ether : {");
			PrintIndent(depth + 1, "etype : %#x", ethertype);
			PrintIndent(depth, "}");
		}
	};

	auto inline EthernetHeader()
	{
		return EncapsulatableHeader<EthernetTemplate>();
	}
}

#endif
