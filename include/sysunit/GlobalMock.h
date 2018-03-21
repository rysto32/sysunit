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

#ifndef SYSUNIT_GLOBAL_MOCK_H
#define SYSUNIT_GLOBAL_MOCK_H

#include "sysunit/Initializer.h"

#include <gmock/gmock.h>

#include <memory>

namespace SysUnit::internal {
template <typename Mock>
class GlobalMockInitializer : public SysUnit::Initializer
{
public:
	// I don't know why, but if this constructor doesn't take
	// a dummy int argument it is never called at all.
	GlobalMockInitializer(int)
	{
	}

	void SetUp() override
	{
		Mock::SetUp();
	}

	void TearDown() override
	{
		Mock::TearDown();
	}
};
}

namespace SysUnit
{
template <typename Derived>
class GlobalMock
{
private:
	typedef std::unique_ptr<testing::StrictMock<Derived>> MockPtr;
	static MockPtr mockobj;

	static void SetUp()
	{
		mockobj = std::make_unique<testing::StrictMock<Derived>>();
	}

	static void TearDown()
	{
		mockobj.reset();
	}

	typedef internal::GlobalMockInitializer<Derived> Initializer;
	friend Initializer;

	static Initializer initializer;

public:

	static auto & MockObj()
	{
		if (!mockobj)
			throw std::runtime_error(std::string("Mock ") + (typeid(Derived).name()) + " was not initialized.");
		return *mockobj;
	}

};

template <typename T>
typename GlobalMock<T>::MockPtr GlobalMock<T>::mockobj;
}

#endif
