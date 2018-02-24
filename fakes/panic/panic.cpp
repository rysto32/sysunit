
#define _KERNEL_UT 1

extern "C" {
#include <kern_include/sys/types.h>
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
	fprintf(stderr, "panic: ");
	vfprintf(stderr, fmt, ap);
	abort();
}