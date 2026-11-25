#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "infosvr_pidfile_policy.h"

static void
read_exact(int fd, const char *expected)
{
	char actual[32];
	ssize_t amount;
	size_t length = strlen(expected);

	assert(length < sizeof(actual));
	assert(lseek(fd, 0, SEEK_SET) == 0);
	amount = read(fd, actual, sizeof(actual));
	assert(amount == (ssize_t)length);
	assert(memcmp(actual, expected, length) == 0);
}

int
main(void)
{
	char path[] = "/tmp/infosvr-pidfile.XXXXXX";
	int owner_fd;
	int contender_fd;
	int result;

	owner_fd = mkstemp(path);
	assert(owner_fd >= 0);
	assert(infosvr_pidfile_try_exclusive_lock(owner_fd) == 0);
	assert(infosvr_pidfile_write(owner_fd, (pid_t)4321) == 0);
	assert(infosvr_pidfile_matches(owner_fd, (pid_t)4321) == 1);
	assert(infosvr_pidfile_matches(owner_fd, (pid_t)4322) == 0);
	read_exact(owner_fd, "4321");

	contender_fd = open(path, O_RDWR);
	assert(contender_fd >= 0);
	errno = 0;
	result = infosvr_pidfile_try_exclusive_lock(contender_fd);
	assert(result == -1);
	assert(errno == EWOULDBLOCK || errno == EAGAIN);
	/* A contender must not truncate or replace the active owner's record. */
	read_exact(contender_fd, "4321");
	assert(infosvr_pidfile_matches(contender_fd, (pid_t)4321) == 1);
	assert(infosvr_pidfile_matches(contender_fd, (pid_t)4322) == 0);

	assert(close(owner_fd) == 0);
	assert(infosvr_pidfile_try_exclusive_lock(contender_fd) == 0);
	assert(infosvr_pidfile_write(contender_fd, (pid_t)9876) == 0);
	assert(infosvr_pidfile_matches(contender_fd, (pid_t)4321) == 0);
	assert(infosvr_pidfile_matches(contender_fd, (pid_t)9876) == 1);
	read_exact(contender_fd, "9876");

	errno = 0;
	assert(infosvr_pidfile_write(contender_fd, (pid_t)0) == -1);
	assert(errno == EINVAL);
	read_exact(contender_fd, "9876");
	errno = 0;
	assert(infosvr_pidfile_matches(contender_fd, (pid_t)0) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(infosvr_pidfile_try_exclusive_lock(-1) == -1);
	assert(errno == EBADF);

	assert(close(contender_fd) == 0);
	assert(unlink(path) == 0);
	puts("infosvr PID-file exclusive lock, complete publication and owner cleanup policy: PASS");
	return 0;
}
