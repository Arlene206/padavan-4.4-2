#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "miniupnpd_pidfile_policy.h"

struct write_fixture {
	size_t bytes_written;
	size_t fail_after;
	size_t max_chunk;
	unsigned int interruptions;
	unsigned int calls;
};
static ssize_t
fixture_write(int fd, const void *buffer, size_t length, void *opaque)
{
	struct write_fixture *fixture = (struct write_fixture *)opaque;
	size_t amount = length;
	++fixture->calls;
	if (fixture->interruptions > 0) {
		--fixture->interruptions;
		errno = EINTR;
		return -1;
	}
	if (fixture->bytes_written >= fixture->fail_after &&
	    fixture->fail_after != (size_t)-1) {
		errno = ENOSPC;
		return -1;
	}
	if (fixture->max_chunk > 0 && amount > fixture->max_chunk)
		amount = fixture->max_chunk;
	if (fixture->fail_after != (size_t)-1 &&
	    amount > fixture->fail_after - fixture->bytes_written)
		amount = fixture->fail_after - fixture->bytes_written;
	if (amount == 0)
		return 0;
	{
		ssize_t result = write(fd, buffer, amount);
		if (result > 0)
			fixture->bytes_written += (size_t)result;
		return result;
	}
}
static ssize_t
zero_write(int fd, const void *buffer, size_t length, void *opaque)
{
	(void)fd;
	(void)buffer;
	(void)length;
	(void)opaque;
	return 0;
}
static void
read_exact_file(const char *path, const char *expected)
{
	char buffer[128];
	size_t expected_length = strlen(expected);
	ssize_t count;
	int fd = open(path, O_RDONLY);
	assert(fd >= 0);
	count = read(fd, buffer, sizeof(buffer));
	assert(count == (ssize_t)expected_length);
	assert(memcmp(buffer, expected, expected_length) == 0);
	assert(read(fd, buffer, 1) == 0);
	assert(close(fd) == 0);
}
static unsigned int
count_directory_entries(const char *path)
{
	DIR *directory = opendir(path);
	struct dirent *entry;
	unsigned int count = 0;
	assert(directory != NULL);
	while ((entry = readdir(directory)) != NULL) {
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
			++count;
	}
	assert(closedir(directory) == 0);
	return count;
}
static void
test_short_write_and_eintr_retry(const char *path)
{
	struct write_fixture fixture;
	struct stat st;
	memset(&fixture, 0, sizeof(fixture));
	fixture.fail_after = (size_t)-1;
	fixture.max_chunk = 1;
	fixture.interruptions = 1;
	assert(miniupnpd_pidfile_publish_with(path, "42\n", 3,
	                                      fixture_write, &fixture) == 0);
	assert(fixture.calls == 4);
	read_exact_file(path, "42\n");
	assert(stat(path, &st) == 0);
	assert((st.st_mode & 0777) == 0644);
}
static void
test_repeated_eintr_is_bounded(const char *path, const char *directory)
{
	struct write_fixture fixture;
	memset(&fixture, 0, sizeof(fixture));
	fixture.fail_after = (size_t)-1;
	fixture.interruptions = MINIUPNPD_PIDFILE_MAX_INTERRUPTS + 1U;
	errno = 0;
	assert(miniupnpd_pidfile_publish_with(path, "99\n", 3,
	                                      fixture_write, &fixture) == -1);
	assert(errno == EINTR);
	assert(fixture.calls == MINIUPNPD_PIDFILE_MAX_INTERRUPTS + 1U);
	read_exact_file(path, "42\n");
	assert(count_directory_entries(directory) == 1);
}
static void
test_failed_write_preserves_previous_pid(const char *path, const char *directory)
{
	struct write_fixture fixture;
	memset(&fixture, 0, sizeof(fixture));
	fixture.fail_after = 1;
	fixture.max_chunk = 1;
	errno = 0;
	assert(miniupnpd_pidfile_publish_with(path, "987654\n", 7,
	                                      fixture_write, &fixture) == -1);
	assert(errno == ENOSPC);
	read_exact_file(path, "42\n");
	assert(count_directory_entries(directory) == 1);
}
static void
test_zero_write_fails_without_replacing(const char *path)
{
	errno = 0;
	assert(miniupnpd_pidfile_publish_with(path, "7\n", 2,
	                                      zero_write, NULL) == -1);
	assert(errno == EIO);
	read_exact_file(path, "42\n");
}
static void
test_symlink_destination_is_replaced_without_following(const char *directory)
{
	char path[256];
	char target[256];
	struct stat st;
	int fd;
	assert(snprintf(path, sizeof(path), "%s/pid-link", directory) > 0);
	assert(snprintf(target, sizeof(target), "%s/target", directory) > 0);
	fd = open(target, O_WRONLY | O_CREAT | O_EXCL, 0600);
	assert(fd >= 0);
	assert(write(fd, "keep\n", 5) == 5);
	assert(close(fd) == 0);
	assert(symlink(target, path) == 0);
	assert(miniupnpd_pidfile_publish(path, "77\n", 3) == 0);
	assert(lstat(path, &st) == 0 && S_ISREG(st.st_mode));
	read_exact_file(path, "77\n");
	read_exact_file(target, "keep\n");
}
static void
test_invalid_arguments(void)
{
	errno = 0;
	assert(miniupnpd_pidfile_publish(NULL, "1\n", 2) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(miniupnpd_pidfile_publish("", "1\n", 2) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(miniupnpd_pidfile_publish("/tmp/unused", NULL, 2) == -1);
	assert(errno == EINVAL);
}
int
main(void)
{
	char directory[] = "/tmp/miniupnpd-pidfile-XXXXXX";
	char path[256];
	int fd;
	assert(mkdtemp(directory) != NULL);
	assert(snprintf(path, sizeof(path), "%s/pid", directory) > 0);
	fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
	assert(fd >= 0);
	assert(write(fd, "123456789012345\n", 16) == 16);
	assert(close(fd) == 0);
	test_short_write_and_eintr_retry(path);
	test_repeated_eintr_is_bounded(path, directory);
	test_failed_write_preserves_previous_pid(path, directory);
	test_zero_write_fails_without_replacing(path);
	test_symlink_destination_is_replaced_without_following(directory);
	test_invalid_arguments();
	assert(unlink(path) == 0);
	{
		char link_path[256];
		char target_path[256];
		assert(snprintf(link_path, sizeof(link_path), "%s/pid-link", directory) > 0);
		assert(snprintf(target_path, sizeof(target_path), "%s/target", directory) > 0);
		assert(unlink(link_path) == 0);
		assert(unlink(target_path) == 0);
	}
	assert(rmdir(directory) == 0);
	puts("miniupnpd atomic PID-file publication and full-write regression: PASS");
	return 0;
}
