#include <errno.h>
#include <stdio.h>

#include "detect_link_modem_reconnect_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_failed_notifications_schedule_bounded_retries(void)
{
	unsigned int retries = 0;
	unsigned long next_tick = 0;

	CHECK(detect_link_modem_reconnect_retry_status(EIO, &retries, 100,
							&next_tick) == 1);
	CHECK(retries == 1);
	CHECK(next_tick == 102);

	CHECK(detect_link_modem_reconnect_retry_status(ESRCH, &retries, 200,
							&next_tick) == 1);
	CHECK(retries == 2);
	CHECK(next_tick == 202);

	CHECK(detect_link_modem_reconnect_retry_status(EIO, &retries, 300,
							&next_tick) == -1);
	CHECK(retries == 0);
	CHECK(next_tick == 0);
	return 0;
}

static int
test_success_and_not_needed_state_clear_pending_retry(void)
{
	unsigned int retries = 2;
	unsigned long next_tick = 402;

	CHECK(detect_link_modem_reconnect_retry_status(0, &retries, 500,
							&next_tick) == 0);
	CHECK(retries == 0);
	CHECK(next_tick == 0);
	return 0;
}

static int
test_invalid_outputs_fail_closed_without_partial_mutation(void)
{
	unsigned int retries = 1;
	unsigned long next_tick = 17;

	CHECK(detect_link_modem_reconnect_retry_status(EIO, NULL, 10,
							&next_tick) == -1);
	CHECK(next_tick == 17);
	CHECK(detect_link_modem_reconnect_retry_status(EIO, &retries, 10,
							NULL) == -1);
	CHECK(retries == 1);
	CHECK(next_tick == 17);
	return 0;
}

int
main(void)
{
	CHECK(test_failed_notifications_schedule_bounded_retries() == 0);
	CHECK(test_success_and_not_needed_state_clear_pending_retry() == 0);
	CHECK(test_invalid_outputs_fail_closed_without_partial_mutation() == 0);
	puts("detect_link bounded USB modem reconnect retry policy: PASS");
	return 0;
}
