#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "inadyn_lifecycle_lock.h"

static void
test_exclusive_process_lock(void)
{
	char directory[] = "/tmp/inadyn-lock-test-XXXXXX";
	char path[256], alias[256];
	pid_t child;
	int fd, status;

	assert(mkdtemp(directory) != NULL);
	assert(snprintf(path, sizeof(path), "%s/lock", directory) > 0);
	assert(snprintf(alias, sizeof(alias), "%s/alias", directory) > 0);
	fd = inadyn_lifecycle_lock_acquire_path(path, 0);
	assert(fd >= 0);
	child = fork();
	assert(child >= 0);
	if (child == 0) {
		int child_fd = inadyn_lifecycle_lock_acquire_path(path, 0);
		_exit(child_fd < 0 && errno == EBUSY ? 0 : 1);
	}
	assert(waitpid(child, &status, 0) == child);
	assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
	assert(inadyn_lifecycle_lock_release(fd) == 0);
	fd = inadyn_lifecycle_lock_acquire_path(path, 0);
	assert(fd >= 0);
	assert(inadyn_lifecycle_lock_release(fd) == 0);
	assert(inadyn_lifecycle_lock_release(-1) == -1 && errno == EBADF);

	assert(link(path, alias) == 0);
	errno = 0;
	assert(inadyn_lifecycle_lock_acquire_path(path, 0) == -1 && errno == EPERM);
	assert(unlink(alias) == 0);
	assert(symlink(path, alias) == 0);
	errno = 0;
	assert(inadyn_lifecycle_lock_acquire_path(alias, 0) == -1 && errno == ELOOP);
	assert(unlink(alias) == 0);
	assert(unlink(path) == 0);
	assert(rmdir(directory) == 0);
}
static void
test_invalid_paths_fail_closed(void)
{
	errno = 0;
	assert(inadyn_lifecycle_lock_acquire_path(NULL, 0) == -1 && errno == EINVAL);
	errno = 0;
	assert(inadyn_lifecycle_lock_acquire_path("", 0) == -1 && errno == EINVAL);
}
int
main(void)
{
	test_exclusive_process_lock();
	test_invalid_paths_fail_closed();
	puts("Inadyn exclusive lifecycle lock and path validation: PASS");
	return 0;
}
