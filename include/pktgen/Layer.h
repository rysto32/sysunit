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

namespace PktGen::internal
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
}

namespace PktGen
{
	namespace NestedLayer
	{
		template <int Nesting>
		const PktGen::internal::LayerImpl<PktGen::internal::LayerVal::L2, Nesting> L2;

		template <int Nesting>
		const PktGen::internal::LayerImpl<PktGen::internal::LayerVal::L3, Nesting> L3;

		template <int Nesting>
		const PktGen::internal::LayerImpl<PktGen::internal::LayerVal::L4, Nesting> L4;

		template <int Nesting>
		const PktGen::internal::LayerImpl<PktGen::internal::LayerVal::PAYLOAD, Nesting> PAYLOAD;
	}

	namespace Layer
	{
		extern const PktGen::internal::LayerImpl<PktGen::internal::LayerVal::L2, 1> L2;
		extern const PktGen::internal::LayerImpl<PktGen::internal::LayerVal::L3, 1> L3;
		extern const PktGen::internal::LayerImpl<PktGen::internal::LayerVal::L4, 1> L4;
		extern const PktGen::internal::LayerImpl<PktGen::internal::LayerVal::PAYLOAD, 1> PAYLOAD;

		extern const PktGen::internal::LayerImpl<PktGen::internal::LayerVal::L2, 1> OUTER_L2;
		extern const PktGen::internal::LayerImpl<PktGen::internal::LayerVal::L3, 1> OUTER_L3;
		extern const PktGen::internal::LayerImpl<PktGen::internal::LayerVal::L4, 1> OUTER_L4;

		extern const PktGen::internal::LayerImpl<PktGen::internal::LayerVal::L2, -1> INNER_L2;
		extern const PktGen::internal::LayerImpl<PktGen::internal::LayerVal::L3, -1> INNER_L3;
		extern const PktGen::internal::LayerImpl<PktGen::internal::LayerVal::L4, -1> INNER_L4;
	}
}

void PrintIndent(int, const char *, ...);

#endif
