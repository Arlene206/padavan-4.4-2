#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "service_wait_policy.h"

struct nanosleep_fixture {
	unsigned int calls;
	struct timespec requested[SERVICE_WAIT_MAX_INTERRUPTS + 1U];
	struct timespec remaining[SERVICE_WAIT_MAX_INTERRUPTS + 1U];
	int errors[SERVICE_WAIT_MAX_INTERRUPTS + 1U];
};

static int
fixture_nanosleep(const struct timespec *requested,
		  struct timespec *remaining, void *opaque)
{
	struct nanosleep_fixture *fixture = (struct nanosleep_fixture *)opaque;
	unsigned int index = fixture->calls++;

	assert(index < SERVICE_WAIT_MAX_INTERRUPTS + 1U);
	fixture->requested[index] = *requested;
	*remaining = fixture->remaining[index];
	if (fixture->errors[index] != 0) {
		errno = fixture->errors[index];
		return -1;
	}
	return 0;
}

static void
test_repeated_sleep_eintr_is_bounded(void)
{
	struct nanosleep_fixture fixture;
	struct timespec requested;
	unsigned int i;

	memset(&fixture, 0, sizeof(fixture));
	for (i = 0; i < SERVICE_WAIT_MAX_INTERRUPTS + 1U; i++)
		fixture.remaining[i].tv_sec = 1;
	errno = 0;
	assert(service_wait_seconds_with(1, fixture_sleep, &fixture) == -1);
	assert(errno == EINTR);
	assert(fixture.calls == SERVICE_WAIT_MAX_INTERRUPTS + 1U);

	memset(&fixture, 0, sizeof(fixture));
	for (i = 0; i < SERVICE_WAIT_MAX_INTERRUPTS + 1U; i++) {
		fixture.errors[i] = EINTR;
		fixture.remaining[i].tv_nsec = 400000000L;
	}
	requested.tv_sec = 0;
	requested.tv_nsec = 500000000L;
	errno = 0;
	assert(service_wait_timespec_with(
		requested, fixture_nanosleep, &fixture) == -1);
	assert(errno == EINTR);
	assert(fixture.calls == SERVICE_WAIT_MAX_INTERRUPTS + 1U);
}

int
main(void)
{
	struct nanosleep_fixture fixture;
	struct timespec invalid;

	errno = 0;
	assert(service_wait_failure_status(EACCES) == -1);
	assert(errno == EACCES);
	errno = 0;
	assert(service_wait_failure_status(0) == -1);
	assert(errno == EIO);
	errno = 0;
	assert(service_wait_failure_status(-1) == -1);
	assert(errno == EIO);

	memset(&fixture, 0, sizeof(fixture));
	fixture.errors[0] = EINTR;
	fixture.errors[1] = EINTR;
	fixture.remaining[0].tv_nsec = 300000000L;
	fixture.remaining[1].tv_nsec = 100000000L;
	assert(service_wait_microseconds_with(
		500000U, fixture_nanosleep, &fixture) == 0);
	assert(fixture.calls == 3);
	assert(fixture.requested[0].tv_sec == 0);
	assert(fixture.requested[0].tv_nsec == 500000000L);
	assert(fixture.requested[1].tv_nsec == 300000000L);
	assert(fixture.requested[2].tv_nsec == 100000000L);

	memset(&fixture, 0, sizeof(fixture));
	fixture.errors[0] = EINTR;
	fixture.remaining[0].tv_nsec = 200000L;
	errno = 0;
	assert(service_wait_microseconds_with(
		100U, fixture_nanosleep, &fixture) == -1);
	assert(errno == EIO && fixture.calls == 1);

	memset(&fixture, 0, sizeof(fixture));
	fixture.errors[0] = EACCES;
	errno = 0;
	assert(service_wait_microseconds_with(
		100U, fixture_nanosleep, &fixture) == -1);
	assert(errno == EACCES && fixture.calls == 1);

	invalid.tv_sec = 0;
	invalid.tv_nsec = 1000000000L;
	errno = 0;
	assert(service_wait_timespec_with(
		invalid, fixture_nanosleep, &fixture) == -1);
	assert(errno == EINVAL);

	errno = 0;
	assert(service_wait_microseconds_with(100U, NULL, NULL) == -1);
	assert(errno == EINVAL);

	test_repeated_sleep_eintr_is_bounded();
	puts("Service waits preserve EINTR remainders and bound repeated interruption: PASS");
	return 0;
}
