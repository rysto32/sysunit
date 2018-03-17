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

#include "pktgen/FieldPropagator.h"
#include "pktgen/Layer.h"
#include "pktgen/MbufPtr.h"
#include "pktgen/PayloadLength.h"

#include <algorithm>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <tuple>

#include <iostream>

namespace PktGen::internal
{
	template <typename... Headers>
	class PacketTemplateWrapper
	{
	private:
		std::tuple<Headers...> headers;

		typedef PacketTemplateWrapper SelfType;

		struct WithHeaderImpl;

		template <std::size_t... Indices, typename... Types>
		static auto SliceFrontImpl(std::index_sequence<Indices...>, const std::tuple<Types...> & tuple)
		{
			return std::make_tuple(std::get<Indices>(tuple)...);
		}

		template <std::size_t index, typename... Types>
		static auto SliceFront(const std::tuple<Types...> & tuple)
		{
			return SliceFrontImpl(std::make_index_sequence<index>(), tuple);
		}

		template <std::size_t index, std::size_t... Indices, typename... Types>
		static auto SliceBackImpl(std::index_sequence<Indices...>, const std::tuple<Types...> & tuple)
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
		static auto Head(const std::tuple<Types...> & tuple)
		{
			return SliceFront<sizeof...(Types) - 1>(tuple);
		}

		template <typename... Types>
		static const auto & Tail(const std::tuple<Types...> & tuple)
		{
			return std::get<sizeof...(Types) - 1>(tuple);
		}

		template <typename T, typename... Mutators>
		static T Apply(const T & original, Mutators... mutators)
		{
			T copy(original);

			ApplyMutators(copy, mutators...);

			return std::move(copy);
		}

		template <typename T, typename Mutator, typename... Rest>
		static void ApplyMutators(T & header, const Mutator & m, Rest... rest)
		{
			m(header);
			ApplyMutators(header, rest...);
		}

		template <typename T>
		static void ApplyMutators(T & header)
		{
		}

		template <LayerVal Layer, int Nesting, typename Header, typename...Rest>
		static constexpr std::size_t
		FindLayerFromOuter()
		{
			if constexpr (Layer != Header::LAYER)
				return SelfType::FindLayerFromOuter<Layer, Nesting, Rest...>() + 1;
			else {
				static_assert(Nesting > 0);

				if constexpr (Nesting > 1)
					return SelfType::FindLayerFromOuter<Layer, Nesting - 1, Rest...>() + 1;
				else
					return 0;
			}

		}

		template <LayerVal Layer, typename Header, typename...Rest>
		static constexpr std::size_t
		CountLayers()
		{
			if constexpr (Layer == Header::LAYER)
				return SelfType::CountLayers<Layer, Rest...>() + 1;
			else
				return SelfType::CountLayers<Layer, Rest...>();
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

		template <typename First, typename Second, typename...Rest>
		static void PropagateInwards(First & first, Second & second, Rest &... rest)
		{
			DefaultInwardFieldSetter setter;
			setter(first, second);

			PropagateInwards(second, rest...);
		}

		template <typename First>
		static void PropagateInwards(First & first)
		{
		}

		void PropagateOutwardFieldSetters()
		{
			std::apply([] (auto &... headers)
				{
					PropagateOutwards(headers...);
				}, headers);
		}

		void PropagateInwardFieldSetters()
		{
			std::apply([] (auto &... headers)
				{
					PropagateInwards(headers...);
				}, headers);
		}

		void PropagateFields()
		{
			PropagateInwardFieldSetters();
			PropagateOutwardFieldSetters();
		}

		template <typename Header>
		static void FillFromHeader(struct mbuf *m, const Header & header,
		    size_t & offset)
		{
			header.FillPacket(m, offset);
			offset += header.GetLen();
		}

	public:
		explicit PacketTemplateWrapper(Headers... h)
		  : headers(std::make_tuple(h...))
		{
			PropagateFields();
		}

		explicit PacketTemplateWrapper(const std::tuple<Headers...> & h)
		  : headers(h)
		{
			PropagateFields();
		}

		template <typename... Fields>
		SelfType With(const Fields &... f) const
		{
			return SelfType(std::tuple_cat(Head(headers),
			    std::make_tuple(Apply(Tail(headers), f...))));
		}

		template <LayerVal Layer, int Nesting>
		auto WithHeader(const LayerImpl<Layer, Nesting> & l) const
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
					(FillFromHeader(m.get(), header, offset), ...);
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
}

namespace PktGen
{
	template <typename... Headers>
	auto PacketTemplate(Headers... rest)
	{
		return internal::PacketTemplateWrapper(std::tuple_cat(std::tuple(rest.Unwrap())...));
	}
}

#endif
