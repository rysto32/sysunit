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

#define _KERNEL_UT 1

#include "pktgen/TcpFlow.h"

namespace PktGen
{
	TcpFlow::TcpFlow(L3Flow & l3, uint16_t src, uint16_t dst,
		uint32_t seq, uint32_t ack, uint16_t window)
		: l3(l3),
		sport(src),
		dport(dst),
		window(window),
		seqno(seq),
		ackno(ack)
	{
	}

	Packet TcpFlow::GetNextPacket(uint32_t seqOff, uint32_t len,
		const char * fill, uint8_t flags)
	{
		uint16_t hdrlen = GetHeaderLen();
		auto m = l3.AllocPacket(len + hdrlen);

		m.ReserveHeader(hdrlen);
		m.FillPayload(fill, len);
		auto * tcp = m.AddHeader<tcphdr>(Packet::Layer::L4);

		tcp->th_sport = htons(sport);
		tcp->th_dport = htons(dport);

		lastSeq = seqno + seqOff;
		tcp->th_seq = htonl(lastSeq);
		tcp->th_ack = htonl(ackno);

		if (seqOff == 0)
			seqno += len;

		tcp->th_x2 = 0;
		tcp->th_off = hdrlen / sizeof(uint32_t);
		tcp->th_flags = flags;

		tcp->th_win = ntohs(window);
		tcp->th_sum = 0;
		tcp->th_urp = 0;

		l3.FillPacket(m, GetIpProto());

		return m;
	}
}
