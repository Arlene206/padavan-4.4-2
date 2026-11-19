#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "process_pidfile.h"
#include "process_io_retry_policy.h"

static void expect_valid(const char *record, size_t length,
			 pid_t expected_pid, unsigned long long expected_start)
{
	pid_t pid = 0;
	unsigned long long start_time = 0;
	assert(process_pidfile_parse_record(record, length, &pid, &start_time) == 0);
	assert(pid == expected_pid);
	assert(start_time == expected_start);
}

static void expect_invalid(const char *record, size_t length)
{
	pid_t pid = 0;
	unsigned long long start_time = 0;
	assert(process_pidfile_parse_record(record, length, &pid, &start_time) == -1);
}

static void expect_valid_link(const char *record, size_t length,
			      pid_t expected_pid, unsigned long long expected_start)
{
	pid_t pid = 0;
	unsigned long long start_time = 0;
	assert(process_pidfile_parse_link_record(record, length, &pid, &start_time) == 0);
	assert(pid == expected_pid);
	assert(start_time == expected_start);
}

static void expect_invalid_link(const char *record, size_t length)
{
	pid_t pid = 0;
	unsigned long long start_time = 0;
	assert(process_pidfile_parse_link_record(record, length, &pid, &start_time) == -1);
}

struct scripted_io {
	unsigned int interruptions;
	unsigned int calls;
	int failure;
	ssize_t result;
};

static ssize_t
scripted_read(int fd, void *buffer, size_t length, void *opaque)
{
	struct scripted_io *io = (struct scripted_io *)opaque;
	assert(fd == 7 && buffer != NULL && length > 0);
	io->calls++;
	if (io->calls <= io->interruptions) {
		errno = EINTR;
		return -1;
	}
	if (io->failure) {
		errno = io->failure;
		return -1;
	}
	return io->result;
}

static ssize_t
scripted_write(int fd, const void *buffer, size_t length, void *opaque)
{
	struct scripted_io *io = (struct scripted_io *)opaque;
	assert(fd == 7 && buffer != NULL && length > 0);
	io->calls++;
	if (io->calls <= io->interruptions) {
		errno = EINTR;
		return -1;
	}
	if (io->failure) {
		errno = io->failure;
		return -1;
	}
	return io->result;
}

static void
test_process_io_retry_policy(void)
{
	char byte = 'x';
	struct scripted_io io;

	memset(&io, 0, sizeof(io));
	io.interruptions = PROCESS_IO_MAX_INTERRUPTS;
	io.result = 1;
	assert(process_io_read_with_retry(7, &byte, 1, scripted_read, &io) == 1);
	assert(io.calls == PROCESS_IO_MAX_INTERRUPTS + 1);

	memset(&io, 0, sizeof(io));
	io.interruptions = PROCESS_IO_MAX_INTERRUPTS + 1;
	errno = 0;
	assert(process_io_read_with_retry(7, &byte, 1, scripted_read, &io) == -1);
	assert(errno == EINTR && io.calls == PROCESS_IO_MAX_INTERRUPTS + 1);

	memset(&io, 0, sizeof(io));
	io.interruptions = PROCESS_IO_MAX_INTERRUPTS;
	io.result = 0;
	assert(process_io_read_with_retry(7, &byte, 1, scripted_read, &io) == 0);
	assert(io.calls == PROCESS_IO_MAX_INTERRUPTS + 1);

	memset(&io, 0, sizeof(io));
	io.failure = EIO;
	errno = 0;
	assert(process_io_read_with_retry(7, &byte, 1, scripted_read, &io) == -1);
	assert(errno == EIO && io.calls == 1);

	memset(&io, 0, sizeof(io));
	io.interruptions = PROCESS_IO_MAX_INTERRUPTS;
	io.result = 1;
	assert(process_io_write_with_retry(7, &byte, 1, scripted_write, &io) == 1);
	assert(io.calls == PROCESS_IO_MAX_INTERRUPTS + 1);

	memset(&io, 0, sizeof(io));
	io.interruptions = PROCESS_IO_MAX_INTERRUPTS + 1;
	errno = 0;
	assert(process_io_write_with_retry(7, &byte, 1, scripted_write, &io) == -1);
	assert(errno == EINTR && io.calls == PROCESS_IO_MAX_INTERRUPTS + 1);

	memset(&io, 0, sizeof(io));
	io.failure = EIO;
	errno = 0;
	assert(process_io_write_with_retry(7, &byte, 1, scripted_write, &io) == -1);
	assert(errno == EIO && io.calls == 1);

	errno = 0;
	assert(process_io_read_with_retry(7, &byte, 1, NULL, &io) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(process_io_write_with_retry(7, &byte, 1, NULL, &io) == -1);
	assert(errno == EINVAL);
}

int main(void)
{
	test_process_io_retry_policy();
	expect_valid("2 1", 3, (pid_t)2, 1ULL);
	expect_valid("2 18446744073709551615", 22, (pid_t)2, ULLONG_MAX);
	expect_valid("00042 98765\n", 12, (pid_t)42, 98765ULL);
	expect_invalid("", 0);
	expect_invalid("2", 1);
	expect_invalid("1 2", 3);
	expect_invalid("0 2", 3);
	expect_invalid("2 0", 3);
	expect_invalid(" 2 3", 4);
	expect_invalid("2  3", 4);
	expect_invalid("2 3 ", 4);
	expect_invalid("2 3\nX", 5);
	expect_invalid("+2 3", 4);
	expect_invalid("2 +3", 4);
	expect_invalid("2147483648 3", 12);
	expect_invalid("2 18446744073709551616", 22);
	expect_invalid("1234567890123456789012345678901234567890123456789012345678901234", 64);
	expect_valid_link("321\nppp12\n98765\n", 16, (pid_t)321, 98765ULL);
	expect_valid_link("321\n\n98765\n", 11, (pid_t)321, 98765ULL);
	expect_invalid_link("321\nppp12\n", 10);
	expect_invalid_link("321\neth0\n98765\n", 15);
	expect_invalid_link("321\npppA\n98765\n", 15);
	expect_invalid_link("321\nppp12\n0\n", 12);
	expect_invalid_link("321\nppp12\n98765\nextra\n", 22);
	expect_invalid_link("1\nppp12\n98765\n", 14);
	puts("process PID-file record regression: PASS");
	return 0;
}
