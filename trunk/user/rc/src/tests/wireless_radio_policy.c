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
test_interface_and_hw_nat_failures_preserve_first_error(void)
{
	return wireless_radio_status_merge(0, 0) != 0 ||
	       wireless_radio_status_merge(0, EIO) != EIO ||
	       wireless_radio_status_merge(0, 256) != 256 ||
	       wireless_radio_status_merge(-ENODEV, EIO) != -ENODEV ||
	       wireless_radio_status_merge(7, 0) != 7;
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

static int
test_startup_failure_is_sticky(void)
{
	int all_started = 1;

	all_started = wireless_radio_startup_status(all_started, 1);
	if (!all_started)
		return 1;

	all_started = wireless_radio_startup_status(all_started, 0);
	if (all_started)
		return 1;

	/* A later successful WDS interface must not erase an earlier failure. */
	all_started = wireless_radio_startup_status(all_started, 1);
	return all_started;
}

static int
test_incomplete_startup_is_reported(void)
{
	return wireless_radio_startup_result(1) != 0 ||
	       wireless_radio_startup_result(0) >= 0;
}

static int
test_restart_failure_wait_status_is_normalized_and_sticky(void)
{
	return wireless_radio_restart_status(0, 0) != 0 ||
	       wireless_radio_restart_status(0, -1) >= 0 ||
	       wireless_radio_restart_status(0, 1) >= 0 ||
	       wireless_radio_restart_status(0, 256) >= 0 ||
	       wireless_radio_restart_status(-EACCES, 0) != -EACCES ||
	       wireless_radio_restart_status(-EACCES, 256) != -EACCES;
}

static int
test_apcli_autoconnect_failure_is_reported(void)
{
	int status = 0;

	status = wireless_radio_restart_status(status, 0);
	status = wireless_radio_restart_status(status, 256);
	if (status != -EIO)
		return 1;

	/* A later successful command must not erase the first failure. */
	status = wireless_radio_restart_status(status, 0);
	return status != -EIO;
}

static int
test_apcli_forced_reenable_failure_is_reported(void)
{
	int status = 0;

	status = wireless_radio_restart_status(status, 0);
	status = wireless_radio_restart_status(status, -1);
	return status != -EIO;
}

static int
test_inic_monitor_launch_failure_is_sticky(void)
{
	int all_started = 1;

	all_started = wireless_radio_startup_status(
		all_started, wireless_radio_status_success(-EIO));
	if (all_started)
		return 1;

	/* A later successful interface must not erase the monitor failure. */
	all_started = wireless_radio_startup_status(
		all_started, wireless_radio_status_success(0));
	return all_started;
}

int
main(void)
{
	if (test_success_is_published() ||
	    test_interface_failures_are_not_published() ||
	    test_interface_and_hw_nat_failures_preserve_first_error() ||
	    test_shutdown_failure_is_sticky() ||
	    test_incomplete_shutdown_is_reported() ||
	    test_startup_failure_is_sticky() ||
	    test_incomplete_startup_is_reported() ||
	    test_restart_failure_wait_status_is_normalized_and_sticky() ||
	    test_apcli_autoconnect_failure_is_reported() ||
	    test_apcli_forced_reenable_failure_is_reported() ||
	    test_inic_monitor_launch_failure_is_sticky()) {
		fprintf(stderr, "Wireless interface status policy: FAIL\n");
		return 1;
	}

	printf("Wireless interface state is published only after successful ifconfig: PASS\n");
	return 0;
}
