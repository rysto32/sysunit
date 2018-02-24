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

extern "C" {
#define _KERNEL_UT 1
#include <kern_include/sys/types.h>
#include <kern_include/sys/lock.h>
#include <kern_include/sys/sdt.h>
#include <kern_include/sys/systm.h>
#include <kern_include/sys/libkern.h>
#include <kern_include/sys/errno.h>
#include <kern_include/sys/mbuf.h>
}

#include "pktgen/Ipv4Flow.h"

#include "pktgen/L2Flow.h"

#include <stdexcept>

#include <sys/socket.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

namespace PktGen {
	Ipv4Flow::Ipv4Flow(L2Flow& l2, const char* srcIp, const char* dstIp)
	  : L3Flow(l2),
	    tos(0),
	    lastId(std::numeric_limits<uint16_t>::max()),
	    ttl(255)
	{
		int success;

		success = inet_pton(AF_INET, srcIp, &src);
		if (success != 1) {
			throw std::runtime_error("inet_pton(src) failed");
		}

		success = inet_pton(AF_INET, dstIp, &dst);
		if (success != 1)
			throw std::runtime_error("inet_pton(dst) failed");
	}

	uint16_t Ipv4Flow::GetHeaderSize() const
	{
		return sizeof(struct ip);
	}

	uint16_t Ipv4Flow::GetEtherType() const
	{
		return ETHERTYPE_IP;
	}

	void Ipv4Flow::FillPacket(Packet & m, uint8_t proto)
	{
		auto hdrlen = GetHeaderSize();
		auto totlen = hdrlen + m->m_pkthdr.len;


		lastId++;

		auto * ip = m.AddHeader<struct ip>(Packet::Layer::L3);
		m.SetL3Len(totlen);

		ip->ip_hl = hdrlen / sizeof(uint32_t);
		ip->ip_v = 4;

		ip->ip_tos = tos;
		ip->ip_len = ntohs(totlen);
		ip->ip_id = htons(lastId);
		ip->ip_off = 0;
		ip->ip_ttl = ttl;
		ip->ip_p = proto;
		ip->ip_sum = 0;
		ip->ip_src  = src;
		ip->ip_dst = dst;

		l2.FillPacket(m, GetEtherType());
	}
}
