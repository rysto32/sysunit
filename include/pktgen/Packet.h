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

#include "pktgen/MbufPtr.h"

extern "C" {
#include <kern_include/sys/types.h>
#include <kern_include/netinet/in.h>
}

#include <algorithm>
#include <memory>
#include <stdio.h>
#include <string.h>

namespace PktGen
{
	template <typename Header>
	auto WrapPacketTemplate(const Header & h);

	template <typename Header>
	auto UnwrapTemplate(const Header &);

	template <typename Header>
	class PacketTemplateWrapper
	{
	private:
		Header header;

		typedef PacketTemplateWrapper SelfType;

		struct WithHeaderImpl;
		struct WithHeaderFieldsImpl;

		template <typename Layer>
		class FieldsImpl
		{
		private:
			const Header & header;

			explicit FieldsImpl(const Header & h)
			  : header(h)
			{
			}

			friend struct WithHeaderImpl;
			friend struct WithHeaderFieldsImpl;
			friend PacketTemplateWrapper;

			FieldsImpl() = delete;

			FieldsImpl(const FieldsImpl & f) = default;
			FieldsImpl(FieldsImpl &&) = delete;

			FieldsImpl & operator=(const FieldsImpl&) = delete;
			FieldsImpl & operator=(FieldsImpl &&) = delete;

		public:
			template <typename... FieldList>
			SelfType Fields(FieldList... f)
			{
				return SelfType(header.template WithHeaderFields<Layer>(f...));
			}

			template <typename... FieldList>
			SelfType operator()(FieldList... f)
			{
				return Fields(f...);
			}
		};

		struct WithHeaderFieldsImpl
		{
		private:
			const SelfType & parent;

			WithHeaderFieldsImpl(const SelfType & p)
			  : parent(p)
			{
			}

			friend SelfType;
			WithHeaderFieldsImpl(const WithHeaderFieldsImpl &) = default;
			WithHeaderFieldsImpl & operator=(const WithHeaderFieldsImpl &) = default;

		public:
			template <typename Layer, typename... Fields>
			SelfType operator()(Layer l, Fields... f) const
			{
				return SelfType(parent.header.template WithHeaderFields<Layer>(f...));
			}

			template <typename Layer>
			auto operator[](Layer l) const
			{
				return FieldsImpl<Layer>(parent.header);
			}

			WithHeaderFieldsImpl() = delete;
			WithHeaderFieldsImpl(WithHeaderFieldsImpl &&) = delete;
			WithHeaderFieldsImpl & operator=(WithHeaderFieldsImpl &&) = delete;
		};

		friend WithHeaderImpl;
		friend WithHeaderFieldsImpl;

	public:
		explicit PacketTemplateWrapper(const Header & h)
		  : header(h),
		    WithHeaderFields(*this)
		{
		}

		WithHeaderFieldsImpl WithHeaderFields;

		template <typename Layer>
		auto WithHeader(Layer l) const
		{
			return FieldsImpl<Layer>(header);
		}

		template <typename... Fields>
		SelfType With(Fields... f) const
		{
			return SelfType(header.With(f...));
		}

		template <typename Lower>
		auto EncapIn(const Lower & lower) const
		{
			return WrapPacketTemplate(header.EncapIn(UnwrapTemplate(lower)));
		}

		template <typename Lower>
		auto Encapsulate(const Lower & lower) const
		{
			return WrapPacketTemplate(header.Encapsulate(UnwrapTemplate(lower)));
		}

		Header Unwrap() const
		{
			return header;
		}

		MbufPtr Generate() const
		{
			size_t offset = 0;
			return header.Generate(offset);
		}

		struct mbuf * GenerateRawMbuf() const
		{
			return Generate().release();
		}

		SelfType Next() const
		{
			return SelfType(header.Next());
		}

		SelfType Retransmission() const
		{
			return SelfType(header.Retransmission());
		}

		void print(int depth) const
		{
			header.print(depth);
		}
	};

	template <typename Header>
	auto UnwrapTemplate(const PacketTemplateWrapper<Header> & h)
	{
		return h.Unwrap();
	}

	template <typename Header>
	auto UnwrapTemplate(const Header & h)
	{
		return h;
	}

	template <typename Lower, typename Upper, typename... Headers>
	auto MakePacketTemplate(const Lower & l, const Upper & h, Headers... rest)
	{
		return UnwrapTemplate(l).Encapsulate(MakePacketTemplate(UnwrapTemplate(h), rest...));
	}

	template <typename Header>
	auto MakePacketTemplate(const Header & h)
	{
		return UnwrapTemplate(h);
	}

	template <typename Header>
	auto WrapPacketTemplate(const PacketTemplateWrapper<Header> & h)
	{
		static_assert(!std::is_same<Header, Header>::value);
	}

	template <typename Header>
	auto WrapPacketTemplate(const Header & h)
	{
		return PacketTemplateWrapper<Header>(h);
	}

	template <typename... Headers>
	auto PacketTemplate(Headers... rest)
	{
		return WrapPacketTemplate(MakePacketTemplate(rest...));
	}

	template <typename T, typename Mbuf>
	inline T * GetMbufHeader(const Mbuf & m, size_t offset = 0)
	{
		return reinterpret_cast<T*>(m->m_data + offset);
	}

	inline uint8_t ntoh(uint8_t x)
	{
		return x;
	}

	inline uint16_t ntoh(uint16_t x)
	{
		return ntohs(x);
	}

	inline uint32_t ntoh(uint32_t x)
	{
		return ntohl(x);
	}

	inline uint8_t hton(uint8_t x)
	{
		return x;
	}

	inline uint16_t hton(uint16_t x)
	{
		return htons(x);
	}

	inline uint32_t hton(uint32_t x)
	{
		return htonl(x);
	}
}

#endif
