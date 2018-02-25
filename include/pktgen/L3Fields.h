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

#ifndef PKTGEN_L3_FIELDS_H
#define PKTGEN_L3_FIELDS_H

#include "pktgen/Ipv4Addr.h"

namespace PktGen
{
	auto inline tos(uint8_t x)
	{
		return [x](auto & h) { h.SetTos(x); };
	}

	auto inline id(uint16_t x)
	{
		return [x](auto & h) { h.SetId(x); };
	}

	auto inline ttl(uint8_t x)
	{
		return [x](auto & h) { h.SetTtl(x); };
	}

	auto inline proto(uint8_t x)
	{
		return [x](auto & h) { h.SetProto(x); };
	}

	auto inline checksum(uint16_t x)
	{
		return [x](auto & h) { h.SetChecksum(x); };
	}

	auto inline srcIp(const Ipv4Addr & x)
	{
		return [x](auto & h) { h.SetSrc(x); };
	}

	auto inline dstIp(const Ipv4Addr & x)
	{
		return [x](auto & h) { h.SetDst(x); };
	}

	auto inline checksumVerified(bool verified = true)
	{
		return [verified] (auto & h) { h.SetChecksumVerified(verified); };
	}

	auto inline checksumPassed(bool valid = true)
	{
		return [valid] (auto & h) { h.SetChecksumPassed(valid); };
	}
}

#endif