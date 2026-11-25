#define _POSIX_C_SOURCE 200809L
#include "iapp_signal_pipe_policy.h"

#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int fake_calls;
static int fake_mode;
static const char test_record[] = "signal-record";

#define FAKE_EINTR_THEN_SUCCESS 1
#define FAKE_SHORT_WRITE 2
#define FAKE_EAGAIN 3

static ssize_t fake_write(int fd, const void *buffer, size_t length)
{
	(void)fd;
	(void)buffer;
	++fake_calls;

	if (fake_mode == FAKE_EINTR_THEN_SUCCESS && fake_calls == 1) {
		errno = EINTR;
		return -1;
	}
	if (fake_mode == FAKE_SHORT_WRITE)
		return (ssize_t)(length - 1);
	if (fake_mode == FAKE_EAGAIN) {
		errno = EAGAIN;
		return -1;
	}
	return (ssize_t)length;
}

static void test_argument_bounds(void)
{
	fake_calls = 0;
	assert(iapp_signal_pipe_write_record(-1, test_record,
		sizeof(test_record), PIPE_BUF, fake_write) == -1);
	assert(errno == EINVAL);
	assert(iapp_signal_pipe_write_record(1, test_record, 0,
		PIPE_BUF, fake_write) == -1);
	assert(errno == EINVAL);
	assert(iapp_signal_pipe_write_record(1, test_record,
		PIPE_BUF + 1U, PIPE_BUF, fake_write) == -1);
	assert(errno == EINVAL);
	assert(iapp_signal_pipe_write_record(1, NULL, 1,
		PIPE_BUF, fake_write) == -1);
	assert(errno == EINVAL);
	assert(fake_calls == 0);
}

static void test_eintr_retry(void)
{
	fake_calls = 0;
	fake_mode = FAKE_EINTR_THEN_SUCCESS;
	assert(iapp_signal_pipe_write_record(1, test_record,
		sizeof(test_record), PIPE_BUF, fake_write) == 0);
	assert(fake_calls == 2);
}

static void test_short_write_rejected(void)
{
	fake_calls = 0;
	fake_mode = FAKE_SHORT_WRITE;
	assert(iapp_signal_pipe_write_record(1, test_record,
		sizeof(test_record), PIPE_BUF, fake_write) == -1);
	assert(errno == EIO);
	assert(fake_calls == 1);
}

static void test_would_block_propagated(void)
{
	fake_calls = 0;
	fake_mode = FAKE_EAGAIN;
	assert(iapp_signal_pipe_write_record(1, test_record,
		sizeof(test_record), PIPE_BUF, fake_write) == -1);
	assert(errno == EAGAIN);
	assert(fake_calls == 1);
}

static void test_full_nonblocking_pipe(void)
{
	int pipefd[2];
	int flags;
	char fill[PIPE_BUF];
	ssize_t written;

	memset(fill, 'x', sizeof(fill));
	assert(pipe(pipefd) == 0);
	flags = fcntl(pipefd[1], F_GETFL, 0);
	assert(flags >= 0);
	assert(fcntl(pipefd[1], F_SETFL, flags | O_NONBLOCK) == 0);
	assert((fcntl(pipefd[1], F_GETFL, 0) & O_NONBLOCK) != 0);

	for (;;) {
		written = write(pipefd[1], fill, sizeof(fill));
		if (written < 0) {
			assert(errno == EAGAIN || errno == EWOULDBLOCK);
			break;
		}
		assert((size_t)written == sizeof(fill));
	}

	/* A full queue must fail immediately rather than deadlock its reader. */
	alarm(2);
	assert(iapp_signal_pipe_write_record(pipefd[1], test_record,
		sizeof(test_record), PIPE_BUF, write) == -1);
	assert(errno == EAGAIN || errno == EWOULDBLOCK);
	alarm(0);

	assert(close(pipefd[0]) == 0);
	assert(close(pipefd[1]) == 0);
}

int main(void)
{
	test_argument_bounds();
	test_eintr_retry();
	test_short_write_rejected();
	test_would_block_propagated();
	test_full_nonblocking_pipe();
	puts("IAPP signal pipe write policy: PASS");
	return 0;
}
