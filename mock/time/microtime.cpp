

extern "C" {

#include <kern_include/sys/time.h>
}

#include "mock/time.h"

extern "C" void
getmicrotime(struct timeval *tvp)
{
	MockTime::MockObj().getmicrotime(tvp);
}

