#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "miniupnpd_event_subscription_policy.h"
#include "miniupnpd_event_timeout_policy.h"

static void
expect_invalid_timeout(const char *value)
{
	int timeout = 77;
	int status;

	errno = 0;
	status = miniupnpd_event_timeout_parse(value,
	                                      value ? strlen(value) : 0U,
	                                      &timeout);
	assert(status == -1);
	assert(errno == EINVAL || errno == ERANGE);
	assert(timeout == 77);
}

static void
test_subscriber_capacity_boundaries(void)
{
	assert(miniupnpd_event_subscriber_has_capacity(0));
	assert(miniupnpd_event_subscriber_has_capacity(1));
	assert(miniupnpd_event_subscriber_has_capacity(
		MINIUPNPD_EVENT_MAX_SUBSCRIBERS - 1U));
	assert(!miniupnpd_event_subscriber_has_capacity(
		MINIUPNPD_EVENT_MAX_SUBSCRIBERS));
	assert(!miniupnpd_event_subscriber_has_capacity(
		MINIUPNPD_EVENT_MAX_SUBSCRIBERS + 1U));
}

static void
test_timeout_parse(void)
{
	int timeout = -1;
	static const char maximum[] = "Second-2147483647";
	static const char spaced[] = " \tSecond-30\t ";

	assert(miniupnpd_event_timeout_parse("Second-0", 8U, &timeout) == 0);
	assert(timeout == 0);
	assert(miniupnpd_event_timeout_parse("Second-1800", 11U, &timeout) == 0);
	assert(timeout == 1800);
	assert(miniupnpd_event_timeout_parse("sEcOnD-InFiNiTe", 15U,
	                                     &timeout) == 0);
	assert(timeout == 0);
	assert(miniupnpd_event_timeout_parse(maximum, sizeof(maximum) - 1U,
	                                     &timeout) == 0);
	assert(timeout == INT_MAX);
	assert(miniupnpd_event_timeout_parse(spaced, sizeof(spaced) - 1U,
	                                     &timeout) == 0);
	assert(timeout == 30);

	expect_invalid_timeout(NULL);
	expect_invalid_timeout("");
	expect_invalid_timeout("Second-");
	expect_invalid_timeout("Seconds-30");
	expect_invalid_timeout("Second--1");
	expect_invalid_timeout("Second-+1");
	expect_invalid_timeout("Second-1junk");
	expect_invalid_timeout("Second-2147483648");
	expect_invalid_timeout("Second-infinitex");
	expect_invalid_timeout("Second-1 2");
	expect_invalid_timeout("Second- 1");
	expect_invalid_timeout("Infinite");
	expect_invalid_timeout("Second-5\r");
}

static void
test_timeout_deadline(void)
{
	time_t deadline = (time_t)-1;
	time_t maximum = miniupnpd_event_time_t_max();

	assert(miniupnpd_event_timeout_deadline_for_limit(
		(time_t)100, 300, (time_t)1000, &deadline) == 0);
	assert(deadline == (time_t)400);
	assert(miniupnpd_event_timeout_deadline_for_limit(
		(time_t)999, 2, (time_t)1000, &deadline) == 0);
	assert(deadline == (time_t)1000);
	assert(miniupnpd_event_timeout_deadline_for_limit(
		(time_t)100, 0, (time_t)1000, &deadline) == 0);
	assert(deadline == (time_t)0);

	deadline = (time_t)777;
	errno = 0;
	assert(miniupnpd_event_timeout_deadline_for_limit(
		(time_t)100, -1, (time_t)1000, &deadline) == -1);
	assert(errno == EINVAL);
	assert(deadline == (time_t)777);
	errno = 0;
	assert(miniupnpd_event_timeout_deadline_for_limit(
		(time_t)-1, 1, (time_t)1000, &deadline) == -1);
	assert(errno == ERANGE);
	assert(deadline == (time_t)777);
	errno = 0;
	assert(miniupnpd_event_timeout_deadline_for_limit(
		(time_t)1001, 1, (time_t)1000, &deadline) == -1);
	assert(errno == ERANGE);
	assert(deadline == (time_t)777);
	errno = 0;
	assert(miniupnpd_event_timeout_deadline_for_limit(
		(time_t)100, 1, (time_t)0, &deadline) == -1);
	assert(errno == ERANGE);
	assert(deadline == (time_t)777);

	assert(maximum > (time_t)1);
	assert(miniupnpd_event_timeout_deadline_for_limit(
		maximum - (time_t)1, 10, maximum, &deadline) == 0);
	assert(deadline == maximum);
	assert(miniupnpd_event_timeout_deadline((time_t)100, 60,
	                                        &deadline) == 0);
	assert(deadline == (time_t)160);
}

int
main(void)
{
	test_subscriber_capacity_boundaries();
	test_timeout_parse();
	test_timeout_deadline();
	return 0;
}
