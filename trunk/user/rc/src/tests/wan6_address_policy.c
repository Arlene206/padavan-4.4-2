#include <arpa/inet.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "wan6_address_policy.h"

static int
is_usable(const char *text)
{
	struct in6_addr address;

	if (!text || inet_pton(AF_INET6, text, &address) != 1)
		return 0;
	return wan6_address_is_usable_unicast(&address);
}

int
main(void)
{
	assert(!wan6_address_is_usable_unicast(NULL));
	assert(is_usable("2001:db8::1"));
	assert(is_usable("fd12:3456::1"));
	assert(is_usable("fe80::1"));
	assert(!is_usable("::"));
	assert(!is_usable("::1"));
	assert(!is_usable("ff02::1"));
	assert(!is_usable("not-an-ipv6-address"));
	return 0;
}
