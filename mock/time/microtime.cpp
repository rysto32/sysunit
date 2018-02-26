

extern "C" {

#include <kern_include/sys/time.h>
}

#include "mock/time.h"

template <>
typename GlobalMock<MockTime>::Initializer GlobalMock<MockTime>::initializer(0);

extern "C" void
getmicrotime(struct timeval *tvp)
{
	MockTime::MockObj().getmicrotime(tvp);
}

