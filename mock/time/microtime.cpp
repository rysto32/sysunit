

extern "C" {

#include <kern_include/sys/time.h>
}

#include "mock/time.h"

namespace SysUnit
{

template <>
typename GlobalMock<MockTime>::Initializer GlobalMock<MockTime>::initializer(0);

}

extern "C" void
getmicrotime(struct timeval *tvp)
{
	SysUnit::MockTime::MockObj().getmicrotime(tvp);
}

