


#define _KERNEL_UT 1

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/sysctl.h>

SYSCTL_ROOT_NODE(0,	  sysctl, CTLFLAG_RW, 0,
	"Sysctl internal magic");
SYSCTL_ROOT_NODE(CTL_KERN,	  kern,   CTLFLAG_RW|CTLFLAG_CAPRD, 0,
	"High kernel, proc, limits &c");
SYSCTL_ROOT_NODE(CTL_VM,	  vm,     CTLFLAG_RW, 0,
	"Virtual memory");
SYSCTL_ROOT_NODE(CTL_VFS,	  vfs,     CTLFLAG_RW, 0,
	"File system");
SYSCTL_ROOT_NODE(CTL_NET,	  net,    CTLFLAG_RW, 0,
	"Network, (see socket.h)");
SYSCTL_ROOT_NODE(CTL_DEBUG,  debug,  CTLFLAG_RW, 0,
	"Debugging");
SYSCTL_NODE(_debug, OID_AUTO,  sizeof,  CTLFLAG_RW, 0,
	"Sizeof various things");
SYSCTL_ROOT_NODE(CTL_HW,	  hw,     CTLFLAG_RW, 0,
	"hardware");
SYSCTL_ROOT_NODE(CTL_MACHDEP, machdep, CTLFLAG_RW, 0,
	"machine dependent");
SYSCTL_ROOT_NODE(CTL_USER,	  user,   CTLFLAG_RW, 0,
	"user-level");
SYSCTL_ROOT_NODE(CTL_P1003_1B,  p1003_1b,   CTLFLAG_RW, 0,
	"p1003_1b, (see p1003_1b.h)");

SYSCTL_ROOT_NODE(OID_AUTO,  compat, CTLFLAG_RW, 0,
	"Compatibility code");
SYSCTL_ROOT_NODE(OID_AUTO, security, CTLFLAG_RW, 0, 
     	"Security");
#ifdef REGRESSION
SYSCTL_ROOT_NODE(OID_AUTO, regression, CTLFLAG_RW, 0,
     "Regression test MIB");
#endif

#ifdef EXT_RESOURCES
SYSCTL_ROOT_NODE(OID_AUTO, clock, CTLFLAG_RW, 0,
	"Clocks");
#endif

SYSCTL_NODE(_net, OID_AUTO,  inet,  CTLFLAG_RD, 0, "");
SYSCTL_NODE(_net_inet, OID_AUTO,  tcp,  CTLFLAG_RD, 0, "");
SYSCTL_NODE(_kern, OID_AUTO,  ipc,  CTLFLAG_RD, 0, "");
