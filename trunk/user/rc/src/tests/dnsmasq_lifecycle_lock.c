#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "dnsmasq_lifecycle_lock.h"

static void
write_all(int fd, const void *buffer, size_t length)
{
	const char *bytes = (const char *)buffer;
	size_t done = 0;
	ssize_t count;
	while (done < length) {
		count = write(fd, bytes + done, length - done);
		if (count < 0 && errno == EINTR)
			continue;
		assert(count > 0);
		done += (size_t)count;
	}
}
static void
read_all(int fd, void *buffer, size_t length)
{
	char *bytes = (char *)buffer;
	size_t done = 0;
	ssize_t count;
	while (done < length) {
		count = read(fd, bytes + done, length - done);
		if (count < 0 && errno == EINTR)
			continue;
		assert(count > 0);
		done += (size_t)count;
	}
}

int
main(void)
{
	char directory[] = "/tmp/dnsmasq-lock-test.XXXXXX";
	char lock_path[256], hardlink_path[256], symlink_path[256], target_path[256];
	struct stat before, after;
	int parent_to_child[2], child_to_parent[2];
	int lock_fd, status, child_fd;
	char signal_byte;
	pid_t child;

	assert(dnsmasq_lifecycle_lock_acquire_path(NULL, 0) == -1 && errno == EINVAL);
	assert(dnsmasq_lifecycle_lock_acquire_path("", 0) == -1 && errno == EINVAL);
	assert(dnsmasq_lifecycle_lock_release(-1) == -1 && errno == EBADF);
	assert(mkdtemp(directory) != NULL);
	assert(snprintf(lock_path, sizeof(lock_path), "%s/lock", directory) > 0);
	assert(snprintf(hardlink_path, sizeof(hardlink_path), "%s/hardlink", directory) > 0);
	assert(snprintf(symlink_path, sizeof(symlink_path), "%s/symlink", directory) > 0);
	assert(snprintf(target_path, sizeof(target_path), "%s/target", directory) > 0);

	lock_fd = dnsmasq_lifecycle_lock_acquire_path(lock_path, 0);
	assert(lock_fd >= 0);
	assert(fstat(lock_fd, &before) == 0);
	assert(S_ISREG(before.st_mode) && before.st_uid == geteuid());
	assert(before.st_nlink == 1 && (before.st_mode & 0777) == 0600);
	assert(dnsmasq_lifecycle_lock_release(lock_fd) == 0);
	assert(stat(lock_path, &after) == 0);
	assert(before.st_dev == after.st_dev && before.st_ino == after.st_ino);

	/* A child owns the lock while the parent must fail closed immediately. */
	assert(pipe(parent_to_child) == 0);
	assert(pipe(child_to_parent) == 0);
	child = fork();
	assert(child >= 0);
	if (child == 0) {
		close(parent_to_child[1]);
		close(child_to_parent[0]);
		child_fd = dnsmasq_lifecycle_lock_acquire_path(lock_path, 0);
		if (child_fd < 0)
			_exit(10);
		signal_byte = 'L';
		write_all(child_to_parent[1], &signal_byte, 1);
		read_all(parent_to_child[0], &signal_byte, 1);
		if (signal_byte != 'R')
			_exit(11);
		if (dnsmasq_lifecycle_lock_release(child_fd) != 0)
			_exit(12);
		close(parent_to_child[0]);
		close(child_to_parent[1]);
		_exit(0);
	}
	close(parent_to_child[0]);
	close(child_to_parent[1]);
	read_all(child_to_parent[0], &signal_byte, 1);
	assert(signal_byte == 'L');
	assert(dnsmasq_lifecycle_lock_acquire_path(lock_path, 0) == -1);
	assert(errno == EBUSY);
	signal_byte = 'R';
	write_all(parent_to_child[1], &signal_byte, 1);
	close(parent_to_child[1]);
	assert(waitpid(child, &status, 0) == child);
	assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
	lock_fd = dnsmasq_lifecycle_lock_acquire_path(lock_path, 0);
	assert(lock_fd >= 0 && dnsmasq_lifecycle_lock_release(lock_fd) == 0);
	close(child_to_parent[0]);

	assert(chmod(lock_path, 0644) == 0);
	assert(dnsmasq_lifecycle_lock_acquire_path(lock_path, 0) == -1 && errno == EPERM);
	assert(chmod(lock_path, 0600) == 0);
	assert(link(lock_path, hardlink_path) == 0);
	assert(dnsmasq_lifecycle_lock_acquire_path(lock_path, 0) == -1 && errno == EPERM);
	assert(unlink(hardlink_path) == 0);
	child_fd = open(target_path, O_WRONLY | O_CREAT | O_EXCL, 0600);
	assert(child_fd >= 0 && close(child_fd) == 0);
	assert(unlink(lock_path) == 0);
	assert(symlink(target_path, symlink_path) == 0);
	assert(dnsmasq_lifecycle_lock_acquire_path(symlink_path, 0) == -1);
	assert(errno == ELOOP);
	assert(unlink(symlink_path) == 0 && unlink(target_path) == 0);
	assert(dnsmasq_lifecycle_lock_acquire_path(directory, 0) == -1);
	assert(rmdir(directory) == 0);
	puts("DNSMasq lifecycle lock policy: PASS");
	return 0;
}
