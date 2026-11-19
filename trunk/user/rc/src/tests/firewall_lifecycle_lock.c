#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../firewall_lifecycle_lock.h"

static void
test_invalid_arguments(void)
{
	errno = 0;
	assert(firewall_lifecycle_lock_acquire_path(NULL, 0) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(firewall_lifecycle_lock_acquire_path("", 0) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(firewall_lifecycle_lock_release(-1) == -1);
	assert(errno == EBADF);
}

static void
test_cross_process_exclusion_and_stable_inode(void)
{
	char directory[] = "/tmp/firewall-lock-test.XXXXXX";
	char path[256];
	struct stat before, after;
	int fd, status;
	pid_t child;

	assert(mkdtemp(directory) != NULL);
	assert(snprintf(path, sizeof(path), "%s/lock", directory) > 0);
	fd = firewall_lifecycle_lock_acquire_path(path, 0);
	assert(fd >= 0);
	assert(fstat(fd, &before) == 0);
	assert(S_ISREG(before.st_mode));
	assert(before.st_uid == geteuid());
	assert(before.st_nlink == 1);
	assert((before.st_mode & 0777) == 0600);

	child = fork();
	assert(child >= 0);
	if (child == 0) {
		int child_fd;
		errno = 0;
		child_fd = firewall_lifecycle_lock_acquire_path(path, 0);
		if (child_fd >= 0) {
			(void)firewall_lifecycle_lock_release(child_fd);
			_exit(10);
		}
		_exit(errno == EBUSY ? 0 : 11);
	}
	assert(waitpid(child, &status, 0) == child);
	assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
	assert(firewall_lifecycle_lock_release(fd) == 0);

	fd = firewall_lifecycle_lock_acquire_path(path, 0);
	assert(fd >= 0);
	assert(firewall_lifecycle_lock_release(fd) == 0);
	assert(stat(path, &after) == 0);
	assert(before.st_dev == after.st_dev && before.st_ino == after.st_ino);
	assert(unlink(path) == 0);
	assert(rmdir(directory) == 0);
}

static void
test_untrusted_lock_files_fail_closed(void)
{
	char directory[] = "/tmp/firewall-lock-security.XXXXXX";
	char path[256], hardlink_path[256], symlink_path[256], target_path[256];
	int fd;

	assert(mkdtemp(directory) != NULL);
	assert(snprintf(path, sizeof(path), "%s/lock", directory) > 0);
	assert(snprintf(hardlink_path, sizeof(hardlink_path), "%s/hardlink", directory) > 0);
	assert(snprintf(symlink_path, sizeof(symlink_path), "%s/symlink", directory) > 0);
	assert(snprintf(target_path, sizeof(target_path), "%s/target", directory) > 0);

	fd = firewall_lifecycle_lock_acquire_path(path, 0);
	assert(fd >= 0 && firewall_lifecycle_lock_release(fd) == 0);

	assert(chmod(path, 0644) == 0);
	errno = 0;
	assert(firewall_lifecycle_lock_acquire_path(path, 0) == -1);
	assert(errno == EPERM);
	assert(chmod(path, 0600) == 0);

	assert(link(path, hardlink_path) == 0);
	errno = 0;
	assert(firewall_lifecycle_lock_acquire_path(path, 0) == -1);
	assert(errno == EPERM);
	assert(unlink(hardlink_path) == 0);

	fd = open(target_path, O_WRONLY | O_CREAT | O_EXCL, 0600);
	assert(fd >= 0 && close(fd) == 0);
	assert(symlink(target_path, symlink_path) == 0);
	errno = 0;
	assert(firewall_lifecycle_lock_acquire_path(symlink_path, 0) == -1);
	assert(errno == ELOOP);

	assert(unlink(symlink_path) == 0);
	assert(unlink(target_path) == 0);
	assert(unlink(path) == 0);
	assert(rmdir(directory) == 0);
}

int
main(void)
{
	test_invalid_arguments();
	test_cross_process_exclusion_and_stable_inode();
	test_untrusted_lock_files_fail_closed();
	puts("Firewall lifecycle lock policy: PASS");
	return 0;
}
