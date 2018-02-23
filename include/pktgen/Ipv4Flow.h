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

#ifndef PKTGEN_IPV4_FLOW_H
#define PKTGEN_IPV4_FLOW_H

#include <netinet/in.h>

#include "pktgen/L3Flow.h"

namespace PktGen
{
	class L2Flow;

	class Ipv4Flow : public L3Flow
	{
	private:
		uint8_t tos;
		uint16_t lastId;
		uint8_t ttl;
		struct in_addr src;
		struct in_addr dst;

	public:
		Ipv4Flow(L2Flow & l2, const char * srcIp, const char *dstIp);

		uint16_t GetHeaderSize() const override;
		uint16_t GetEtherType() const override;
		void FillPacket(Packet &, uint8_t proto) override;

		uint8_t GetTos() const
		{
			return tos;
		}

		uint16_t GetLastId() const
		{
			return lastId;
		}

		uint8_t GetTtl() const
		{
			return ttl;
		}

		in_addr GetSrc() const
		{
			return src;
		}

		in_addr GetDst() const
		{
			return dst;
		}
	};
}

#endif
