#include "wireless_radio_policy.h"

#include <stdio.h>

static int
test_success_is_published(void)
{
	return !wireless_radio_status_success(0);
}

static int
test_interface_failures_are_not_published(void)
{
	return wireless_radio_status_success(-1) ||
	       wireless_radio_status_success(1) ||
	       wireless_radio_status_success(256);
}

static int
test_shutdown_failure_is_sticky(void)
{
	int all_stopped = 1;

	all_stopped = wireless_radio_shutdown_status(all_stopped, 1);
	if (!all_stopped)
		return 1;

	all_stopped = wireless_radio_shutdown_status(all_stopped, 0);
	if (all_stopped)
		return 1;

	/* A later successful stop must not erase an earlier failure. */
	all_stopped = wireless_radio_shutdown_status(all_stopped, 1);
	return all_stopped;
}

static int
test_incomplete_shutdown_is_reported(void)
{
	return wireless_radio_shutdown_result(1) != 0 ||
	       wireless_radio_shutdown_result(0) >= 0;
}

int
main(void)
{
	if (test_success_is_published() ||
	    test_interface_failures_are_not_published() ||
	    test_shutdown_failure_is_sticky() ||
	    test_incomplete_shutdown_is_reported()) {
		fprintf(stderr, "Wireless interface status policy: FAIL\n");
		return 1;
	}

	printf("Wireless interface state is published only after successful ifconfig: PASS\n");
	return 0;
}
