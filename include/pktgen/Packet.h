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

#ifndef PKTGEN_PACKET_H
#define PKTGEN_PACKET_H

#include "fake/mbuf.h"

#include <algorithm>
#include <memory>
#include <stdio.h>
#include <string.h>

namespace PktGen
{
	class Packet
	{
	public:
		enum class Layer
		{
			L2 = 0,
			L3,
			L4,
			MAX_LAYERS
		};

	private:
		class MbufDeleter
		{
		public:
			void operator()(struct mbuf * m) const
			{
				m_freem(m);
			}
		};

		typedef std::unique_ptr<mbuf, MbufDeleter> MbufPtr;

		MbufPtr m;
		size_t headerLen[int(Layer::MAX_LAYERS)];
		size_t l3Payload;

		void
		AddHeader(size_t hdrlen)
		{
			MPASS(M_LEADINGSPACE(m.get()) >= hdrlen);
			m->m_data -= hdrlen;
			m->m_len += hdrlen;
			m->m_pkthdr.len += hdrlen;
		}

	public:
		Packet(size_t size)
		  : m(alloc_mbuf(size)),
		    l3Payload(0)
		{
			memset(headerLen, 0, sizeof(headerLen));
		}

		Packet(const Packet &) = delete;
		Packet & operator=(const Packet &) = delete;

		Packet(Packet &&) = default;
		Packet & operator=(Packet &&) = default;

		template <typename Header>
		Header *
		AddHeader(Layer layer)
		{
			if (layer > Layer::L4)
				throw std::runtime_error("Invalid layer");

			AddHeader(sizeof(Header));
			headerLen[int(layer)] = sizeof(Header);

			return mtod(m.get(), Header*);
		}

		void SetL3Len(size_t len)
		{
			l3Payload = len;
		}

		auto GetL3Len() const
		{
			return l3Payload;
		}

		size_t GetHeaderLen(Layer layer)
		{
			if (layer > Layer::L4)
				throw std::runtime_error("Invalid layer");

			return headerLen[int(layer)];
		}

		void ReserveHeader(size_t hdrlen)
		{
			MPASS(M_TRAILINGSPACE(m.get()) > hdrlen);
			m->m_data += hdrlen;
		}

		void FillPayload(const char * pattern, size_t len)
		{
			MPASS(M_TRAILINGSPACE(m.get()) > len);

			m->m_len = len;
			m->m_pkthdr.len = len;

			if (pattern == nullptr)
				memset(m->m_data, 0, len);
			else {
				auto * dst = mtod(m, uint8_t*);
				size_t remaining = len;
				size_t patlen = strlen(pattern);
				while (remaining > 0) {
					size_t copylen = std::min(remaining, patlen);
					memcpy(dst, pattern, copylen);
					dst += copylen;
					remaining -= copylen;
				}
			}
		}

		mbuf * get()
		{
			return m.get();
		}

		mbuf * operator->()
		{
			return m.get();
		}
	};
}

#endif
