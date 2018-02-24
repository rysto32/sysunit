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

#include <stdlib.h>

#include "pktgen/EtherFlow.h"

#include <stdexcept>

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <netinet/in.h>

namespace PktGen {
	EtherFlow::EtherFlow(const char* srcAddr, const char* dstAddr)
	  : dst(dstAddr),
	    src(srcAddr)
	{
	}

	uint16_t EtherFlow::GetHeaderSize() const
	{
		return sizeof(struct ether_header);
	}

	void EtherFlow::FillPacket(Packet & m, uint16_t ethertype)
	{
		auto * ether = m.AddHeader<ether_header>(Packet::Layer::L2);

		memcpy(ether->ether_dhost, dst.GetAddr(), ETHER_ADDR_LEN);
		memcpy(ether->ether_shost, src.GetAddr(), ETHER_ADDR_LEN);
		ether->ether_type = htons(ethertype);
	}
}

