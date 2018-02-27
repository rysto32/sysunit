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

#ifndef PKTGEN_PAYLOAD_LENGTH_H
#define PKTGEN_PAYLOAD_LENGTH_H

namespace PktGen
{
	template <typename Header>
	class PayloadLengthSetter
	{
	public:
		void operator()(Header & h, size_t length) const
		{
			h.SetPayloadLength(length);
		}
	};

	class PayloadLengthGenerator
	{
	private:
		size_t length;
	public:
		PayloadLengthGenerator(size_t l = 0)
		  : length(l)
		{
		}

		template <typename Header>
		Header Apply(Header h)
		{
			PayloadLengthSetter<Header> setter;

			setter(h, length);

			length += h.GetLen();
			return h;
		}
	};

	template <typename Setter>
	class PayloadField
	{
		std::vector<uint8_t> payload;
		Setter setter;

	public:
		PayloadField(std::vector<uint8_t> && p, Setter s)
		  : payload(p),
		    setter(s)
		{
		}

		typedef PayloadLengthGenerator DownwardFieldGenerator;
		typedef DownwardFieldGenerator ApplyReturn;

		template <typename Header>
		ApplyReturn operator()(Header & h) const
		{
			setter(h, payload);
			return DownwardFieldGenerator(h.GetLen());
		}
	};

	class PayloadLengthField
	{
		size_t size;

	public:
		PayloadLengthField(size_t s)
		  : size(s)
		{
		}

		typedef PayloadLengthGenerator DownwardFieldGenerator;
		typedef DownwardFieldGenerator ApplyReturn;

		template <typename Header>
		ApplyReturn operator()(Header & h) const
		{
			h.SetPayloadLength(size);
			return DownwardFieldGenerator(h.GetLen() + size);
		}
	};
}

#endif