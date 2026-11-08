#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../dhcp6c_duid_file_policy.h"

static void make_path(char *out, size_t out_size, const char *dir, const char *name)
{
	int written = snprintf(out, out_size, "%s/%s", dir, name);
	assert(written > 0 && (size_t)written < out_size);
}

static void write_file(const char *path, const char *value)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
	assert(fd >= 0);
	assert(write(fd, value, strlen(value)) == (ssize_t)strlen(value));
	assert(close(fd) == 0);
}

static void expect_file_contents(const char *path, const char *expected)
{
	char actual[64];
	int fd = open(path, O_RDONLY);
	ssize_t count;

	assert(fd >= 0);
	count = read(fd, actual, sizeof(actual));
	assert(count == (ssize_t)strlen(expected));
	assert(memcmp(actual, expected, (size_t)count) == 0);
	assert(close(fd) == 0);
}

int main(void)
{
	char dir[] = "/tmp/dhcp6c-duid-policy.XXXXXX";
	char path[256];
	char target[256];
	char alias[256];
	char fifo_path[256];
	struct stat st;
	int fd;

	assert(mkdtemp(dir) != NULL);
	make_path(path, sizeof(path), dir, "duid");
	make_path(target, sizeof(target), dir, "target");
	make_path(alias, sizeof(alias), dir, "alias");
	make_path(fifo_path, sizeof(fifo_path), dir, "fifo");

	/* New DUID files are exclusive, private regular files. */
	fd = dhcp6c_duid_open_create(path);
	assert(fd >= 0);
	assert(fstat(fd, &st) == 0);
	assert(S_ISREG(st.st_mode));
	assert(st.st_uid == geteuid());
	assert(st.st_nlink == 1);
	assert((st.st_mode & 0077) == 0);
	assert(write(fd, "duid", 4) == 4);
	assert(close(fd) == 0);

	/* A second create must never truncate the existing DUID. */
	fd = dhcp6c_duid_open_create(path);
	assert(fd < 0 && errno == EEXIST);
	expect_file_contents(path, "duid");

	/* Existing private and legacy owner-readable files remain readable. */
	fd = dhcp6c_duid_open_existing(path);
	assert(fd >= 0);
	assert(close(fd) == 0);
	assert(chmod(path, 0644) == 0);
	fd = dhcp6c_duid_open_existing(path);
	assert(fd >= 0);
	assert(close(fd) == 0);

	/* Group/world-writable files are rejected. */
	assert(chmod(path, 0666) == 0);
	fd = dhcp6c_duid_open_existing(path);
	assert(fd < 0 && errno == EPERM);
	assert(chmod(path, 0600) == 0);

	/* Hard-linked DUID aliases are rejected. */
	assert(link(path, alias) == 0);
	fd = dhcp6c_duid_open_existing(path);
	assert(fd < 0 && errno == EPERM);
	assert(unlink(alias) == 0);
	fd = dhcp6c_duid_open_existing(path);
	assert(fd >= 0);
	assert(close(fd) == 0);

	/* Symlink reads fail and exclusive creation never follows the link. */
	write_file(target, "sentinel");
	assert(symlink(target, alias) == 0);
	fd = dhcp6c_duid_open_existing(alias);
	assert(fd < 0);
	fd = dhcp6c_duid_open_create(alias);
	assert(fd < 0 && errno == EEXIST);
	expect_file_contents(target, "sentinel");
	assert(unlink(alias) == 0);

	/* A FIFO cannot block the reader and is rejected as non-regular. */
	assert(mkfifo(fifo_path, 0600) == 0);
	fd = dhcp6c_duid_open_existing(fifo_path);
	assert(fd < 0 && errno == EPERM);
	assert(unlink(fifo_path) == 0);

	assert(unlink(path) == 0);
	assert(unlink(target) == 0);
	assert(rmdir(dir) == 0);
	puts("DHCPv6 DUID safe file policy: PASS");
	return 0;
}
