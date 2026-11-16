#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "modem_wan_start_policy.h"
#include "service_start_status.h"

static void
test_miniupnp_migration_failure_blocks_dhcp(void)
{
	assert(!modem_wan_start_migration_succeeded(EBUSY));
	assert(modem_wan_start_status(EBUSY, 0) == EBUSY);
	assert(modem_wan_start_status(EBUSY, EIO) == EBUSY);
	assert(!modem_wan_start_migration_succeeded(-1));
	assert(modem_wan_start_status(-1, 0) == -1);
}

static void
test_dhcp_launch_result_is_returned_after_migration(void)
{
	assert(modem_wan_start_migration_succeeded(0));
	assert(modem_wan_start_status(0, 0) == 0);
	assert(modem_wan_start_status(0, EIO) == EIO);
	assert(modem_wan_start_status(0, 23) == 23);
	assert(modem_wan_start_status(0, -1) == -1);
}

static void
test_first_modem_error_survives_best_effort_followup(void)
{
	int status = 0;
	int dhcp_attempts = 0;
	int passthrough_attempts = 0;
	int migration_status = ENOENT;

	if (modem_wan_start_migration_succeeded(migration_status))
		dhcp_attempts++;
	status = rc_first_service_start_status(
		status, modem_wan_start_status(migration_status, 0));
	passthrough_attempts++;
	status = rc_first_service_start_status(status, 0);

	assert(dhcp_attempts == 0);
	assert(passthrough_attempts == 1);
	assert(status == ENOENT);
}

int
main(void)
{
	test_miniupnp_migration_failure_blocks_dhcp();
	test_dhcp_launch_result_is_returned_after_migration();
	test_first_modem_error_survives_best_effort_followup();
	puts("USB modem WAN startup status policy: PASS");
	return 0;
}
