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

int
main(void)
{
	if (test_success_is_published() ||
	    test_interface_failures_are_not_published()) {
		fprintf(stderr, "Wireless interface status policy: FAIL\n");
		return 1;
	}

	printf("Wireless interface state is published only after successful ifconfig: PASS\n");
	return 0;
}
