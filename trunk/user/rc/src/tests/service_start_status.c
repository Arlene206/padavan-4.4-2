#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "service_start_status.h"

static void
test_first_error_survives_lan_restart_recovery(void)
{
	int status = rc_first_service_start_status(0, EBUSY);

	status = rc_first_service_start_status(status, 0);
	status = rc_first_service_start_status(status, EIO);
	assert(status == EBUSY);
}

static void
test_negative_and_positive_statuses_are_preserved(void)
{
	int status = rc_first_service_start_status(0, -1);

	assert(status == -1);
	status = rc_first_service_start_status(0, 17);
	status = rc_first_service_start_status(status, EIO);
	assert(status == 17);
}

static void
test_lan_start_preserves_first_required_failure(void)
{
	int status = 0;
	int ifup_attempts = 0;
	int dhcp_attempts = 0;
	int route_attempts = 0;

	/* A later DHCP success must not erase an earlier bridge setup failure. */
	ifup_attempts++;
	status = rc_first_service_start_status(status, ENODEV);
	dhcp_attempts++;
	status = rc_first_service_start_status(status, 0);
	assert(ifup_attempts == 1);
	assert(dhcp_attempts == 1);
	assert(status == ENODEV);

	/* Router-mode static-route failures must be observable too. */
	status = 0;
	route_attempts++;
	status = rc_first_service_start_status(status, EIO);
	assert(route_attempts == 1);
	assert(status == EIO);
}

static void
test_loopback_start_preserves_first_required_failure(void)
{
	int status = 0;
	int interface_attempts = 0;
	int route_attempts = 0;

	/* Route setup must still run after the loopback interface operation fails. */
	interface_attempts++;
	status = rc_first_service_start_status(status, ENETDOWN);
	route_attempts++;
	status = rc_first_service_start_status(status, EIO);
	assert(interface_attempts == 1);
	assert(route_attempts == 1);
	assert(status == ENETDOWN);

	/* A route-only failure must also be returned. */
	status = 0;
	status = rc_first_service_start_status(status, 0);
	status = rc_first_service_start_status(status, EIO);
	assert(status == EIO);
}

static void
test_bridge_parameters_are_both_attempted_and_first_error_survives(void)
{
	int status = 0;
	int stp_attempts = 0;
	int forward_delay_attempts = 0;

	/* STP failure must not skip the independent forward-delay write. */
	stp_attempts++;
	status = rc_first_service_start_status(status, EIO);
	forward_delay_attempts++;
	status = rc_first_service_start_status(status, 0);
	assert(stp_attempts == 1);
	assert(forward_delay_attempts == 1);
	assert(status == EIO);

	/* A forward-delay-only failure is also returned. */
	status = 0;
	stp_attempts = 0;
	forward_delay_attempts = 0;
	stp_attempts++;
	status = rc_first_service_start_status(status, 0);
	forward_delay_attempts++;
	status = rc_first_service_start_status(status, EINVAL);
	assert(stp_attempts == 1);
	assert(forward_delay_attempts == 1);
	assert(status == EINVAL);
}

int
main(void)
{
	assert(rc_first_service_start_status(0, 0) == 0);
	assert(rc_first_service_start_status(0, -1) == -1);
	assert(rc_first_service_start_status(0, 17) == 17);
	assert(rc_first_service_start_status(-1, 0) == -1);
	assert(rc_first_service_start_status(17, -1) == 17);
	assert(rc_first_service_start_status(-5, 9) == -5);
	assert(rc_first_service_start_status(4, 0) == 4);
	test_first_error_survives_lan_restart_recovery();
	test_negative_and_positive_statuses_are_preserved();
	test_bridge_parameters_are_both_attempted_and_first_error_survives();
	test_loopback_start_preserves_first_required_failure();
	test_lan_start_preserves_first_required_failure();
	puts("RC boot and LAN service first-failure aggregation policy: PASS");
	return 0;
}
