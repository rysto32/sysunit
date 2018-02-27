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

#ifndef PKTGEN_TCP_HEADER_H
#define PKTGEN_TCP_HEADER_H

#include "fake/mbuf.h"

extern "C" {
#include <kern_include/sys/types.h>
#include <kern_include/netinet/in.h>
#include <kern_include/netinet/tcp.h>
}

#include "pktgen/EncapsulatableHeader.h"
#include "pktgen/Layer.h"
#include "pktgen/L3Fields.h"
#include "pktgen/L4Fields.h"
#include "pktgen/PayloadLength.h"
#include "pktgen/PacketTemplates.h"

namespace PktGen
{
	template <typename Nesting>
	class TcpTemplate
	{
	private:
		uint16_t th_sport;
		uint16_t th_dport;
		uint32_t th_seq;
		uint32_t th_ack;
		uint8_t th_off;
		uint8_t th_x2;
		uint8_t th_flags;
		uint16_t th_win;
		uint16_t th_sum;
		uint16_t th_urp;

		bool checksumVerified;
		bool checksumPassed;
		size_t payloadLength;

		typedef TcpTemplate<Nesting> SelfType;

	public:
		typedef typename Nesting::NextL4 NESTING_LEVEL;
		typedef typename NESTING_LEVEL::L4 LAYER;

		struct EncapFieldSetter
		{
			template <typename Header>
			Header operator()(const Header & h, const TcpTemplate & t) const
			{
				DefaultEncapFieldSetter setter;
				return setter(h, t).template With(
				    proto(t.GetIpProto())
				);
			}
		};

		TcpTemplate()
		  : th_sport(0),
		    th_dport(0),
		    th_seq(0),
		    th_ack(0),
		    th_off(sizeof(struct tcphdr) / sizeof(uint32_t)),
		    th_x2(0),
		    th_flags(TH_ACK),
		    th_win(4),
		    th_sum(0),
		    th_urp(0),
		    checksumVerified(false),
		    checksumPassed(false),
		    payloadLength(0)
		{
		}

		template <typename U>
		TcpTemplate(const TcpTemplate<U> & h)
		  : th_sport(h.GetSrcPort()),
		    th_dport(h.GetDstPort()),
		    th_seq(h.GetSeq()),
		    th_ack(h.GetAck()),
		    th_off(h.GetOff()),
		    th_x2(h.GetX2()),
		    th_flags(h.GetFlags()),
		    th_win(h.GetWindow()),
		    th_sum(h.GetChecksum()),
		    th_urp(h.GetUrgentPointer()),
		    checksumVerified(h.GetChecksumVerified()),
		    checksumPassed(h.GetChecksumPassed()),
		    payloadLength(h.GetPayloadLength())
		{
		}

		uint16_t GetSrcPort() const
		{
			return th_sport;
		}

		void SetSrc(uint16_t x)
		{
			th_sport = x;
		}

		uint16_t GetDstPort() const
		{
			return th_dport;
		}

		void SetDst(uint16_t x)
		{
			th_dport = x;
		}

		uint32_t GetSeq() const
		{
			return th_seq;
		}

		void SetSeq(uint32_t x)
		{
			th_seq = x;
		}

		uint32_t GetAck() const
		{
			return th_ack;
		}

		void SetAck(uint32_t x)
		{
			th_ack = x;
		}

		uint8_t GetOff() const
		{
			return th_off;
		}

		uint8_t GetX2() const
		{
			return th_x2;
		}

		uint8_t GetFlags() const
		{
			return th_flags;
		}

		void SetFlags(uint8_t x)
		{
			th_flags = x;
		}

		uint16_t GetWindow() const
		{
			return th_win;
		}

		void SetWindow(uint16_t x)
		{
			th_win = x;
		}

		uint16_t GetChecksum() const
		{
			return th_sum;
		}

		void SetChecksum(uint16_t x)
		{
			th_sum = x;
		}

		uint16_t GetUrgentPointer() const
		{
			return th_urp;
		}

		void SetUrgentPointer(uint16_t x)
		{
			th_urp = x;
		}

		bool GetChecksumVerified() const
		{
			return checksumVerified;
		}

		void SetChecksumVerified(bool v)
		{
			checksumVerified = v;
		}

		bool GetChecksumPassed() const
		{
			return checksumPassed;
		}

		void SetChecksumPassed(bool v)
		{
			checksumPassed = v;
		}

		constexpr static uint8_t GetIpProto()
		{
			return IPPROTO_TCP;
		}

		size_t GetLen() const
		{
			return th_off * sizeof(uint32_t);
		}

		size_t GetPayloadLength() const
		{
			return payloadLength;
		}

		void SetPayloadLength(size_t len)
		{
			payloadLength = len;
		}

		TcpTemplate Next() const
		{
			TcpTemplate copy(*this);
			copy.SetSeq(th_seq + payloadLength);
			return copy;
		}

		void FillPacket(mbuf * m, size_t parentLen, size_t & offset) const
		{
			auto * tcp = GetMbufHeader<tcphdr>(m, offset);

			tcp->th_sport = hton(th_sport);
			tcp->th_dport = hton(th_dport);

			tcp->th_seq = hton(th_seq);
			tcp->th_ack = hton(th_ack);

			tcp->th_x2 = hton(th_x2);
			tcp->th_off = hton(th_off);
			tcp->th_flags = hton(th_flags);

			tcp->th_win = hton(th_win);
			tcp->th_sum = hton(th_sum);
			tcp->th_urp = hton(th_urp);

			if (checksumVerified) {
				m->m_pkthdr.csum_flags |= CSUM_L4_CALC;
				if (checksumPassed) {
					m->m_pkthdr.csum_flags |= CSUM_L4_VALID;
				}
			}

			offset += GetLen();
		}

		template <typename NestingLevel>
		static auto MakeNested(const SelfType & up)
		{
			return TcpTemplate<NestingLevel>(up);
		}

		UnnestedTcpTemplate StripNesting() const
		{
			return UnnestedTcpTemplate(*this);
		}

		void print(int depth)
		{
			PrintIndent(depth, "TCP : {");
			PrintIndent(depth + 1, "seq : %d", th_seq);
			PrintIndent(depth + 1, "payloadLen : %d", payloadLength);
			PrintIndent(depth, "}");
		}
	};

	auto inline TcpHeader()
	{
		return EncapsulatableHeader<UnnestedTcpTemplate>();
	}
}

#endif
