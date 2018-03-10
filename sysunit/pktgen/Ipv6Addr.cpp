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

#include "pktgen/Ipv6Addr.h"

#include <arpa/inet.h>

#include <cstring>
#include <stdexcept>

std::ostream & operator<<(std::ostream & os, struct in6_addr a)
{
	return os << PktGen::internal::Ipv6Addr(a);
}

namespace PktGen::internal
{
	std::ostream & operator<<(std::ostream & os, const PktGen::internal::Ipv6Addr & a)
	{
		return os << a.ToString();
	}

	Ipv6Addr::Ipv6Addr(const char *ip6)
	{
		int success;

		success = inet_pton(AF_INET6, ip6, &addr);
		if (success != 1) {
			throw std::runtime_error("inet_pton(AF_INET6) failed");
		}
	}

	bool Ipv6Addr::operator==(const Ipv6Addr & rhs) const
	{
		return std::memcmp(&addr, &rhs.addr, sizeof(addr)) == 0;
	}

	std::string Ipv6Addr::ToString() const
	{
		char addrStr[INET6_ADDRSTRLEN];
		const char * str = inet_ntop(AF_INET6, &addr, addrStr, sizeof(addr));

		if (str == NULL)
			throw std::runtime_error("inet_ntop(AF_INET6) failed");

		return str;
	}
}
