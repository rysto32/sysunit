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

#include "pktgen/Layer.h"
#include "pktgen/MbufPtr.h"
#include "pktgen/PayloadLength.h"

extern "C" {
#include <kern_include/sys/types.h>
#include <kern_include/netinet/in.h>
}

#include <algorithm>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <tuple>

#include <iostream>

namespace PktGen
{
	struct DefaultOutwardFieldSetter
	{
		template <typename Lower, typename Upper>
		void operator()(Lower & l, const Upper & u) const
		{
			PayloadLengthSetter<Lower>()(l, u.GetLen() + u.GetPayloadLength());
		}
	};

	template <typename... Headers>
	class PacketTemplateWrapper
	{
	private:
		std::tuple<Headers...> headers;

		typedef PacketTemplateWrapper SelfType;

		struct WithHeaderImpl;

		template <std::size_t... Indices, typename... Types>
		static auto SliceFrontImpl(std::index_sequence<Indices...>, std::tuple<Types...> tuple)
		{
			return std::make_tuple(std::get<Indices>(tuple)...);
		}

		template <std::size_t index, typename... Types>
		static auto SliceFront(const std::tuple<Types...> & tuple)
		{
			return SliceFrontImpl(std::make_index_sequence<index>(), tuple);
		}

		template <std::size_t index, std::size_t... Indices, typename... Types>
		static auto SliceBackImpl(std::index_sequence<Indices...>, std::tuple<Types...> tuple)
		{
			return std::make_tuple(std::get<Indices + index + 1>(tuple)...);
		}

		template <std::size_t index, typename... Types>
		static auto SliceBack(const std::tuple<Types...> & tuple)
		{
			return SliceBackImpl<index>(std::make_index_sequence<sizeof...(Types) - index - 1>(), tuple);
		}

		template <std::size_t HeaderIndex>
		class FieldsImpl
		{
		private:
			const std::tuple<Headers...> & headers;

			explicit FieldsImpl(const std::tuple<Headers...> & h)
			  : headers(h)
			{
			}

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
				return SelfType(std::tuple_cat(SliceFront<HeaderIndex>(headers),
				    std::make_tuple(Apply(std::get<HeaderIndex>(headers), f...)),
				    SliceBack<HeaderIndex>(headers)));
			}
		};

		template <typename... Types>
		static auto Head(std::tuple<Types...> tuple)
		{
			return SliceFront<sizeof...(Types) - 1>(tuple);
		}

		template <typename... Types>
		static auto Tail(std::tuple<Types...> tuple)
		{
			return std::get<sizeof...(Types) - 1>(tuple);
		}

		template <typename T, typename... Mutators>
		static T Apply(const T & original, Mutators... mutators)
		{
			T copy(original);

			return ApplyMutators(copy, mutators...);
		}

		template <typename T, typename Mutator, typename... Rest>
		static T & ApplyMutators(T & header, const Mutator & m, Rest... rest)
		{
			m(header);
			return ApplyMutators(header, rest...);
		}

		template <typename T>
		static T & ApplyMutators(T & header)
		{
			return header;
		}

		template <LayerVal Layer, int Nesting, typename Header, typename...Rest>
		static constexpr
		typename std::enable_if<Layer != Header::LAYER, std::size_t>::type
		FindLayerFromOuter()
		{
			return SelfType::FindLayerFromOuter<Layer, Nesting, Rest...>() + 1;
		}

		template <LayerVal Layer, int Nesting, typename Header, typename...Rest>
		static constexpr
		typename std::enable_if<Layer == Header::LAYER && std::greater<int>()(Nesting, 1), std::size_t>::type
		FindLayerFromOuter()
		{
			return SelfType::FindLayerFromOuter<Layer, Nesting - 1, Rest...>() + 1;
		}

		template <LayerVal Layer, int Nesting, typename Header, typename...Rest>
		static constexpr
		typename std::enable_if<Layer == Header::LAYER && Nesting == 1, std::size_t>::type
		FindLayerFromOuter()
		{
			return 0;
		}

		template <LayerVal Layer, typename Header, typename...Rest>
		static constexpr
		typename std::enable_if<Layer != Header::LAYER, std::size_t>::type
		CountLayers()
		{
			return SelfType::CountLayers<Layer, Rest...>();
		}

		template <LayerVal Layer, typename Header, typename...Rest>
		static constexpr
		typename std::enable_if<Layer == Header::LAYER, std::size_t>::type
		CountLayers()
		{
			return SelfType::CountLayers<Layer, Rest...>() + 1;
		}

		template <LayerVal Layer>
		static constexpr std::size_t
		CountLayers()
		{
			return 0;
		}

		template <LayerVal Layer, int Nesting>
		static constexpr std::size_t FindLayer()
		{
			static_assert(Nesting != 0);
			constexpr std::size_t NestCount =  Nesting > 0 ?
			    Nesting :
			    CountLayers<Layer, Headers...>() + Nesting + 1;

			return SelfType::FindLayerFromOuter<Layer, NestCount, Headers...>();
		}

		template <typename First, typename Second, typename...Rest>
		static void PropagateOutwards(First & first, Second & second, Rest &... rest)
		{
			PropagateOutwards(second, rest...);
			typename Second::OutwardFieldSetter setter;
			setter(first, second);
		}

		template <typename First>
		static void PropagateOutwards(First & first)
		{
		}

		void PropagateOutwardFieldSetters()
		{
			std::apply([] (auto &... headers)
				{
					PropagateOutwards(headers...);
				}, headers);
		}


	public:
		explicit PacketTemplateWrapper(Headers... h)
		  : headers(std::make_tuple(h...))
		{
			PropagateOutwardFieldSetters();
		}

		explicit PacketTemplateWrapper(const std::tuple<Headers...> & h)
		  : headers(h)
		{
			PropagateOutwardFieldSetters();
		}

		template <typename... Fields>
		SelfType With(Fields... f) const
		{
			return SelfType(std::tuple_cat(Head(headers),
			    std::make_tuple(Apply(Tail(headers), f...))));
		}

		template <LayerVal Layer, int Nesting>
		auto WithHeader(LayerImpl<Layer, Nesting> l) const
		{
			return FieldsImpl<FindLayer<Layer, Nesting>()>(headers);
		}

		MbufPtr Generate() const
		{
			const auto & outer = std::get<0>(headers);
			int len = outer.GetLen() + outer.GetPayloadLength();
			MbufPtr m(alloc_mbuf(len));

			m->m_pkthdr.len = len;
			m->m_len = len;

			std::apply( [&m] (const auto &... header)
				{
					size_t offset = 0;
					((header.FillPacket(m.get(), offset), offset += header.GetLen()), ...);
				},
				headers);

			return m;
		}

		struct mbuf * GenerateRawMbuf() const
		{
			return Generate().release();
		}

		SelfType Next() const
		{
			return std::apply([] (const auto &... header)
				{
					return SelfType(header.Next()...);
				},
				headers);
		}

		SelfType Retransmission() const
		{
			return std::apply([] (const auto &... header)
				{
					return SelfType(header.Retransmission()...);
				},
				headers);
		}

		const std::tuple<Headers...> & Unwrap() const
		{
			return headers;
		}

		void Print() const
		{
			std::apply([] (const auto &... header)
				{
					((header.print(0)) , ...);
				},
				headers);
		}
	};

	template <typename... Headers>
	auto PacketTemplate(Headers... rest)
	{
		return PacketTemplateWrapper(std::tuple_cat(std::tuple(rest.Unwrap())...));
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
