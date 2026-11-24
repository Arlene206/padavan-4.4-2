#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "bin_sem_asus.h"
#define STORAGE_OPERATION_LOCK_TAG "storage-regression"
#include "../storage_operation_lock.h"

static int
remove_lock_file(const char *tag)
{
	char path[96];
	int len;

	len = snprintf(path, sizeof(path), "/var/lock/%s.lock", tag);
	if (len < 0 || len >= (int)sizeof(path))
		return -1;
	if (unlink(path) != 0 && errno != ENOENT)
		return -1;
	return 0;
}

static int
elapsed_milliseconds(const struct timeval *start, const struct timeval *end)
{
	return (int)((end->tv_sec - start->tv_sec) * 1000 +
		     (end->tv_usec - start->tv_usec) / 1000);
}

int
main(void)
{
	char disk_tag[48], partition_tag[48], ready;
	int disk_lock, partition_lock, lock_fd, pipe_fd[2], elapsed;
	int status;
	pid_t child;
	struct timeval start, finish;

	if (snprintf(disk_tag, sizeof(disk_tag), "sda-regression-%ld", (long)getpid()) >=
	    (int)sizeof(disk_tag) ||
	    snprintf(partition_tag, sizeof(partition_tag), "sda1-regression-%ld", (long)getpid()) >=
	    (int)sizeof(partition_tag))
		return 1;

	/* Different mdev node tags do not serialize a disk and its partition. */
	disk_lock = file_lock(disk_tag);
	if (disk_lock < 0)
		return 2;
	partition_lock = file_lock(partition_tag);
	if (partition_lock < 0) {
		file_unlock(disk_lock);
		return 3;
	}
	file_unlock(partition_lock);
	file_unlock(disk_lock);

	if (pipe(pipe_fd) != 0)
		return 4;
	child = fork();
	if (child < 0) {
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		return 5;
	}
	if (child == 0) {
		close(pipe_fd[0]);
		lock_fd = storage_operation_lock_acquire(file_lock);
		if (lock_fd < 0)
			_exit(6);
		ready = 'L';
		if (write(pipe_fd[1], &ready, 1) != 1) {
			file_unlock(lock_fd);
			_exit(7);
		}
		sleep(2);
		file_unlock(lock_fd);
		close(pipe_fd[1]);
		_exit(0);
	}

	close(pipe_fd[1]);
	if (read(pipe_fd[0], &ready, 1) != 1 || ready != 'L') {
		close(pipe_fd[0]);
		waitpid(child, &status, 0);
		return 8;
	}
	close(pipe_fd[0]);

	if (gettimeofday(&start, NULL) != 0)
		return 9;
	lock_fd = storage_operation_lock_acquire(file_lock);
	if (gettimeofday(&finish, NULL) != 0) {
		if (lock_fd >= 0)
			file_unlock(lock_fd);
		return 10;
	}
	if (lock_fd < 0)
		return 11;
	elapsed = elapsed_milliseconds(&start, &finish);
	file_unlock(lock_fd);

	if (waitpid(child, &status, 0) != child || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		return 12;
	if (elapsed < 1000) {
		fprintf(stderr, "storage operation lock did not block across processes (%d ms)\n", elapsed);
		return 13;
	}

	if (remove_lock_file(disk_tag) != 0 || remove_lock_file(partition_tag) != 0 ||
	    remove_lock_file(STORAGE_OPERATION_LOCK_TAG) != 0)
		return 14;
	printf("storage operation lock regression: PASS (shared lock blocked for %d ms)\n", elapsed);
	return 0;
}
