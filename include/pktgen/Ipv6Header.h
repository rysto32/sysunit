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

#ifndef PKTGEN_IPV6_HEADER_H
#define PKTGEN_IPV6_HEADER_H

#include "fake/mbuf.h"

extern "C" {
#include <kern_include/sys/types.h>
#include <kern_include/net/ethernet.h>
#include <kern_include/netinet/in.h>
#include <kern_include/netinet/ip6.h>
}

#include "pktgen/FieldPropagator.h"
#include "pktgen/Ipv6Addr.h"
#include "pktgen/Layer.h"
#include "pktgen/L2Fields.h"
#include "pktgen/L3Fields.h"
#include "pktgen/PayloadLength.h"
#include "pktgen/PacketParsing.h"
#include "pktgen/PrintIndent.h"

namespace PktGen::internal
{
	class Ipv6Template
	{
	private:
		uint8_t ipv6_version;
		uint8_t ipv6_class;
		uint32_t ipv6_flow;
		uint16_t ipv6_plen;
		uint8_t  ipv6_nxt;
		uint8_t  ipv6_hlim;
		Ipv6Addr ipv6_src;
		Ipv6Addr ipv6_dst;

		size_t outerMtu;
		size_t localMtu;

		typedef Ipv6Template SelfType;

	public:
		static const auto LAYER = LayerVal::L3;

		struct OutwardFieldSetter
		{
			template <typename Header>
			void operator()(Header & h, const Ipv6Template & t) const
			{
				DefaultOutwardFieldSetter setter;

				setter(h, t);
				ethertype(GetEthertype())(h);
			}
		};

		Ipv6Template()
		  : ipv6_version(6),
		    ipv6_class(0),
		    ipv6_flow(0),
		    ipv6_plen(0),
		    ipv6_nxt(0),
		    ipv6_hlim(255),
		    outerMtu(DEFAULT_MTU),
		    localMtu(DEFAULT_MTU)
		{
		}

		uint8_t GetVersion() const
		{
			return ipv6_version;
		}

		void SetVersion(uint8_t v)
		{
			ipv6_version = v;
		}

		auto GetClass() const
		{
			return ipv6_class;
		}

		void SetClass(uint8_t x)
		{
			ipv6_class = x;
		}

		auto GetFlow() const
		{
			return ipv6_flow;
		}

		void SetFlow(uint32_t x)
		{
			ipv6_flow = x;
		}

		auto GetPayloadLength() const
		{
			return ipv6_plen;
		}

		void SetPayloadLength(uint16_t x)
		{
			ipv6_plen = x;
		}

		auto GetProto() const
		{
			return ipv6_nxt;
		}

		void SetProto(uint8_t x)
		{
			ipv6_nxt = x;
		}

		auto GetHopLimit() const
		{
			return ipv6_hlim;
		}

		void SetTtl(uint8_t x)
		{
			ipv6_hlim = x;
		}

		const auto & GetSrc() const
		{
			return ipv6_src;
		}

		void SetSrc(const Ipv6Addr & x)
		{
			ipv6_src = x;
		}

		const auto & GetDst() const
		{
			return ipv6_dst;
		}

		void SetDst(const Ipv6Addr & x)
		{
			ipv6_dst = x;
		}

		size_t GetMtu() const
		{
			return std::min(localMtu, outerMtu);
		}

		void SetMtu(size_t x)
		{
			if (x > IPV6_MAXPACKET)
				throw std::runtime_error("IPV6 jumbo packet not supported");
			localMtu = x;
		}

		void SetOuterMtu(size_t x)
		{
			outerMtu = x;
		}

		size_t GetLen() const
		{
			return sizeof(struct ip6_hdr);
		}

		constexpr static uint16_t GetEthertype()
		{
			return ETHERTYPE_IPV6;
		}

		void FillPacket(mbuf * m, size_t offset) const
		{
			auto * ip6 = GetMbufHeader<ip6_hdr>(m, offset);

			ip6->ip6_flow = hton(
			    (ipv6_version << 28) |
			    (ipv6_class << 20) |
			    (ipv6_flow & 0xfffff));
			ip6->ip6_nxt = hton(ipv6_nxt);
			ip6->ip6_hlim = hton(ipv6_hlim);
			ip6->ip6_plen = hton(ipv6_plen);
			ip6->ip6_src = ipv6_src.GetAddr();
			ip6->ip6_dst = ipv6_dst.GetAddr();
		}

		Ipv6Template Next() const
		{
			return *this;
		}

		Ipv6Template Retransmission() const
		{
			return *this;
		}

		void print(int depth) const
		{
			std::string srcStr(ipv6_src.ToString());
			std::string dstStr(ipv6_dst.ToString());

			PrintIndent(depth, "IPv6 : {");
			PrintIndent(depth +1, "src : %s", srcStr.c_str());
			PrintIndent(depth +1, "dst : %s", dstStr.c_str());
			PrintIndent(depth + 1, "proto : %d", ipv6_nxt);
			PrintIndent(depth + 1, "ip6_plen : %d", ipv6_plen);
			PrintIndent(depth, "}");
		}
	};
}

#endif
