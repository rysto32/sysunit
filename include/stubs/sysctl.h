
#ifndef STUBS_SYSCTL_H
#define STUBS_SYSCTL_H

#include <sys/cdefs.h>
#include <sys/types.h>
#define _KERNEL 1

extern "C" {
#define new newp
#include <kern_include/sys/sysctl.h>
#undef new
}

#undef _KERNEL
#include <stdint.h>

extern "C" {

struct sysctl_oid;
struct sysctl_req;

struct sysctl_oid_list sysctl__children;

int 
sysctl_handle_int(struct sysctl_oid *oidp, void *arg1,
	intmax_t arg2, struct sysctl_req *req)
{
	return (0);
}

int 
sysctl_handle_long(struct sysctl_oid *oidp, void *arg1,
	intmax_t arg2, struct sysctl_req *req)
{
	return (0);
}

int 
sysctl_handle_string(struct sysctl_oid *oidp, void *arg1,
	intmax_t arg2, struct sysctl_req *req)
{
	return (0);
}

}

#endif
