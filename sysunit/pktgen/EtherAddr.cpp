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

#include "pktgen/EtherAddr.h"

#include <stdexcept>
#include <stdio.h>
#include <string.h>

namespace PktGen
{
	EtherAddr::EtherAddr()
	{
		memset(addr.octet, 0, ETHER_ADDR_LEN);
	}

	EtherAddr::EtherAddr(const uint8_t * a)
	{
		memcpy(addr.octet, a, ETHER_ADDR_LEN);
	}

	EtherAddr::EtherAddr(const char * str)
	{
		int count = sscanf(str, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx",
		    &addr.octet[0], &addr.octet[1], &addr.octet[2],
		    &addr.octet[3], &addr.octet[4], &addr.octet[5]);

		if (count != ETHER_ADDR_LEN)
			throw std::runtime_error("parse ether addr failed");
	}

	bool EtherAddr::operator==(const EtherAddr & rhs) const
	{
		return memcmp(addr.octet, rhs.addr.octet, ETHER_ADDR_LEN) == 0;
	}
}