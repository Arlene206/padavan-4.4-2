#include "wireless_radio_policy.h"

#include <stdio.h>

static int
test_success_is_published(void)
{
	return !wireless_radio_status_success(0);
}

static int
test_command_and_restart_failures_are_not_published(void)
{
	return wireless_radio_status_success(-1) ||
	       wireless_radio_status_success(1) ||
	       wireless_radio_status_success(256);
}

int
main(void)
{
	if (test_success_is_published() ||
	    test_command_and_restart_failures_are_not_published()) {
		fprintf(stderr, "Wireless radio status policy: FAIL\n");
		return 1;
	}

	printf("Wireless radio state is published only after successful operations: PASS\n");
	return 0;
}
