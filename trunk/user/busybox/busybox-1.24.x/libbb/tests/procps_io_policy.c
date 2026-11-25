#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "procps_io_policy.h"

struct fake_read_state {
	unsigned int interruptions_before_result;
	unsigned int calls;
	ssize_t result;
	int error;
};

static ssize_t
fake_read(int fd, void *buffer, size_t length, void *opaque)
{
	struct fake_read_state *state = (struct fake_read_state *)opaque;

	(void)fd;
	++state->calls;
	if (state->calls <= state->interruptions_before_result) {
		errno = EINTR;
		return -1;
	}
	if (state->error) {
		errno = state->error;
		return -1;
	}
	if (state->result > 0 && length > 0)
		((char *)buffer)[0] = 'r';
	return state->result;
}

static void
test_retries_after_interrupted_reads(void)
{
	struct fake_read_state state = { 3U, 0U, 1, 0 };
	char buffer[4] = {0};
	ssize_t result = procps_read_with_retry_cb(
		7, buffer, sizeof(buffer), fake_read, &state);

	assert(result == 1);
	assert(state.calls == 4U);
	assert(buffer[0] == 'r');
}

static void
test_interrupt_retries_are_bounded(void)
{
	struct fake_read_state state = {
		PROCPS_READ_MAX_INTERRUPTS + 1U, 0U, 1, 0
	};
	char buffer[4] = {0};
	ssize_t result;

	errno = 0;
	result = procps_read_with_retry_cb(
		7, buffer, sizeof(buffer), fake_read, &state);
	assert(result == -1);
	assert(errno == EINTR);
	assert(state.calls == PROCPS_READ_MAX_INTERRUPTS + 1U);
}

static void
test_non_interrupted_errors_are_not_retried(void)
{
	struct fake_read_state state = { 0U, 0U, -1, EIO };
	char buffer[4] = {0};
	ssize_t result;

	errno = 0;
	result = procps_read_with_retry_cb(
		7, buffer, sizeof(buffer), fake_read, &state);
	assert(result == -1);
	assert(errno == EIO);
	assert(state.calls == 1U);
}

static void
test_eof_is_returned_without_retry(void)
{
	struct fake_read_state state = { 0U, 0U, 0, 0 };
	ssize_t result = procps_read_with_retry_cb(
		7, NULL, 0U, fake_read, &state);

	assert(result == 0);
	assert(state.calls == 1U);
}

static void
test_invalid_callback_fails_closed(void)
{
	char buffer[1] = {0};
	ssize_t result;

	errno = 0;
	result = procps_read_with_retry_cb(7, buffer, sizeof(buffer), NULL, NULL);
	assert(result == -1);
	assert(errno == EINVAL);
}

int
main(void)
{
	test_retries_after_interrupted_reads();
	test_interrupt_retries_are_bounded();
	test_non_interrupted_errors_are_not_retried();
	test_eof_is_returned_without_retry();
	test_invalid_callback_fails_closed();
	puts("BusyBox procps interrupted-read policy regression: PASS");
	return 0;
}
