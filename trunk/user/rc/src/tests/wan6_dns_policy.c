#include <assert.h>
#include <stdio.h>

#include "wan6_dns_policy.h"

static void
test_withdrawn_dns_option_clears_previous_lease(void)
{
	assert(wan6_dns_update_required("2001:db8::53", NULL));
	assert(wan6_dns_update_required("2001:db8::53", ""));
}

static void
test_absent_dns_option_is_noop_when_already_empty(void)
{
	assert(!wan6_dns_update_required(NULL, NULL));
	assert(!wan6_dns_update_required("", NULL));
}

static void
test_changed_dns_list_is_published(void)
{
	assert(wan6_dns_update_required("2001:db8::53", "2001:db8::54"));
}

static void
test_unchanged_dns_list_is_not_republished(void)
{
	assert(!wan6_dns_update_required("2001:db8::53", "2001:db8::53"));
}

int
main(void)
{
	test_withdrawn_dns_option_clears_previous_lease();
	test_absent_dns_option_is_noop_when_already_empty();
	test_changed_dns_list_is_published();
	test_unchanged_dns_list_is_not_republished();
	puts("WAN DHCPv6 DNS withdrawal policy regression: PASS");
	return 0;
}
