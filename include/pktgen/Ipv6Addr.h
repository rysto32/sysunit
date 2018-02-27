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

#ifndef PKTGEN_IPV6_ADDR_H
#define PKTGEN_IPV6_ADDR_H

extern "C" {
#include <kern_include/sys/types.h>
#include <kern_include/sys/socket.h>
#include <kern_include/netinet/in.h>
#include <kern_include/netinet/ip6.h>
}

#include <ostream>
#include <string>

std::ostream & operator<<(std::ostream & os, struct in6_addr a);

namespace PktGen
{
	class Ipv6Addr
	{
	private:
		struct in6_addr addr;

	public:
		Ipv6Addr()
		  : addr({})
		{
		}

		Ipv6Addr(struct in6_addr a)
		  : addr(a)
		{
		}

		Ipv6Addr(const char *ip);

		struct in6_addr GetAddr() const
		{
			return addr;
		}

		bool operator==(const Ipv6Addr & rhs) const;

		bool operator!=(const Ipv6Addr & rhs) const
		{
			return !(*this == rhs);
		}

		std::string ToString() const;
	};

	std::ostream & operator<<(std::ostream & os, const PktGen::Ipv6Addr & a);
}

#endif
