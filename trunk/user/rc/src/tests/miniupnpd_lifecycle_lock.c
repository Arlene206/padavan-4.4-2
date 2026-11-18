#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

#include "miniupnpd_lifecycle_lock.h"
#include "service_wait_policy.h"

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

struct sleep_fixture {
	unsigned int calls;
	unsigned int requested[3];
	unsigned int remaining[3];
};

static unsigned int
fixture_sleep(unsigned int seconds, void *opaque)
{
	struct sleep_fixture *fixture = (struct sleep_fixture *)opaque;
	unsigned int index = fixture->calls++;

	assert(index < 3);
	fixture->requested[index] = seconds;
	return fixture->remaining[index];
}

static int
test_interrupted_service_wait_retries_remaining_time(void)
{
	struct sleep_fixture fixture;

	memset(&fixture, 0, sizeof(fixture));
	fixture.remaining[0] = 3; /* sleep(5) interrupted with 3 seconds left. */
	fixture.remaining[1] = 1; /* A second signal leaves one second. */
	fixture.remaining[2] = 0;
	assert(service_wait_seconds_with(5, fixture_sleep, &fixture) == 0);
	assert(fixture.calls == 3);
	assert(fixture.requested[0] == 5);
	assert(fixture.requested[1] == 3);
	assert(fixture.requested[2] == 1);

	memset(&fixture, 0, sizeof(fixture));
	fixture.remaining[0] = 2; /* Remaining time cannot grow. */
	errno = 0;
	assert(service_wait_seconds_with(1, fixture_sleep, &fixture) == -1);
	assert(errno == EIO);
	assert(fixture.calls == 1);

	errno = 0;
	assert(service_wait_seconds_with(1, NULL, NULL) == -1);
	assert(errno == EINVAL);
	return 0;
}

int
main(void)
{
	assert(test_exclusive_process_lock() == 0);
	assert(test_invalid_paths_are_rejected() == 0);
	assert(test_interrupted_service_wait_retries_remaining_time() == 0);
	puts("MiniUPnP lock and detached service waits preserve remaining time across EINTR: PASS");
	return 0;
}
