#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "../firewall_mangle_ttl_policy.h"

static void
test_unconfigured_ttl_is_not_a_dependency(void)
{
	assert(firewall_mangle_ttl_write_status(0, -1, EACCES) == 0);
	assert(firewall_mangle_ttl_write_status(1, EIO, EIO) == 0);
}

static void
test_configured_ttl_requires_successful_write(void)
{
	assert(firewall_mangle_ttl_write_status(2, 0, 0) == 0);
	assert(firewall_mangle_ttl_write_status(64, EACCES, 0) == EACCES);
	assert(firewall_mangle_ttl_write_status(255, -1, ENOENT) == ENOENT);
	assert(firewall_mangle_ttl_write_status(32, -1, 0) == EIO);
}

int
main(void)
{
	test_unconfigured_ttl_is_not_a_dependency();
	test_configured_ttl_requires_successful_write();
	puts("Firewall mangle TTL sysctl status policy: PASS");
	return 0;
}
