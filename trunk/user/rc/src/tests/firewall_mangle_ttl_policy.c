#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "../firewall_mangle_ttl_policy.h"

static void
test_snapshot_success_preserves_transaction_state(void)
{
	int failed = 0;

	assert(firewall_mangle_snapshot_failure_status(0, &failed) == 0);
	assert(failed == 0);
	failed = 1;
	assert(firewall_mangle_snapshot_failure_status(0, &failed) == 0);
	assert(failed == 1);
}

static void
test_snapshot_read_errors_abort_the_enclosing_transaction(void)
{
	int failed = 0;

	assert(firewall_mangle_snapshot_failure_status(EACCES, &failed) == EACCES);
	assert(failed == 1);
	failed = 0;
	assert(firewall_mangle_snapshot_failure_status(-1, &failed) == EIO);
	assert(failed == 1);
	assert(firewall_mangle_snapshot_failure_status(EIO, NULL) == EINVAL);
}

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
	test_snapshot_success_preserves_transaction_state();
	test_snapshot_read_errors_abort_the_enclosing_transaction();
	test_unconfigured_ttl_is_not_a_dependency();
	test_configured_ttl_requires_successful_write();
	puts("Firewall mangle TTL sysctl status policy: PASS");
	return 0;
}
