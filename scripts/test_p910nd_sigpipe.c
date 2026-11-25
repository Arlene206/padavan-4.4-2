#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

#include "../trunk/user/p910nd/p910nd_sigpipe_policy.h"
#include "../trunk/user/p910nd/p910nd_io_policy.h"

static int read_attempts;
static int write_attempts;
static char written_bytes[8];

static ssize_t
scripted_read(int fd, void *buffer, size_t length)
{
	(void)fd;
	++read_attempts;
	if (read_attempts == 1) {
		errno = EINTR;
		return -1;
	}
	assert(length >= 2);
	memcpy(buffer, "ok", 2);
	return 2;
}

static ssize_t
scripted_write(int fd, const void *buffer, size_t length)
{
	(void)fd;
	++write_attempts;
	if (write_attempts == 1) {
		errno = EINTR;
		return -1;
	}
	assert(length <= sizeof(written_bytes));
	memcpy(written_bytes, buffer, length);
	return (ssize_t)length;
}

static ssize_t
would_block_read(int fd, void *buffer, size_t length)
{
	(void)fd;
	(void)buffer;
	(void)length;
	errno = EAGAIN;
	return -1;
}

static ssize_t
would_block_write(int fd, const void *buffer, size_t length)
{
	(void)fd;
	(void)buffer;
	(void)length;
	errno = EWOULDBLOCK;
	return -1;
}

static void
test_io_retries_interrupted_calls(void)
{
	char input[4] = {0};
	const char output[] = "print";

	read_attempts = 0;
	assert(p910nd_read_retry_eintr(1, input, sizeof(input),
			scripted_read) == 2);
	assert(read_attempts == 2);
	assert(memcmp(input, "ok", 2) == 0);

	write_attempts = 0;
	assert(p910nd_write_retry_eintr(1, output, sizeof(output) - 1,
			scripted_write) == (ssize_t)(sizeof(output) - 1));
	assert(write_attempts == 2);
	assert(memcmp(written_bytes, output, sizeof(output) - 1) == 0);
}

static void
test_io_would_block_is_transient(void)
{
	assert(p910nd_read_retry_eintr(1, written_bytes, sizeof(written_bytes),
			would_block_read) == -1);
	assert(p910nd_io_would_block(errno));
	assert(p910nd_write_retry_eintr(1, written_bytes,
			sizeof(written_bytes), would_block_write) == -1);
	assert(p910nd_io_would_block(errno));
	assert(p910nd_io_would_block(EAGAIN));
	assert(p910nd_io_would_block(EWOULDBLOCK));
	assert(!p910nd_io_would_block(EIO));
}

static void
test_select_fd_bounds_are_checked_before_fd_set(void)
{
	assert(p910nd_fd_is_selectable(0, FD_SETSIZE));
	assert(p910nd_fd_is_selectable(FD_SETSIZE - 1, FD_SETSIZE));
	assert(!p910nd_fd_is_selectable(-1, FD_SETSIZE));
	assert(!p910nd_fd_is_selectable(FD_SETSIZE, FD_SETSIZE));
	assert(!p910nd_fd_is_selectable(FD_SETSIZE + 1, FD_SETSIZE));
	assert(!p910nd_fd_is_selectable(0, 0));
	assert(!p910nd_fd_is_selectable(0, -1));
}

static void
test_zero_length_write_progress_contract(void)
{
	assert(p910nd_write_no_progress(0, 1));
	assert(p910nd_write_no_progress(0, 8192));
	assert(!p910nd_write_no_progress(0, 0));
	assert(!p910nd_write_no_progress(1, 1));
	assert(!p910nd_write_no_progress(-1, 1));
}

int
main(void)
{
	int sockets[2];
	char received[2];
	const char payload[2] = {'o', 'k'};
	const char byte = 'x';
	struct sigaction action;

	test_io_retries_interrupted_calls();
	test_io_would_block_is_transient();
	test_zero_length_write_progress_contract();
	test_select_fd_bounds_are_checked_before_fd_set();

	assert(signal(SIGPIPE, SIG_DFL) != SIG_ERR);
	assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
	assert(write(sockets[0], payload, sizeof(payload)) == (ssize_t)sizeof(payload));
	assert(read(sockets[1], received, sizeof(received)) == (ssize_t)sizeof(received));
	assert(memcmp(payload, received, sizeof(payload)) == 0);

	assert(p910nd_install_sigpipe_policy() == 0);
	assert(sigaction(SIGPIPE, NULL, &action) == 0);
	assert(action.sa_handler == SIG_IGN);

	assert(close(sockets[1]) == 0);
	errno = 0;
	assert(write(sockets[0], &byte, sizeof(byte)) == -1);
	assert(errno == EPIPE);
	assert(close(sockets[0]) == 0);
	return 0;
}
