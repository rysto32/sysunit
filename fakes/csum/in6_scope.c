
#include <sys/types.h>
#include <kern_include/netinet/in.h>
#include <kern_include/netinet/ip6.h>
#include <kern_include/netinet6/scope6_var.h>

/*
 * Return the scope identifier or zero.
 */
uint16_t
in6_getscope(struct in6_addr *in6)
{

	if (IN6_IS_SCOPE_LINKLOCAL(in6) || IN6_IS_ADDR_MC_INTFACELOCAL(in6))
		return (in6->s6_addr16[1]);

	return (0);
}
