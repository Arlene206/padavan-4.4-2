#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "service_wait_policy.h"

struct nanosleep_fixture {
	unsigned int calls;
	struct timespec requested[3];
	struct timespec remaining[3];
	int errors[3];
};

static int
fixture_nanosleep(const struct timespec *requested,
		  struct timespec *remaining, void *opaque)
{
	struct nanosleep_fixture *fixture = (struct nanosleep_fixture *)opaque;
	unsigned int index = fixture->calls++;

	assert(index < 3);
	fixture->requested[index] = *requested;
	*remaining = fixture->remaining[index];
	if (fixture->errors[index] != 0) {
		errno = fixture->errors[index];
		return -1;
	}
	return 0;
}

int
main(void)
{
	struct nanosleep_fixture fixture;
	struct timespec invalid;

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

	puts("EINTR-safe nanosleep remainder, validation and error propagation: PASS");
	return 0;
}
