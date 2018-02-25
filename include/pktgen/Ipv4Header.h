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

namespace PktGen
{
	class Ipv4Template
	{
	private:
		uint8_t header_len;
		uint8_t tos;
		uint16_t id;
		uint16_t off;
		uint8_t ttl;
		uint8_t proto;
		uint16_t checksum;
		Ipv4Addr src;
		Ipv4Addr dst;

		bool checksumVerified;
		bool checksumPassed;

	public:
		static const auto LAYER = Layer::L3;

		Ipv4Template()
		  : header_len(sizeof(struct ip) / sizeof(uint32_t)),
		    tos(0),
		    id(0),
		    off(0),
		    ttl(255),
		    proto(0),
		    checksum(0),
		    checksumVerified(false),
		    checksumPassed(false)
		{
		}

		void SetTos(uint8_t t)
		{
			tos = t;
		}

		void SetId(uint16_t i)
		{
			id = i;
		}

		void SetTtl(uint8_t t)
		{
			ttl = t;
		}

		void SetProto(uint8_t p)
		{
			proto = p;
		}

		void SetChecksum(uint16_t sum)
		{
			checksum = sum;
		}

		void SetSrc(const Ipv4Addr & a)
		{
			src = a;
		}

		void SetDst(const Ipv4Addr & a)
		{
			dst = a;
		}

		void SetChecksumVerified(bool v)
		{
			checksumVerified = v;
		}

		void SetChecksumPassed(bool v)
		{
			checksumPassed = v;
		}

		constexpr static uint16_t GetEthertype()
		{
			return ETHERTYPE_IP;
		}

		void FillPacket(mbuf * m, size_t parentLen, size_t & offset) const
		{
			auto * ip = GetMbufHeader<struct ip>(m, offset);

			ip->ip_hl = hton(header_len);
			ip->ip_v = 4;

			uint16_t ip_len = GetLen() + parentLen;
			ip->ip_tos = hton(tos);
			ip->ip_len = hton(ip_len);
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
			return header_len * sizeof(uint32_t);
		}

		struct EncapFieldSetter
		{
			template <typename T>
			void operator()(T & t)
			{
				ethertype(GetEthertype())(t);
			}
		};

		void print(int depth)
		{
			PrintIndent(depth, "IPv4 : {");
			PrintIndent(depth + 1, "proto : %d", proto);
			PrintIndent(depth, "}");
		}
	};

	auto Ipv4Header()
	{
		return EncapsulatableHeader<Ipv4Template>();
	}

	auto tos(uint8_t x)
	{
		return [x](auto & h) { h.SetTos(x); };
	}

	auto id(uint16_t x)
	{
		return [x](auto & h) { h.SetId(x); };
	}

	auto ttl(uint8_t x)
	{
		return [x](auto & h) { h.SetTtl(x); };
	}

	auto proto(uint8_t x)
	{
		return [x](auto & h) { h.SetProto(x); };
	}

	auto checksum(uint16_t x)
	{
		return [x](auto & h) { h.SetChecksum(x); };
	}

	auto srcIp(const Ipv4Addr & x)
	{
		return [x](auto & h) { h.SetSrc(x); };
	}

	auto dstIp(const Ipv4Addr & x)
	{
		return [x](auto & h) { h.SetDst(x); };
	}

	auto checksumVerified(bool verified = true)
	{
		return [verified] (auto & h) { h.SetChecksumVerified(verified); };
	}

	auto checksumPassed(bool valid = true)
	{
		return [valid] (auto & h) { h.SetChecksumPassed(valid); };
	}
}

#endif
