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
	puts("RC boot and LAN service first-failure aggregation policy: PASS");
	return 0;
}
