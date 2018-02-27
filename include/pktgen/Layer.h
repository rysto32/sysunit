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

#ifndef PKTGEN_LAYER_H
#define PKTGEN_LAYER_H

#include <string>

namespace PktGen
{
	enum class LayerVal
	{
		L2,
		L3,
		L4,
		PAYLOAD
	};

	std::string MakeLayerName(LayerVal l, int n);

	template <LayerVal Layer, int Nesting>
	struct LayerImpl
	{
		static const LayerVal LAYER = Layer;
		static const int NESTING = Nesting;

		static std::string Name()
		{
			return MakeLayerName(Layer, Nesting);
		}
	};

	namespace NestedLayer
	{

		template <int Nesting>
		struct L2
		{
			typedef LayerImpl<LayerVal::L2, Nesting> IMPL;
		};

		template <int Nesting>
		struct L3
		{
			typedef LayerImpl<LayerVal::L3, Nesting> IMPL;
		};

		template <int Nesting>
		struct L4
		{
			typedef LayerImpl<LayerVal::L4, Nesting> IMPL;
		};

		template <int Nesting>
		struct PAYLOAD
		{
			typedef LayerImpl<LayerVal::PAYLOAD, Nesting> IMPL;
		};
	}

	namespace Layer
	{
		typedef NestedLayer::L2<1> L2;
		typedef NestedLayer::L3<1> L3;
		typedef NestedLayer::L4<1> L4;
		typedef NestedLayer::PAYLOAD<1> PAYLOAD;

		typedef NestedLayer::L2<1> OUTER_L2;
		typedef NestedLayer::L3<1> OUTER_L3;
		typedef NestedLayer::L4<1> OUTER_L4;

		typedef NestedLayer::L2<-1> INNER_L2;
		typedef NestedLayer::L3<-1> INNER_L3;
		typedef NestedLayer::L4<-1> INNER_L4;
	};

	template <int l2Nest = 0, int l3Nest = 0, int l4Nest = 0, int payloadNest = 0>
	struct CurrentNesting
	{
		typedef typename NestedLayer::L2<l2Nest>::IMPL L2;
		typedef typename NestedLayer::L3<l3Nest>::IMPL L3;
		typedef typename NestedLayer::L4<l4Nest>::IMPL L4;
		typedef typename NestedLayer::PAYLOAD<payloadNest>::IMPL PAYLOAD;

		static constexpr int Depth(LayerVal l)
		{
			switch (l) {
			case LayerVal::L2:
				return l2Nest;
			case LayerVal::L3:
				return l3Nest;
			case LayerVal::L4:
				return l4Nest;
			case LayerVal::PAYLOAD:
				return payloadNest;
			default:
				return -1;
			}
		}

		template <LayerVal layer, int depth>
		static constexpr int ConvertInnerDepth()
		{
			return depth > 0 ? depth : Depth(layer) + depth + 1;
		}

		template <typename Layer>
		static constexpr int ConvertInnerDepth()
		{
			return ConvertInnerDepth<Layer::IMPL::LAYER, Layer::IMPL::NESTING>();
		}

		typedef CurrentNesting<l2Nest + 1, l3Nest, l4Nest, payloadNest> NextL2;
		typedef CurrentNesting<l2Nest, l3Nest + 1, l4Nest, payloadNest> NextL3;
		typedef CurrentNesting<l2Nest, l3Nest, l4Nest + 1, payloadNest> NextL4;
		typedef CurrentNesting<l2Nest, l3Nest, l4Nest, payloadNest + 1> NextPayload;
	};

	typedef CurrentNesting<> DefaultNestingLevel;
}

void PrintIndent(int, const char *, ...);

#endif
