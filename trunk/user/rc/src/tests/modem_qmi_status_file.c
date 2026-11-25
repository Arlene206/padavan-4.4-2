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

#include "../modem_qmi_status_file.h"

static void
make_path(char *path, size_t size, const char *directory, const char *name)
{
	int length = snprintf(path, size, "%s/%s", directory, name);

	assert(length > 0 && (size_t)length < size);
}

static void
write_fixture(const char *path, const void *data, size_t length, mode_t mode)
{
	const char *bytes = (const char *)data;
	size_t offset = 0;
	ssize_t count;
	int fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);

	assert(fd >= 0);
	assert(fchmod(fd, mode) == 0);
	while (offset < length) {
		count = write(fd, bytes + offset, length - offset);
		if (count < 0 && errno == EINTR)
			continue;
		assert(count > 0);
		offset += (size_t)count;
	}
	assert(close(fd) == 0);
}

static void
test_qmi_status_file_metadata_policy(void)
{
	struct stat st;

	memset(&st, 0, sizeof(st));
	st.st_mode = S_IFREG | 0600;
	st.st_uid = geteuid();
	st.st_nlink = 1;
	assert(modem_qmi_status_file_metadata_status(&st) == 0);

	st.st_uid = (uid_t)(geteuid() ^ 1);
	assert(modem_qmi_status_file_metadata_status(&st) == EPERM);
	st.st_uid = geteuid();

	st.st_mode = S_IFREG | 0644;
	assert(modem_qmi_status_file_metadata_status(&st) == EPERM);
	st.st_mode = S_IFREG | 0600;

	st.st_nlink = 2;
	assert(modem_qmi_status_file_metadata_status(&st) == EPERM);
	st.st_nlink = 1;

	st.st_mode = S_IFIFO | 0600;
	assert(modem_qmi_status_file_metadata_status(&st) == EPERM);
	assert(modem_qmi_status_file_metadata_status(NULL) == EINVAL);
}

static void
test_qmi_status_file_reads_only_valid_numeric_handle(const char *directory)
{
	char path[512];
	int handle = -1;

	make_path(path, sizeof(path), directory, "valid");
	write_fixture(path, "17", 2, 0600);
	assert(modem_qmi_status_file_read(path, &handle) == 0);
	assert(handle == 17);
	assert(unlink(path) == 0);

	make_path(path, sizeof(path), directory, "malformed");
	write_fixture(path, "17junk", 6, 0600);
	handle = 99;
	assert(modem_qmi_status_file_read(path, &handle) == EINVAL);
	assert(handle == -1);
	assert(unlink(path) == 0);
}

static void
test_qmi_status_file_rejects_symlinks_and_fifo_without_blocking(const char *directory)
{
	char target[512], link_path[512], fifo_path[512];
	int handle = 71;

	make_path(target, sizeof(target), directory, "target");
	make_path(link_path, sizeof(link_path), directory, "link");
	make_path(fifo_path, sizeof(fifo_path), directory, "fifo");
	write_fixture(target, "23", 2, 0600);
	assert(symlink(target, link_path) == 0);
	assert(modem_qmi_status_file_read(link_path, &handle) == ELOOP);
	assert(handle == -1);

	assert(mkfifo(fifo_path, 0600) == 0);
	handle = 71;
	assert(modem_qmi_status_file_read(fifo_path, &handle) == EPERM);
	assert(handle == -1);

	assert(unlink(fifo_path) == 0);
	assert(unlink(link_path) == 0);
	assert(unlink(target) == 0);
}

static void
test_qmi_status_file_enforces_permissions_and_link_count(const char *directory)
{
	char path[512], alias[512];
	int handle = 71;

	make_path(path, sizeof(path), directory, "metadata");
	make_path(alias, sizeof(alias), directory, "metadata-link");
	write_fixture(path, "29", 2, 0600);

	assert(chmod(path, 0644) == 0);
	assert(modem_qmi_status_file_read(path, &handle) == EPERM);
	assert(handle == -1);
	assert(chmod(path, 0600) == 0);

	assert(link(path, alias) == 0);
	handle = 71;
	assert(modem_qmi_status_file_read(path, &handle) == EPERM);
	assert(handle == -1);

	assert(unlink(alias) == 0);
	assert(modem_qmi_status_file_read(path, &handle) == 0);
	assert(handle == 29);
	assert(unlink(path) == 0);
}

static void
test_qmi_status_file_rejects_embedded_nul_and_oversized_data(const char *directory)
{
	char path[512];
	char embedded_nul[] = { '3', '1', '\0', 'x' };
	char oversized[64];
	int handle = 71;

	make_path(path, sizeof(path), directory, "embedded-nul");
	write_fixture(path, embedded_nul, sizeof(embedded_nul), 0600);
	assert(modem_qmi_status_file_read(path, &handle) == EINVAL);
	assert(handle == -1);
	assert(unlink(path) == 0);

	make_path(path, sizeof(path), directory, "oversized");
	memset(oversized, '9', sizeof(oversized));
	write_fixture(path, oversized, sizeof(oversized), 0600);
	handle = 71;
	assert(modem_qmi_status_file_read(path, &handle) == EOVERFLOW);
	assert(handle == -1);
	assert(unlink(path) == 0);
}

static void
test_qmi_status_file_rejects_relative_missing_and_directory_paths(const char *directory)
{
	char missing[512];
	int handle = 71;

	make_path(missing, sizeof(missing), directory, "missing");
	assert(modem_qmi_status_file_read("tmp/qmi-handle", &handle) == EINVAL);
	assert(handle == -1);

	handle = 71;
	assert(modem_qmi_status_file_read(missing, &handle) == ENOENT);
	assert(handle == -1);

	handle = 71;
	assert(modem_qmi_status_file_read(directory, &handle) == EPERM);
	assert(handle == -1);
}

int
main(void)
{
	char template[] = "/tmp/modem-qmi-status.XXXXXX";
	char *directory = mkdtemp(template);

	assert(directory != NULL);
	assert(chmod(directory, 0700) == 0);
	test_qmi_status_file_metadata_policy();
	test_qmi_status_file_reads_only_valid_numeric_handle(directory);
	test_qmi_status_file_rejects_symlinks_and_fifo_without_blocking(directory);
	test_qmi_status_file_enforces_permissions_and_link_count(directory);
	test_qmi_status_file_rejects_embedded_nul_and_oversized_data(directory);
	test_qmi_status_file_rejects_relative_missing_and_directory_paths(directory);
	assert(rmdir(directory) == 0);
	puts("QMI status-file trusted reader policy: PASS");
	return 0;
}
