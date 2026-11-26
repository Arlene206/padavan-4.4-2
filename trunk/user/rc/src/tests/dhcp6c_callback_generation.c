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
test_primary_operation_errno_survives_cleanup_failure(void)
{
	assert(dhcp6c_callback_primary_errno(EBUSY, EIO) == EBUSY);
	assert(dhcp6c_callback_primary_errno(0, EIO) == EIO);
	assert(dhcp6c_callback_primary_errno(0, 0) == EIO);
	assert(dhcp6c_callback_primary_errno(EPERM, 0) == EPERM);
}

static int fake_unsetenv_calls;

static int
fake_unsetenv_failure(const char *name)
{
	fake_unsetenv_calls++;
	if (strcmp(name, "CALLBACK_IFNAME") == 0) {
		errno = EBUSY;
		return -1;
	}
	if (strcmp(name, "CALLBACK_GENERATION") == 0) {
		errno = EIO;
		return -1;
	}
	errno = EINVAL;
	return -1;
}

static void
test_environment_cleanup_attempts_both_and_preserves_first_error(void)
{
	fake_unsetenv_calls = 0;
	errno = 0;
	assert(dhcp6c_callback_environment_clear(
		"CALLBACK_IFNAME", "CALLBACK_GENERATION",
		fake_unsetenv_failure) == -1);
	assert(fake_unsetenv_calls == 2);
	assert(errno == EBUSY);
}

static void
test_environment_cleanup_success(void)
{
	assert(setenv("PADAVAN_DHCP6C_TEST_IFNAME", "wan0", 1) == 0);
	assert(setenv("PADAVAN_DHCP6C_TEST_GENERATION", "7", 1) == 0);
	assert(dhcp6c_callback_environment_clear(
		"PADAVAN_DHCP6C_TEST_IFNAME", "PADAVAN_DHCP6C_TEST_GENERATION",
		unsetenv) == 0);
	assert(getenv("PADAVAN_DHCP6C_TEST_IFNAME") == NULL);
	assert(getenv("PADAVAN_DHCP6C_TEST_GENERATION") == NULL);
}

static void
test_successful_start_with_environment_cleanup_failure_requires_stop(void)
{
	assert(dhcp6c_callback_start_requires_stop(0, -1));
	assert(dhcp6c_callback_start_requires_stop(0, EIO));
	assert(!dhcp6c_callback_start_requires_stop(0, 0));
	assert(!dhcp6c_callback_start_requires_stop(-1, EIO));
	assert(!dhcp6c_callback_start_requires_stop(-1, 0));
}

static void
test_start_errno_uses_cleanup_error_after_successful_launch(void)
{
	/* errno may be stale after a successful eval()/launch. */
	assert(dhcp6c_callback_start_errno(0, EBUSY, -1, EIO) == EIO);
	assert(dhcp6c_callback_start_errno(0, EBUSY, 0, 0) == 0);
	/* Preserve a real launch error ahead of a secondary cleanup error. */
	assert(dhcp6c_callback_start_errno(-1, ENOENT, -1, EIO) == ENOENT);
	assert(dhcp6c_callback_start_errno(-1, 0, -1, EIO) == EIO);
	assert(dhcp6c_callback_start_errno(-1, 0, 0, 0) == EIO);
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

struct callback_observation {
	unsigned long generation;
	int accepted;
	int status;
};

static void
run_stop_fencing_case(int stop_status, int expected_accept,
		      unsigned long expected_generation)
{
	char path[] = "/tmp/dhcp6c-callback-policy-XXXXXX";
	unsigned long generation, next;
	int fd, ready_pipe[2], result_pipe[2], started, child_status;
	struct callback_observation observed;
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

	/* Hold the same lock stop_dhcp6c() keeps across kill_services(). */
	fd = dhcp6c_callback_session_lock(path, &generation, 0);
	assert(fd >= 0 && generation == 1);
	assert(pipe(ready_pipe) == 0);
	assert(pipe(result_pipe) == 0);
	child = fork();
	assert(child >= 0);
	if (child == 0) {
		int child_fd;
		unsigned long child_generation;

		close(ready_pipe[0]);
		close(result_pipe[0]);
		close(fd);
		started = 1;
		if (write(ready_pipe[1], &started, sizeof(started)) !=
		    (ssize_t)sizeof(started))
			_exit(2);
		close(ready_pipe[1]);
		observed.generation = 0;
		observed.accepted = 0;
		observed.status = 0;
		child_fd = dhcp6c_callback_session_lock(path, &child_generation, 0);
		if (child_fd < 0) {
			observed.status = 1;
		} else {
			observed.generation = child_generation;
			observed.accepted = dhcp6c_callback_generation_matches(
				"1", child_generation, "eth0", "eth0");
			if (dhcp6c_callback_session_unlock(child_fd) != 0)
				observed.status = 2;
		}
		if (write(result_pipe[1], &observed, sizeof(observed)) !=
		    (ssize_t)sizeof(observed))
			_exit(3);
		close(result_pipe[1]);
		_exit(observed.status == 0 ? 0 : 4);
	}

	close(ready_pipe[1]);
	close(result_pipe[1]);
	assert(read(ready_pipe[0], &started, sizeof(started)) ==
	       (ssize_t)sizeof(started));
	close(ready_pipe[0]);
	/* Let the child reach its fcntl() attempt while the parent owns the lock. */
	usleep(20000);

	/* A failed kill leaves the prior generation intact; success fences it. */
	if (stop_status == 0) {
		assert(dhcp6c_callback_generation_next(generation, &next) == 0);
		assert(dhcp6c_callback_session_write(fd, next) == 0);
	}
	assert(dhcp6c_callback_session_unlock(fd) == 0);

	count = read(result_pipe[0], &observed, sizeof(observed));
	close(result_pipe[0]);
	assert(count == (ssize_t)sizeof(observed));
	assert(waitpid(child, &child_status, 0) == child);
	assert(WIFEXITED(child_status) && WEXITSTATUS(child_status) == 0);
	assert(observed.status == 0);
	assert(observed.accepted == expected_accept);
	assert(observed.generation == expected_generation);

	fd = dhcp6c_callback_session_lock(path, &generation, 0);
	assert(fd >= 0 && generation == expected_generation);
	assert(dhcp6c_callback_session_unlock(fd) == 0);
	assert(unlink(path) == 0);
}

static void
test_failed_stop_preserves_waiting_old_callback_generation(void)
{
	run_stop_fencing_case(EBUSY, 1, 1);
}

static void
test_successful_stop_invalidates_waiting_old_callback_generation(void)
{
	run_stop_fencing_case(0, 0, 2);
}

int main(void)
{
	test_strict_generation_parsing_and_interface_identity();
	test_generation_overflow_and_format();
	test_primary_operation_errno_survives_cleanup_failure();
	test_environment_cleanup_attempts_both_and_preserves_first_error();
	test_environment_cleanup_success();
	test_successful_start_with_environment_cleanup_failure_requires_stop();
	test_start_errno_uses_cleanup_error_after_successful_launch();
	test_callback_does_not_create_missing_generation_file();
	test_symlink_state_file_is_rejected();
	test_failed_stop_preserves_waiting_old_callback_generation();
	test_successful_stop_invalidates_waiting_old_callback_generation();
	puts("DHCPv6 callback generation/stop fencing regression: PASS");
	return 0;
}
