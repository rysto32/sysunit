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

#ifndef PKTGEN_IPV4_HEADER_H
#define PKTGEN_IPV4_HEADER_H

#include "fake/mbuf.h"

extern "C" {
#include <kern_include/sys/types.h>
#include <kern_include/net/ethernet.h>
#include <kern_include/netinet/in.h>
#include <kern_include/netinet/ip.h>
}

#include "pktgen/EncapsulatableHeader.h"
#include "pktgen/Ipv4Addr.h"
#include "pktgen/Layer.h"
#include "pktgen/L2Fields.h"
#include "pktgen/L3Fields.h"
#include "pktgen/PayloadLength.h"
#include "pktgen/PacketTemplates.h"

namespace PktGen
{
	template <typename Nesting>
	class Ipv4Template
	{
	private:
		uint8_t headerLen;
		uint8_t version;
		uint8_t tos;
		uint16_t id;
		uint16_t off;
		uint16_t ipLen;
		uint8_t ttl;
		uint8_t proto;
		uint16_t checksum;
		Ipv4Addr src;
		Ipv4Addr dst;

		bool checksumVerified;
		bool checksumPassed;

		typedef Ipv4Template<Nesting> SelfType;

	public:
		typedef typename Nesting::NextL3 NESTING_LEVEL;
		typedef typename NESTING_LEVEL::L3 LAYER;

		struct EncapFieldSetter
		{
			template <typename Header>
			Header operator()(const Header & h, const Ipv4Template & t) const
			{
				DefaultEncapFieldSetter setter;
				return setter(h, t).template With(
				    ethertype(GetEthertype())
				);
			}
		};

		Ipv4Template()
		  : headerLen(sizeof(struct ip) / sizeof(uint32_t)),
		    version(4),
		    tos(0),
		    id(0),
		    off(0),
		    ipLen(GetLen()),
		    ttl(255),
		    proto(0),
		    checksum(0),
		    checksumVerified(false),
		    checksumPassed(false)
		{
		}

		template <typename U>
		explicit Ipv4Template(const Ipv4Template<U> & h)
		  : headerLen(h.GetHeaderLen()),
		    version(h.GetVersion()),
		    tos(h.GetTos()),
		    id(h.GetId()),
		    off(h.GetOff()),
		    ipLen(h.GetIpLen()),
		    ttl(h.GetTtl()),
		    proto(h.GetProto()),
		    checksum(h.GetChecksum()),
		    src(h.GetSrc()),
		    dst(h.GetDst()),
		    checksumVerified(h.GetChecksumVerified()),
		    checksumPassed(h.GetChecksumPassed())
		{
		}

		uint8_t GetHeaderLen() const
		{
			return headerLen;
		}

		void SetHeaderLen(uint8_t x)
		{
			headerLen = x;
		}

		uint8_t GetVersion() const
		{
			return version;
		}

		void SetVersion(uint8_t v)
		{
			version = v;
		}

		uint8_t GetTos() const
		{
			return tos;
		}

		void SetTos(uint8_t t)
		{
			tos = t;
		}

		uint16_t GetId() const
		{
			return id;
		}

		void SetId(uint16_t i)
		{
			id = i;
		}

		uint16_t GetOff() const
		{
			return off;
		}

		uint16_t GetIpLen() const
		{
			return ipLen;
		}

		uint8_t GetTtl() const
		{
			return ttl;
		}

		void SetTtl(uint8_t t)
		{
			ttl = t;
		}

		uint8_t GetProto() const
		{
			return proto;
		}

		void SetProto(uint8_t p)
		{
			proto = p;
		}

		uint16_t GetChecksum() const
		{
			return checksum;
		}

		void SetChecksum(uint16_t sum)
		{
			checksum = sum;
		}

		const Ipv4Addr & GetSrc() const
		{
			return src;
		}

		void SetSrc(const Ipv4Addr & a)
		{
			src = a;
		}

		const Ipv4Addr & GetDst() const
		{
			return dst;
		}

		void SetDst(const Ipv4Addr & a)
		{
			dst = a;
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

		size_t GetPayloadLength() const
		{
			return ipLen - GetLen();
		}

		void SetPayloadLength(size_t payLen)
		{
			ipLen = GetLen() + payLen;
		}

		constexpr static uint16_t GetEthertype()
		{
			return ETHERTYPE_IP;
		}

		void FillPacket(mbuf * m,size_t & offset) const
		{
			auto * ip = GetMbufHeader<struct ip>(m, offset);

			ip->ip_hl = hton(headerLen);
			ip->ip_v = hton(version);

			ip->ip_tos = hton(tos);
			ip->ip_len = hton(ipLen);
			ip->ip_id = hton(id);
			ip->ip_off = hton(off);
			ip->ip_ttl = hton(ttl);
			ip->ip_p = hton(proto);
			ip->ip_sum = hton(checksum);
			ip->ip_src = src.GetAddr();
			ip->ip_dst = dst.GetAddr();

			if (checksumVerified) {
				m->m_pkthdr.csum_flags |= CSUM_L3_CALC;
				if (checksumPassed) {
					m->m_pkthdr.csum_flags |= CSUM_L3_VALID;
				}
			}

			offset += GetLen();
		}

		size_t GetLen() const
		{
			return sizeof(struct ip);
		}

		Ipv4Template Next() const
		{
			Ipv4Template copy(*this);
			copy.SetId(id + 1);
			return copy;
		}

		Ipv4Template Retransmission() const
		{
			return Next();
		}

		template <typename NestingLevel>
		static auto MakeNested(const SelfType & up)
		{
			return Ipv4Template<NestingLevel>(up);
		}

		UnnestedIpv4Template StripNesting() const
		{
			return UnnestedIpv4Template(*this);
		}

		void print(int depth) const
		{
			PrintIndent(depth, "IPv4 : {");
			PrintIndent(depth + 1, "proto : %d", proto);
			PrintIndent(depth + 1, "ip_len : %d", ipLen);
			PrintIndent(depth + 1, "hl : %d", headerLen);
			PrintIndent(depth + 1, "id : %d", id);
			PrintIndent(depth, "}");
		}
	};

	auto inline Ipv4Header()
	{
		return EncapsulatableHeader<Ipv4Template<DefaultNestingLevel>>();
	}
}

#endif
