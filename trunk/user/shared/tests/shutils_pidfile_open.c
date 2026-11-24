#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "shutils_pidfile_policy.h"

static void
make_path(char *buffer, size_t size, const char *directory, const char *name)
{
	int written = snprintf(buffer, size, "%s/%s", directory, name);
	assert(written > 0 && (size_t)written < size);
}

static void
regular_pidfile_opens(const char *path)
{
	struct stat status;
	int fd = shutils_pidfile_open(path);

	assert(fd >= 0);
	assert(fstat(fd, &status) == 0);
	assert(S_ISREG(status.st_mode));
	assert(close(fd) == 0);
}

static void
symlink_pidfile_is_rejected(const char *path)
{
	int fd;

	errno = 0;
	fd = shutils_pidfile_open(path);
	assert(fd == -1);
	assert(errno == ELOOP);
}

static void
metadata_policy_rejects_unsafe_files(void)
{
	struct stat status;

	memset(&status, 0, sizeof(status));
	status.st_mode = S_IFREG | 0600;
	status.st_uid = geteuid();
	status.st_nlink = 1;
	assert(shutils_pidfile_metadata_is_safe(&status, geteuid()));

	status.st_uid = (uid_t)(geteuid() == 0 ? 1 : 0);
	assert(!shutils_pidfile_metadata_is_safe(&status, geteuid()));
	status.st_uid = geteuid();

	status.st_nlink = 2;
	assert(!shutils_pidfile_metadata_is_safe(&status, geteuid()));
	status.st_nlink = 1;

	status.st_mode |= 0020;
	assert(!shutils_pidfile_metadata_is_safe(&status, geteuid()));
	status.st_mode = S_IFIFO | 0600;
	assert(!shutils_pidfile_metadata_is_safe(&status, geteuid()));
}

int
main(void)
{
	char directory[96];
	char real_path[128];
	char link_path[128];
	int fd;
	int written;

	written = snprintf(directory, sizeof(directory),
			   "/tmp/shutils-pidfile-%ld", (long)getpid());
	assert(written > 0 && (size_t)written < sizeof(directory));
	metadata_policy_rejects_unsafe_files();
	assert(mkdir(directory, 0700) == 0);
	make_path(real_path, sizeof(real_path), directory, "real.pid");
	make_path(link_path, sizeof(link_path), directory, "linked.pid");

	fd = open(real_path, O_WRONLY | O_CREAT | O_EXCL, 0600);
	assert(fd >= 0);
	assert(write(fd, "123\n", 4) == 4);
	assert(close(fd) == 0);
	assert(symlink(real_path, link_path) == 0);

	regular_pidfile_opens(real_path);
	symlink_pidfile_is_rejected(link_path);

	assert(unlink(link_path) == 0);
	assert(unlink(real_path) == 0);
	assert(rmdir(directory) == 0);
	puts("Privileged PID-file opens reject symlinks and unsafe owner/mode/link metadata: PASS");
	return 0;
}
