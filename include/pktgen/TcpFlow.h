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

#ifndef PKTGEN_TCP_FLOW_H
#define PKTGEN_TCP_FLOW_H

#include "pktgen/L3Flow.h"

#include <stdint.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

namespace PktGen
{
	class L3Flow;

	class TcpFlow
	{
	private:
		L3Flow & l3;
		uint16_t sport;
		uint16_t dport;

		uint16_t window;

		uint32_t lastSeq;
		uint32_t seqno;
		uint32_t ackno;

	public:
		TcpFlow(L3Flow & l3, uint16_t src, uint16_t dst,
		    uint32_t seq, uint32_t ack, uint16_t window = 4);

		uint32_t GetHeaderLen() const
		{
			return sizeof(struct tcphdr);
		}

		uint8_t GetIpProto() const
		{
			return IPPROTO_TCP;
		}

		static const int DEFAULT_FLAGS = TH_ACK;

		Packet GetNextPacket(uint32_t seqOff, uint32_t len,
		    const char * fill = NULL, uint8_t flags = DEFAULT_FLAGS);

		void AdvanceAck(uint32_t ackOff)
		{
			ackno += ackOff;
		}

		void SetWindow(uint16_t w)
		{
			window = w;
		}

		uint16_t GetSrcPort() const
		{
			return sport;
		}

		uint16_t GetDstPort() const
		{
			return dport;
		}

		uint32_t GetLastSeq() const
		{
			return lastSeq;
		}

		uint32_t GetLastAck() const
		{
			return ackno;
		}

		uint8_t GetDefaultFlags() const
		{
			return DEFAULT_FLAGS;
		}

		uint16_t GetWindow() const
		{
			return window;
		}
	};
}

#endif
