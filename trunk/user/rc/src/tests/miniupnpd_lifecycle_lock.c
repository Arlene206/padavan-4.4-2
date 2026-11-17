#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "miniupnpd_lifecycle_lock.h"

static int
test_exclusive_process_lock(void)
{
	char directory[] = "/tmp/miniupnpd-lock-test-XXXXXX";
	char path[256], alias[256];
	pid_t child;
	int fd, status;

	assert(mkdtemp(directory) != NULL);
	assert(snprintf(path, sizeof(path), "%s/lock", directory) > 0);
	assert(snprintf(alias, sizeof(alias), "%s/alias", directory) > 0);
	fd = miniupnpd_lifecycle_lock_acquire_path(path, 0);
	assert(fd >= 0);

	child = fork();
	assert(child >= 0);
	if (child == 0) {
		int child_fd = miniupnpd_lifecycle_lock_acquire_path(path, 0);
		_exit(child_fd < 0 && errno == EBUSY ? 0 : 1);
	}
	assert(waitpid(child, &status, 0) == child);
	assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
	assert(miniupnpd_lifecycle_lock_release(fd) == 0);

	fd = miniupnpd_lifecycle_lock_acquire_path(path, 0);
	assert(fd >= 0);
	assert(miniupnpd_lifecycle_lock_release(fd) == 0);
	assert(miniupnpd_lifecycle_lock_release(-1) == -1 && errno == EBADF);

	assert(link(path, alias) == 0);
	errno = 0;
	assert(miniupnpd_lifecycle_lock_acquire_path(path, 0) == -1);
	assert(errno == EPERM);
	assert(unlink(alias) == 0);

	assert(symlink(path, alias) == 0);
	errno = 0;
	assert(miniupnpd_lifecycle_lock_acquire_path(alias, 0) == -1);
	assert(errno == ELOOP);
	assert(unlink(alias) == 0);

	assert(unlink(path) == 0);
	assert(rmdir(directory) == 0);
	return 0;
}

static int
test_invalid_paths_are_rejected(void)
{
	errno = 0;
	assert(miniupnpd_lifecycle_lock_acquire_path(NULL, 0) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(miniupnpd_lifecycle_lock_acquire_path("", 0) == -1);
	assert(errno == EINVAL);
	return 0;
}

int
main(void)
{
	assert(test_exclusive_process_lock() == 0);
	assert(test_invalid_paths_are_rejected() == 0);
	puts("MiniUPnP lifecycle lock contention, inode validation and release: PASS");
	return 0;
}
