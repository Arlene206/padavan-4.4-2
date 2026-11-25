#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ppp_callback_state_file.h"

static void
assert_no_staged_files(const char *directory)
{
	DIR *stream;
	struct dirent *entry;

	stream = opendir(directory);
	assert(stream != NULL);
	while ((entry = readdir(stream)) != NULL)
		assert(strstr(entry->d_name, ".tmp.") == NULL);
	assert(closedir(stream) == 0);
}

int
main(void)
{
	char directory[] = "/tmp/ppp-callback-state.XXXXXX";
	char lock_path[320], state_path[320], link_path[320];
	char victim_path[320], empty_path[320], directory_path[320];
	struct stat lock_fd_stat, lock_path_stat, state_stat;
	struct rlimit saved_limit, limited;
	void (*saved_sigxfsz)(int);
	unsigned long generation, next_generation;
	char formatted[sizeof(unsigned long) * CHAR_BIT + 2];
	int lock_fd, status, fd;
	pid_t child;
	int child_status;

	assert(mkdtemp(directory) != NULL);
	assert(ppp_callback_generation_matches("42", 42));
	assert(ppp_callback_generation_initial_state_allowed(1, 1));
	assert(ppp_callback_generation_next(42, &next_generation) == 0);
	assert(next_generation == 43);
	assert(ppp_callback_generation_format(next_generation, formatted,
					      sizeof(formatted)) == 0);
	assert(strcmp(formatted, "43") == 0);
	assert(snprintf(lock_path, sizeof(lock_path), "%s/wan-session.lock",
			directory) > 0);
	assert(snprintf(state_path, sizeof(state_path), "%s/wan-session.state",
			directory) > 0);
	assert(snprintf(link_path, sizeof(link_path), "%s/state-link",
			directory) > 0);
	assert(snprintf(victim_path, sizeof(victim_path), "%s/victim",
			directory) > 0);
	assert(snprintf(empty_path, sizeof(empty_path), "%s/empty",
			directory) > 0);
	assert(snprintf(directory_path, sizeof(directory_path), "%s/target-dir",
			directory) > 0);

	assert(ppp_callback_state_file_read(state_path, 0, &generation) == -1);
	assert(errno == ENOENT);
	assert(ppp_callback_state_file_read(state_path, 1, &generation) == 0);
	assert(generation == 0);

	lock_fd = ppp_callback_state_file_lock(lock_path, state_path,
					       &generation, 1, 10);
	assert(lock_fd >= 0);
	assert(generation == 0);
	assert(fstat(lock_fd, &lock_fd_stat) == 0);
	assert(stat(lock_path, &lock_path_stat) == 0);
	assert(lock_fd_stat.st_dev == lock_path_stat.st_dev);
	assert(lock_fd_stat.st_ino == lock_path_stat.st_ino);
	assert((lock_fd_stat.st_mode & 0777) == 0600);

	/* Replacing state must leave the inode containing the lock untouched. */
	assert(ppp_callback_state_file_publish(state_path, 41) == 0);
	assert(stat(lock_path, &lock_path_stat) == 0);
	assert(lock_fd_stat.st_dev == lock_path_stat.st_dev);
	assert(lock_fd_stat.st_ino == lock_path_stat.st_ino);
	assert(ppp_callback_state_file_read(state_path, 0, &generation) == 0);
	assert(generation == 41);
	assert(stat(state_path, &state_stat) == 0);
	assert((state_stat.st_mode & 0777) == 0600);
	assert(ppp_callback_state_file_unlock(lock_fd) == 0);

	lock_fd = ppp_callback_state_file_lock(lock_path, state_path,
					       &generation, 0, 10);
	assert(lock_fd >= 0);
	assert(generation == 41);

	/* The sidecar lock fences a second process across data-file renames. */
	child = fork();
	assert(child >= 0);
	if (child == 0) {
		close(lock_fd);
		fd = ppp_callback_state_file_lock(lock_path, state_path,
						  &generation, 0, 1);
		if (fd >= 0) {
			(void)ppp_callback_state_file_unlock(fd);
			_exit(2);
		}
		_exit(errno == EBUSY ? 0 : 3);
	}
	assert(waitpid(child, &child_status, 0) == child);
	assert(WIFEXITED(child_status) && WEXITSTATUS(child_status) == 0);

	assert(ppp_callback_state_file_publish(state_path, 42) == 0);
	assert(ppp_callback_state_file_read(state_path, 0, &generation) == 0);
	assert(generation == 42);
	assert(stat(lock_path, &lock_path_stat) == 0);
	assert(lock_fd_stat.st_ino == lock_path_stat.st_ino);

	/* A partial write must leave the previous complete generation intact. */
	assert(getrlimit(RLIMIT_FSIZE, &saved_limit) == 0);
	assert(saved_limit.rlim_max == RLIM_INFINITY || saved_limit.rlim_max >= 1);
	limited = saved_limit;
	limited.rlim_cur = 1;
	saved_sigxfsz = signal(SIGXFSZ, SIG_IGN);
	assert(saved_sigxfsz != SIG_ERR);
	assert(setrlimit(RLIMIT_FSIZE, &limited) == 0);
	status = ppp_callback_state_file_publish(state_path, 43);
	assert(setrlimit(RLIMIT_FSIZE, &saved_limit) == 0);
	assert(signal(SIGXFSZ, saved_sigxfsz) != SIG_ERR);
	assert(status == -1);
	assert(ppp_callback_state_file_read(state_path, 0, &generation) == 0);
	assert(generation == 42);
	assert_no_staged_files(directory);

	/* Empty, non-regular and linked state must never reset or follow. */
	fd = open(empty_path, O_WRONLY | O_CREAT | O_EXCL, 0600);
	assert(fd >= 0 && close(fd) == 0);
	assert(ppp_callback_state_file_read(empty_path, 1, &generation) == -1);
	assert(errno == ENOENT);
	fd = open(victim_path, O_WRONLY | O_CREAT | O_EXCL, 0600);
	assert(fd >= 0);
	assert(write(fd, "do-not-follow", 13) == 13);
	assert(close(fd) == 0);
	assert(symlink(victim_path, link_path) == 0);
	assert(ppp_callback_state_file_read(link_path, 1, &generation) == -1);
	assert(errno == ELOOP || errno == EINVAL);
	assert(ppp_callback_state_file_publish(link_path, 44) == -1);
	{
		FILE *victim = fopen(victim_path, "r");
		char text[32];
		size_t length;

		assert(victim != NULL);
		length = fread(text, 1, sizeof(text), victim);
		assert(!ferror(victim));
		assert(fclose(victim) == 0);
		assert(length == 13 && memcmp(text, "do-not-follow", 13) == 0);
	}
	assert(mkdir(directory_path, 0700) == 0);
	assert(ppp_callback_state_file_read(directory_path, 1, &generation) == -1);
	assert(errno == EINVAL);

	assert(ppp_callback_state_file_unlock(lock_fd) == 0);
	assert(unlink(state_path) == 0);
	assert(unlink(lock_path) == 0);
	assert(unlink(link_path) == 0);
	assert(unlink(victim_path) == 0);
	assert(unlink(empty_path) == 0);
	assert(rmdir(directory_path) == 0);
	assert(rmdir(directory) == 0);
	puts("PPP callback stable lock and atomic generation-state publication: PASS");
	return 0;
}
