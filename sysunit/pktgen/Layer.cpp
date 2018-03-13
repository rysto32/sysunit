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

#include "pktgen/Layer.h"

#include <sstream>
#include <string>

namespace PktGen::internal
{
	std::string MakeLayerName(LayerVal l, int nesting)
	{
		std::string shortName;

		switch (l) {
		case LayerVal::L2:
			shortName = "L2";
			break;
		case LayerVal::L3:
			shortName = "L3";
			break;
		case LayerVal::L4:
			shortName = "L4";
			break;
		case LayerVal::PAYLOAD:
			shortName = "PAYLOAD";
			break;
		}

		if (nesting == 1)
			return shortName;

		std::ostringstream str;
		str << shortName << "<" << nesting << ">";
		return str.str();
	}
}

namespace PktGen::Layer
{
	const internal::LayerImpl<internal::LayerVal::L2, 1> L2;
	const internal::LayerImpl<internal::LayerVal::L3, 1> L3;
	const internal::LayerImpl<internal::LayerVal::L4, 1> L4;
	const internal::LayerImpl<internal::LayerVal::PAYLOAD, 1> PAYLOAD;

	const internal::LayerImpl<internal::LayerVal::L2, 1> OUTER_L2;
	const internal::LayerImpl<internal::LayerVal::L3, 1> OUTER_L3;
	const internal::LayerImpl<internal::LayerVal::L4, 1> OUTER_L4;

	const internal::LayerImpl<internal::LayerVal::L2, -1> INNER_L2;
	const internal::LayerImpl<internal::LayerVal::L3, -1> INNER_L3;
	const internal::LayerImpl<internal::LayerVal::L4, -1> INNER_L4;
}
