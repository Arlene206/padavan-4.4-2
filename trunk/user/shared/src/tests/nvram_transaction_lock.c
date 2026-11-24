#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../nvram_transaction_lock.h"

static int
read_byte_timeout(int fd, char *value, unsigned int milliseconds)
{
	fd_set readfds;
	struct timeval timeout;
	int result;

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);
	timeout.tv_sec = milliseconds / 1000;
	timeout.tv_usec = (milliseconds % 1000) * 1000;
	result = select(fd + 1, &readfds, NULL, NULL, &timeout);
	if (result <= 0)
		return result;
	return (read(fd, value, 1) == 1) ? 1 : -1;
}

static int
cross_process_exclusion_test(const char *path)
{
	int pipefd[2], lock_fd, status;
	pid_t child;
	char signal = '\0';

	lock_fd = nvram_transaction_lock_acquire_path(path, 1, 1);
	if (lock_fd < 0)
		return 0;
	if (pipe(pipefd) != 0) {
		nvram_transaction_lock_release(lock_fd);
		return 0;
	}
	child = fork();
	if (child < 0) {
		close(pipefd[0]); close(pipefd[1]);
		nvram_transaction_lock_release(lock_fd);
		return 0;
	}
	if (child == 0) {
		int child_lock;
		close(pipefd[0]);
		(void)close(lock_fd); /* Drop the inherited open-file description. */
		child_lock = nvram_transaction_lock_acquire_path(path, 1, 2);
		signal = (child_lock < 0) ? 'F' : 'A';
		(void)write(pipefd[1], &signal, 1);
		if (child_lock >= 0)
			(void)nvram_transaction_lock_release(child_lock);
		close(pipefd[1]);
		_exit(child_lock < 0 ? 1 : 0);
	}

	close(pipefd[1]);
	status = read_byte_timeout(pipefd[0], &signal, 200);
	if (status != 0 || nvram_transaction_lock_release(lock_fd) != 0) {
		(void)kill(child, SIGKILL);
		(void)waitpid(child, NULL, 0);
		close(pipefd[0]);
		return 0;
	}
	(void)read_byte_timeout(pipefd[0], &signal, 3000);
	close(pipefd[0]);
	if (waitpid(child, &status, 0) != child)
		return 0;
	return status == 0 && signal == 'A';
}

static int
process_exit_release_test(const char *path)
{
	int pipefd[2], status, lock_fd;
	pid_t child;
	char signal = '\0';

	if (pipe(pipefd) != 0)
		return 0;
	child = fork();
	if (child < 0) {
		close(pipefd[0]); close(pipefd[1]);
		return 0;
	}
	if (child == 0) {
		int child_lock;
		close(pipefd[0]);
		child_lock = nvram_transaction_lock_acquire_path(path, 1, 1);
		if (child_lock < 0)
			_exit(2);
		signal = 'H';
		(void)write(pipefd[1], &signal, 1);
		close(pipefd[1]);
		_exit(0); /* The kernel releases flock when the process exits. */
	}
	close(pipefd[1]);
	(void)read_byte_timeout(pipefd[0], &signal, 3000);
	close(pipefd[0]);
	if (waitpid(child, &status, 0) != child || status != 0 || signal != 'H')
		return 0;
	lock_fd = nvram_transaction_lock_acquire_path(path, 1, 0);
	if (lock_fd < 0)
		return 0;
	return nvram_transaction_lock_release(lock_fd) == 0;
}

int
main(void)
{
	char directory[] = "/tmp/nvram-lock-test.XXXXXX";
	char path[256], symlink_path[256], hardlink_path[256];
	struct stat st;
	int lock_fd, result;

	if (!mkdtemp(directory)) {
		perror("mkdtemp");
		return 1;
	}
	snprintf(path, sizeof(path), "%s/lock", directory);
	snprintf(symlink_path, sizeof(symlink_path), "%s/symlink", directory);
	snprintf(hardlink_path, sizeof(hardlink_path), "%s/hardlink", directory);

	lock_fd = nvram_transaction_lock_acquire_path(path, 1, 1);
	if (lock_fd < 0 || fstat(lock_fd, &st) != 0 ||
	    !S_ISREG(st.st_mode) || st.st_nlink != 1 ||
	    (st.st_mode & 07777) != NVRAM_TRANSACTION_LOCK_MODE) {
		fprintf(stderr, "FAIL: secure lock inode creation/permissions\n");
		return 1;
	}
	errno = 0;
	result = nvram_transaction_lock_acquire_path(path, 1, 0);
	if (result >= 0 || errno != EBUSY) {
		fprintf(stderr, "FAIL: bounded exclusive lock contention\n");
		return 1;
	}
	if (nvram_transaction_lock_release(lock_fd) != 0) {
		fprintf(stderr, "FAIL: lock release\n");
		return 1;
	}

	if (symlink(path, symlink_path) != 0) {
		perror("symlink");
		return 1;
	}
	errno = 0;
	result = nvram_transaction_lock_acquire_path(symlink_path, 1, 0);
	if (result >= 0 || (errno != ELOOP && errno != EPERM)) {
		fprintf(stderr, "FAIL: symlink lock path rejection\n");
		return 1;
	}
	unlink(symlink_path);

	if (link(path, hardlink_path) != 0) {
		perror("link");
		return 1;
	}
	errno = 0;
	result = nvram_transaction_lock_acquire_path(hardlink_path, 1, 0);
	if (result >= 0 || errno != EPERM) {
		fprintf(stderr, "FAIL: hard-linked lock inode rejection\n");
		return 1;
	}
	unlink(hardlink_path);

	if (!cross_process_exclusion_test(path)) {
		fprintf(stderr, "FAIL: cross-process exclusive lock ordering\n");
		return 1;
	}
	if (!process_exit_release_test(path)) {
		fprintf(stderr, "FAIL: process-exit lock release\n");
		return 1;
	}

	unlink(path);
	rmdir(directory);
	puts("NVRAM shared-userland transaction lock inode, contention, symlink, hardlink, cross-process and crash-release regression: PASS");
	return 0;
}
