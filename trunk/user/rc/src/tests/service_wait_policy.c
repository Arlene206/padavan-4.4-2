#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "service_wait_policy.h"

struct sleep_fixture {
	unsigned int calls;
	unsigned int requested[SERVICE_WAIT_MAX_INTERRUPTS + 1U];
	unsigned int remaining[SERVICE_WAIT_MAX_INTERRUPTS + 1U];
};

static unsigned int
fixture_sleep(unsigned int seconds, void *opaque)
{
	struct sleep_fixture *fixture = (struct sleep_fixture *)opaque;
	unsigned int index = fixture->calls++;

	assert(index < SERVICE_WAIT_MAX_INTERRUPTS + 1U);
	fixture->requested[index] = seconds;
	return fixture->remaining[index];
}

struct nanosleep_fixture {
	unsigned int calls;
	struct timespec requested[SERVICE_WAIT_MAX_INTERRUPTS + 1U];
	struct timespec remaining[SERVICE_WAIT_MAX_INTERRUPTS + 1U];
	int errors[SERVICE_WAIT_MAX_INTERRUPTS + 1U];
	int fail_without_errno[SERVICE_WAIT_MAX_INTERRUPTS + 1U];
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
	if (fixture->fail_without_errno[index])
		return -1;
	return 0;
}

static void
test_sleep_eintr_retries_use_unslept_remainder(void)
{
	struct sleep_fixture fixture;

	memset(&fixture, 0, sizeof(fixture));
	fixture.remaining[0] = 2U;
	fixture.remaining[1] = 1U;
	fixture.remaining[2] = 0U;
	assert(service_wait_seconds_with(3U, fixture_sleep, &fixture) == 0);
	assert(fixture.calls == 3U);
	assert(fixture.requested[0] == 3U);
	assert(fixture.requested[1] == 2U);
	assert(fixture.requested[2] == 1U);
}

static void
test_repeated_sleep_eintr_is_bounded(void)
{
	struct sleep_fixture sleep_fixture;
	struct nanosleep_fixture fixture;
	struct timespec requested;
	unsigned int i;

	memset(&sleep_fixture, 0, sizeof(sleep_fixture));
	for (i = 0; i < SERVICE_WAIT_MAX_INTERRUPTS + 1U; i++)
		sleep_fixture.remaining[i] = 1;
	errno = 0;
	assert(service_wait_seconds_with(1, fixture_sleep, &sleep_fixture) == -1);
	assert(errno == EINTR);
	assert(sleep_fixture.calls == SERVICE_WAIT_MAX_INTERRUPTS + 1U);
	for (i = 0; i < sleep_fixture.calls; i++)
		assert(sleep_fixture.requested[i] == 1);

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

	/* A failed nanosleep shim that violates errno reporting fails as EIO. */
	memset(&fixture, 0, sizeof(fixture));
	fixture.fail_without_errno[0] = 1;
	errno = EINTR;
	assert(service_wait_microseconds_with(
		100U, fixture_nanosleep, &fixture) == -1);
	assert(errno == EIO && fixture.calls == 1);

	invalid.tv_sec = 0;
	invalid.tv_nsec = 1000000000L;
	errno = 0;
	assert(service_wait_timespec_with(
		invalid, fixture_nanosleep, &fixture) == -1);
	assert(errno == EINVAL);

	errno = 0;
	assert(service_wait_microseconds_with(100U, NULL, NULL) == -1);
	assert(errno == EINVAL);

	test_sleep_eintr_retries_use_unslept_remainder();
	test_repeated_sleep_eintr_is_bounded();
	puts("Service waits preserve EINTR remainders and bound repeated interruption: PASS");
	return 0;
}
