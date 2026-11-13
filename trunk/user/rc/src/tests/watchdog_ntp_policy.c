#include <errno.h>
#include <stdio.h>

#include "watchdog_ntp_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_successful_stop_allows_refresh(void)
{
	CHECK(watchdog_ntp_refresh_should_launch(0));
	return 0;
}

static int
test_incomplete_stop_blocks_refresh(void)
{
	CHECK(!watchdog_ntp_refresh_should_launch(-1));
	CHECK(!watchdog_ntp_refresh_should_launch(1));
	CHECK(!watchdog_ntp_refresh_should_launch(EBUSY));
	CHECK(!watchdog_ntp_refresh_should_launch(EIO));
	return 0;
}

int
main(void)
{
	CHECK(test_successful_stop_allows_refresh() == 0);
	CHECK(test_incomplete_stop_blocks_refresh() == 0);
	puts("watchdog NTP stop-before-start policy: PASS");
	return 0;
}
