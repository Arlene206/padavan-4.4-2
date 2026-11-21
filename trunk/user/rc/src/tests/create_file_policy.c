#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "create_file_policy.h"
static int open_result, open_error, close_result, close_error;
static int open_calls, close_calls, closed_fd;
static int
mock_open(const char *path, int flags, mode_t mode)
{
	assert(strcmp(path, "/tmp/create-file-policy") == 0);
	assert(flags == (O_RDWR | O_CREAT));
	assert(mode == 0666);
	open_calls++;
	errno = open_error;
	return open_result;
}
static int
mock_close(int fd)
{
	close_calls++;
	closed_fd = fd;
	errno = close_error;
	return close_result;
}
static void
reset_mocks(int open_status, int open_errno, int close_status, int close_errno)
{
	open_result = open_status;
	open_error = open_errno;
	close_result = close_status;
	close_error = close_errno;
	open_calls = 0;
	close_calls = 0;
	closed_fd = -1;
}
static void
test_success_waits_for_close(void)
{
	reset_mocks(17, EBUSY, 0, EIO);
	assert(create_file_with_ops("/tmp/create-file-policy", mock_open, mock_close) == 0);
	assert(open_calls == 1 && close_calls == 1 && closed_fd == 17);
}
static void
test_open_failure_preserves_errno(void)
{
	reset_mocks(-1, EACCES, 0, 0);
	assert(create_file_with_ops("/tmp/create-file-policy", mock_open, mock_close) == 1);
	assert(errno == EACCES && open_calls == 1 && close_calls == 0);
}
static void
test_close_failure_preserves_errno(void)
{
	reset_mocks(23, 0, -1, ENOSPC);
	assert(create_file_with_ops("/tmp/create-file-policy", mock_open, mock_close) == 1);
	assert(errno == ENOSPC && open_calls == 1 && close_calls == 1 && closed_fd == 23);
}
static void
test_close_failure_without_errno_uses_eio(void)
{
	reset_mocks(29, 0, -1, 0);
	assert(create_file_with_ops("/tmp/create-file-policy", mock_open, mock_close) == 1);
	assert(errno == EIO && close_calls == 1);
}
static void
test_invalid_arguments_fail_closed(void)
{
	reset_mocks(0, 0, 0, 0);
	assert(create_file_with_ops(NULL, mock_open, mock_close) == 1);
	assert(errno == EINVAL && open_calls == 0 && close_calls == 0);
	assert(create_file_with_ops("/tmp/create-file-policy", NULL, mock_close) == 1);
	assert(errno == EINVAL);
}
int
main(void)
{
	test_success_waits_for_close();
	test_open_failure_preserves_errno();
	test_close_failure_preserves_errno();
	test_close_failure_without_errno_uses_eio();
	test_invalid_arguments_fail_closed();
	puts("create_file open/close result policy: PASS");
	return 0;
}
