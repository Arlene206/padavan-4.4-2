#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "../firewall_rp_filter_policy.h"

static void
test_successful_write_is_authoritative(void)
{
	assert(firewall_rp_filter_write_status(0, 0, 0, EIO) == 0);
	assert(firewall_rp_filter_write_status(1, 0, 0, 0) == 0);
}

static void
test_absent_interface_only_tolerates_restrictive_default(void)
{
	assert(firewall_rp_filter_write_status(0, 1, -1, ENOENT) == 0);
	assert(firewall_rp_filter_write_status(0, 1, -1, 0) == 0);
	assert(firewall_rp_filter_write_status(0, 0, -1, ENOENT) == ENODEV);
}

static void
test_present_interface_write_failures_propagate(void)
{
	assert(firewall_rp_filter_write_status(1, 0, EACCES, 0) == EACCES);
	assert(firewall_rp_filter_write_status(1, 1, -1, EIO) == EIO);
	assert(firewall_rp_filter_write_status(1, 0, -1, 0) == EIO);
}

int
main(void)
{
	test_successful_write_is_authoritative();
	test_absent_interface_only_tolerates_restrictive_default();
	test_present_interface_write_failures_propagate();
	puts("Firewall rp_filter sysctl status policy: PASS");
	return 0;
}
