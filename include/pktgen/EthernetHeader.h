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

#include "pktgen/EtherAddr.h"
#include "pktgen/FieldPropagator.h"
#include "pktgen/Layer.h"
#include "pktgen/L2Fields.h"
#include "pktgen/PacketParsing.h"
#include "pktgen/PrintIndent.h"

namespace PktGen::internal
{

	class EthernetTemplate
	{
	private:
		EtherAddr dst;
		EtherAddr src;
		uint16_t ethertype;
		uint16_t mbVlan;
		size_t outerMtu;
		size_t localMtu;
		size_t payloadLength;


		typedef EthernetTemplate SelfType;

	public:
		const static auto LAYER = LayerVal::L2;

		typedef DefaultOutwardFieldSetter OutwardFieldSetter;

		EthernetTemplate()
		  : ethertype(0),
		    mbVlan(0),
		    outerMtu(DEFAULT_MTU),
		    localMtu(DEFAULT_MTU),
		    payloadLength(0)
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

		uint16_t GetMbufVlan() const
		{
			return mbVlan;
		}

		void SetSrc(const EtherAddr & a)
		{
			src = a;
		}

		void SetDst(const EtherAddr & a)
		{
			dst = a;
		}

		void SetEthertype(uint16_t t)
		{
			ethertype = t;
		}

		void SetMbufVlan(uint16_t v)
		{
			mbVlan = v;
		}

		void FillPacket(mbuf * m, size_t offset) const
		{
			auto * eh = GetMbufHeader<struct ether_header>(m, offset);

			memcpy(eh->ether_dhost, dst.GetAddr(), ETHER_ADDR_LEN);
			memcpy(eh->ether_shost, src.GetAddr(), ETHER_ADDR_LEN);
			eh->ether_type = ntohs(ethertype);

			if (mbVlan != 0) {
				m->m_pkthdr.ether_vtag = mbVlan;
				m->m_flags |= M_VLANTAG;
			}
		}

		size_t GetLen() const
		{
			return sizeof(struct ether_header);
		}

		size_t GetPayloadLength() const
		{
			return payloadLength;
		}

		void SetPayloadLength(size_t len)
		{
			payloadLength = len;
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

		void CalcSegmentation()
		{
		}

		SelfType Next() const
		{
			return *this;
		}

		SelfType Retransmission() const
		{
			return *this;
		}

		void print(int depth) const
		{
			PrintIndent(depth, "Ether : {");
			PrintIndent(depth + 1, "dst : %02x:%02x:%02x:%02x:%02x:%02x",
			    dst.GetAddr()[0], dst.GetAddr()[1], dst.GetAddr()[2],
			    dst.GetAddr()[3], dst.GetAddr()[4], dst.GetAddr()[5]);
			PrintIndent(depth + 1, "src : %02x:%02x:%02x:%02x:%02x:%02x",
			    src.GetAddr()[0], src.GetAddr()[1], src.GetAddr()[2],
			    src.GetAddr()[3], src.GetAddr()[4], src.GetAddr()[5]);
			PrintIndent(depth + 1, "etype : %#x", ethertype);
			PrintIndent(depth, "}");
		}
	};
}

#endif
