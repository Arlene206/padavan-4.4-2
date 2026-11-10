#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "dhcp6c_callback_generation_policy.h"

static void
test_strict_generation_parsing_and_interface_identity(void)
{
	unsigned long generation;

	assert(dhcp6c_callback_generation_parse("1", &generation) == 0);
	assert(generation == 1);
	assert(dhcp6c_callback_generation_parse("184467440737095516160", &generation) == -1);
	assert(dhcp6c_callback_generation_parse("", &generation) == -1);
	assert(dhcp6c_callback_generation_parse("0", &generation) == -1);
	assert(dhcp6c_callback_generation_parse("01", &generation) == -1);
	assert(dhcp6c_callback_generation_parse("+1", &generation) == -1);
	assert(dhcp6c_callback_generation_parse("-1", &generation) == -1);
	assert(dhcp6c_callback_generation_parse("1x", &generation) == -1);

	assert(dhcp6c_callback_generation_matches("7", 7, "eth0", "eth0"));
	assert(!dhcp6c_callback_generation_matches("6", 7, "eth0", "eth0"));
	assert(!dhcp6c_callback_generation_matches("7", 7, "eth0", "eth1"));
	assert(!dhcp6c_callback_generation_matches("7", 7, NULL, "eth0"));
	assert(!dhcp6c_callback_generation_matches("7", 0, "eth0", "eth0"));
}

static void
test_generation_overflow_and_format(void)
{
	unsigned long next;
	char text[sizeof(unsigned long) * CHAR_BIT + 2];

	assert(dhcp6c_callback_generation_next(0, &next) == 0 && next == 1);
	assert(dhcp6c_callback_generation_next(ULONG_MAX, &next) == -1);
	assert(dhcp6c_callback_generation_format(42, text, sizeof(text)) == 0);
	assert(strcmp(text, "42") == 0);
	assert(dhcp6c_callback_generation_format(0, text, sizeof(text)) == -1);
}

static void
test_callback_does_not_create_missing_generation_file(void)
{
	char path[] = "/tmp/dhcp6c-callback-missing-XXXXXX";
	unsigned long generation;
	int fd;

	fd = mkstemp(path);
	assert(fd >= 0);
	assert(close(fd) == 0);
	assert(unlink(path) == 0);

	errno = 0;
	assert(dhcp6c_callback_session_lock(path, &generation, 0) == -1);
	assert(errno == ENOENT);
	assert(access(path, F_OK) == -1 && errno == ENOENT);
}

static void
test_symlink_state_file_is_rejected(void)
{
	char target[] = "/tmp/dhcp6c-callback-target-XXXXXX";
	char path[] = "/tmp/dhcp6c-callback-link-XXXXXX";
	unsigned long generation;
	int fd;

	fd = mkstemp(target);
	assert(fd >= 0);
	assert(write(fd, "1\n", 2) == 2);
	assert(close(fd) == 0);
	fd = mkstemp(path);
	assert(fd >= 0);
	assert(close(fd) == 0);
	assert(unlink(path) == 0);
	assert(symlink(target, path) == 0);

	assert(dhcp6c_callback_session_lock(path, &generation, 1) == -1);
	assert(access(target, F_OK) == 0);
	assert(unlink(path) == 0);
	assert(unlink(target) == 0);
}

static void
test_stop_invalidation_rejects_waiting_old_callback(void)
{
	char path[] = "/tmp/dhcp6c-callback-policy-XXXXXX";
	unsigned long generation, next;
	int fd, pipefd[2], child_result, child_status;
	pid_t child;
	ssize_t count;

	fd = mkstemp(path);
	assert(fd >= 0);
	assert(close(fd) == 0);
	assert(unlink(path) == 0);

	fd = dhcp6c_callback_session_lock(path, &generation, 1);
	assert(fd >= 0 && generation == 0);
	assert(dhcp6c_callback_generation_next(generation, &next) == 0);
	assert(dhcp6c_callback_session_write(fd, next) == 0);
	assert(dhcp6c_callback_session_unlock(fd) == 0);

	/* Simulate stop_dhcp6c holding the lock while an old script child waits. */
	fd = dhcp6c_callback_session_lock(path, &generation, 0);
	assert(fd >= 0 && generation == 1);
	assert(pipe(pipefd) == 0);
	child = fork();
	assert(child >= 0);
	if (child == 0) {
		int child_fd, accepted;
		unsigned long child_generation;
		close(pipefd[0]);
		child_fd = dhcp6c_callback_session_lock(path, &child_generation, 0);
		if (child_fd < 0) {
			child_result = 2;
		} else {
			accepted = dhcp6c_callback_generation_matches("1", child_generation,
							     "eth0", "eth0");
			child_result = accepted ? 1 : 0;
			if (dhcp6c_callback_session_unlock(child_fd) != 0)
				child_result = 3;
		}
		(void)write(pipefd[1], &child_result, sizeof(child_result));
		close(pipefd[1]);
		_exit(child_result == 0 ? 0 : 1);
	}

	close(pipefd[1]);
	assert(dhcp6c_callback_generation_next(generation, &next) == 0);
	assert(dhcp6c_callback_session_write(fd, next) == 0);
	assert(dhcp6c_callback_session_unlock(fd) == 0);
	count = read(pipefd[0], &child_result, sizeof(child_result));
	close(pipefd[0]);
	assert(count == (ssize_t)sizeof(child_result));
	assert(waitpid(child, &child_status, 0) == child);
	assert(WIFEXITED(child_status) && WEXITSTATUS(child_status) == 0);
	assert(child_result == 0);

	fd = dhcp6c_callback_session_lock(path, &generation, 0);
	assert(fd >= 0 && generation == 2);
	assert(dhcp6c_callback_session_unlock(fd) == 0);
	assert(unlink(path) == 0);
}

int main(void)
{
	test_strict_generation_parsing_and_interface_identity();
	test_generation_overflow_and_format();
	test_callback_does_not_create_missing_generation_file();
	test_symlink_state_file_is_rejected();
	test_stop_invalidation_rejects_waiting_old_callback();
	puts("DHCPv6 callback generation/stop fencing regression: PASS");
	return 0;
}
