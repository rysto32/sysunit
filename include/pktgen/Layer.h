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
	namespace NestedLayer
	{
		std::string MakeName(const std::string & shortName, int nesting);

		template <int Nesting>
		struct L2
		{
			static std::string ShortName()
			{
				return "L2";
			}

			static std::string Name()
			{
				return MakeName(ShortName(), Nesting);
			}
		};

		template <int Nesting>
		struct L3
		{
			static std::string ShortName()
			{
				return "L3";
			}

			static std::string Name()
			{
				return MakeName(ShortName(), Nesting);
			}
		};

		template <int Nesting>
		struct L4
		{
			static std::string ShortName()
			{
				return "L4";
			}

			static std::string Name()
			{
				return MakeName(ShortName(), Nesting);
			}
		};

		template <int Nesting>
		struct PAYLOAD
		{
			static std::string ShortName()
			{
				return "PAYLOAD";
			}

			static std::string Name()
			{
				return MakeName(ShortName(), Nesting);
			}
		};
	}

	namespace Layer
	{
		typedef NestedLayer::L2<1> L2;
		typedef NestedLayer::L3<1> L3;
		typedef NestedLayer::L4<1> L4;
		typedef NestedLayer::PAYLOAD<1> PAYLOAD;
	};

	template <int l2Nest = 0, int l3Nest = 0, int l4Nest = 0, int payloadNest = 0>
	struct CurrentNesting
	{
		typedef NestedLayer::L2<l2Nest> L2;
		typedef NestedLayer::L3<l3Nest> L3;
		typedef NestedLayer::L4<l4Nest> L4;
		typedef NestedLayer::PAYLOAD<payloadNest> PAYLOAD;

		typedef CurrentNesting<l2Nest + 1, l3Nest, l4Nest, payloadNest> NextL2;
		typedef CurrentNesting<l2Nest, l3Nest + 1, l4Nest, payloadNest> NextL3;
		typedef CurrentNesting<l2Nest, l3Nest, l4Nest + 1, payloadNest> NextL4;
		typedef CurrentNesting<l2Nest, l3Nest, l4Nest, payloadNest + 1> NextPayload;
	};

	typedef CurrentNesting<> DefaultNestingLevel;
}

void PrintIndent(int, const char *, ...);

#endif
