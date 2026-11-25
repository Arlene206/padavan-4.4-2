#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "wsd_stream_send_policy.h"

struct send_step {
	ssize_t result;
	int error;
};

struct send_fixture {
	const unsigned char *expected;
	size_t expected_length;
	size_t offset;
	const struct send_step *steps;
	size_t step_count;
	size_t calls;
};

static struct send_fixture *active_fixture;

static ssize_t
fake_send(int fd, const void *buffer, size_t length, int flags)
{
	struct send_fixture *fixture = active_fixture;
	const struct send_step *step;

	assert(fixture != NULL);
	assert(fd == 17);
	assert(flags == MSG_NOSIGNAL);
	assert(fixture->calls < fixture->step_count);
	assert(fixture->offset <= fixture->expected_length);
	assert(length == fixture->expected_length - fixture->offset);

	step = &fixture->steps[fixture->calls++];
	if (step->result < 0) {
		errno = step->error;
		return -1;
	}
	if (step->result == 0)
		return 0;

	assert((size_t)step->result <= length);
	assert(memcmp(buffer, fixture->expected + fixture->offset,
		      (size_t)step->result) == 0);
	fixture->offset += (size_t)step->result;
	return step->result;
}

static ssize_t
impossible_send(int fd, const void *buffer, size_t length, int flags)
{
	(void)fd;
	(void)buffer;
	(void)flags;
	return (ssize_t)length + 1;
}

static void
test_short_writes_and_eintr_send_every_byte(void)
{
	static const unsigned char payload[] = {
		'W', 'S', 'D', 0, 'r', 'e', 's', 'p', 'o', 'n', 's', 'e'
	};
	static const struct send_step steps[] = {
		{ 3, 0 }, { -1, EINTR }, { 4, 0 }, { 5, 0 }
	};
	struct send_fixture fixture;

	fixture.expected = payload;
	fixture.expected_length = sizeof(payload);
	fixture.offset = 0;
	fixture.steps = steps;
	fixture.step_count = sizeof(steps) / sizeof(steps[0]);
	fixture.calls = 0;
	active_fixture = &fixture;

	assert(wsd_stream_send_all(17, payload, sizeof(payload), fake_send) == 0);
	assert(fixture.offset == sizeof(payload));
	assert(fixture.calls == 4);
}

static void
test_permanent_error_preserves_successful_prefix_and_errno(void)
{
	static const unsigned char payload[] = "SOAP response";
	static const struct send_step steps[] = {
		{ 4, 0 }, { -1, EPIPE }
	};
	struct send_fixture fixture;

	fixture.expected = payload;
	fixture.expected_length = sizeof(payload) - 1;
	fixture.offset = 0;
	fixture.steps = steps;
	fixture.step_count = sizeof(steps) / sizeof(steps[0]);
	fixture.calls = 0;
	active_fixture = &fixture;
	errno = 0;

	assert(wsd_stream_send_all(17, payload, sizeof(payload) - 1,
				   fake_send) == -1);
	assert(errno == EPIPE);
	assert(fixture.offset == 4);
	assert(fixture.calls == 2);
}

static void
test_zero_length_send_fails_instead_of_spinning(void)
{
	static const unsigned char payload[] = "x";
	static const struct send_step steps[] = { { 0, 0 } };
	struct send_fixture fixture;

	fixture.expected = payload;
	fixture.expected_length = sizeof(payload) - 1;
	fixture.offset = 0;
	fixture.steps = steps;
	fixture.step_count = sizeof(steps) / sizeof(steps[0]);
	fixture.calls = 0;
	active_fixture = &fixture;
	errno = 0;

	assert(wsd_stream_send_all(17, payload, 1, fake_send) == -1);
	assert(errno == EPIPE);
	assert(fixture.offset == 0);
	assert(fixture.calls == 1);
}

static void
test_impossible_send_count_is_rejected(void)
{
	const unsigned char payload[] = "x";

	errno = 0;
	assert(wsd_stream_send_all(17, payload, 1, impossible_send) == -1);
	assert(errno == EIO);
}

static void
test_invalid_arguments_are_rejected(void)
{
	const unsigned char payload[] = "x";

	errno = 0;
	assert(wsd_stream_send_all(17, NULL, 1, fake_send) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(wsd_stream_send_all(17, payload, 1, NULL) == -1);
	assert(errno == EINVAL);
	assert(wsd_stream_send_all(17, payload, 0, fake_send) == 0);
}

int
main(void)
{
	test_short_writes_and_eintr_send_every_byte();
	test_permanent_error_preserves_successful_prefix_and_errno();
	test_zero_length_send_fails_instead_of_spinning();
	test_impossible_send_count_is_rejected();
	test_invalid_arguments_are_rejected();
	puts("wsdd2 TCP stream send-all regression: PASS");
	return 0;
}
