

extern "C" {

#include <kern_include/sys/time.h>
}

#include "mock/timeval.h"

extern "C" void
getmicrotime(struct timeval *tvp)
{
	MockTimeval::MockObj().getmicrotime(tvp);
}

