
#define _KERNEL 1
#define UT_FRIENDLY 1

extern "C" {
#include <sys/types.h>
#include <kern_include/sys/systm.h>
#include <kern_include/sys/libkern.h>
}

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

void
kassert_panic(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vpanic(fmt, ap);
}

void
panic(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vpanic(fmt, ap);
}

void
vpanic(const char *fmt, va_list ap)
{
	vfprintf(stderr, fmt, ap);
	abort();
}