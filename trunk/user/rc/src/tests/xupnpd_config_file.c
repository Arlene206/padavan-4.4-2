#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "xupnpd_config_file.h"

static void
join(char *out, size_t size, const char *dir, const char *name)
{
	int n = snprintf(out, size, "%s/%s", dir, name);
	assert(n >= 0 && (size_t)n < size);
}

static void
write_text(const char *path, const char *value, mode_t mode)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
	size_t length = strlen(value), offset = 0;
	assert(fd >= 0);
	while (offset < length) {
		ssize_t n = write(fd, value + offset, length - offset);
		assert(n > 0);
		offset += (size_t)n;
	}
	assert(fchmod(fd, mode) == 0);
	assert(close(fd) == 0);
}

static void
read_text(const char *path, char *out, size_t size)
{
	FILE *fp = fopen(path, "r");
	size_t n;
	assert(fp != NULL);
	n = fread(out, 1, size - 1, fp);
	assert(!ferror(fp));
	out[n] = '\0';
	assert(fclose(fp) == 0);
}


static void
test_ensure_directory_creates_and_rejects_unsafe_paths(const char *directory)
{
	char created[PATH_MAX], real_dir[PATH_MAX], link_path[PATH_MAX];
	char regular[PATH_MAX], missing_parent[PATH_MAX];
	int fd;

	join(created, sizeof(created), directory, "created-dir");
	join(real_dir, sizeof(real_dir), directory, "real-dir");
	join(link_path, sizeof(link_path), directory, "dir-link");
	join(regular, sizeof(regular), directory, "regular-file");
	join(missing_parent, sizeof(missing_parent), directory, "missing-parent/child");

	assert(xupnpd_config_ensure_directory(created, 0755) == 0);
	assert(xupnpd_config_ensure_directory(created, 0755) == 0);
	assert(xupnpd_config_directory_validate(created) == 0);
	assert(mkdir(real_dir, 0700) == 0);
	assert(symlink(real_dir, link_path) == 0);
	fd = open(regular, O_WRONLY | O_CREAT | O_EXCL, 0600);
	assert(fd >= 0 && close(fd) == 0);
	assert(xupnpd_config_ensure_directory(link_path, 0755) == ELOOP);
	assert(xupnpd_config_ensure_directory(regular, 0755) == ENOTDIR);
	assert(xupnpd_config_ensure_directory(missing_parent, 0755) == ENOENT);
	assert(xupnpd_config_ensure_directory(NULL, 0755) == EINVAL);

	assert(unlink(link_path) == 0);
	assert(unlink(regular) == 0);
	assert(rmdir(real_dir) == 0);
	assert(rmdir(created) == 0);
}

static void
test_subdirectory_requires_expected_source_link(const char *directory)
{
	char source[PATH_MAX], alternate[PATH_MAX], expected_link[PATH_MAX];
	char wrong_link[PATH_MAX], regular[PATH_MAX], missing[PATH_MAX];
	int fd;

	join(source, sizeof(source), directory, "source-dir");
	join(alternate, sizeof(alternate), directory, "alternate-dir");
	join(expected_link, sizeof(expected_link), directory, "expected-link");
	join(wrong_link, sizeof(wrong_link), directory, "wrong-link");
	join(regular, sizeof(regular), directory, "subdir-file");
	join(missing, sizeof(missing), directory, "subdir-missing");
	assert(mkdir(source, 0700) == 0);
	assert(mkdir(alternate, 0700) == 0);
	assert(symlink(source, expected_link) == 0);
	assert(symlink(alternate, wrong_link) == 0);
	fd = open(regular, O_WRONLY | O_CREAT | O_EXCL, 0600);
	assert(fd >= 0 && close(fd) == 0);

	assert(xupnpd_config_subdirectory_validate(expected_link, source, 1) == 0);
	assert(xupnpd_config_subdirectory_validate(expected_link, source, 0) == ELOOP);
	assert(xupnpd_config_subdirectory_validate(wrong_link, source, 1) == ELOOP);
	assert(xupnpd_config_subdirectory_validate(regular, source, 1) == ENOTDIR);
	assert(xupnpd_config_subdirectory_validate(missing, source, 1) == ENOENT);
	assert(xupnpd_config_subdirectory_validate(NULL, source, 1) == EINVAL);

	assert(unlink(expected_link) == 0);
	assert(unlink(wrong_link) == 0);
	assert(unlink(regular) == 0);
	assert(rmdir(source) == 0);
	assert(rmdir(alternate) == 0);
}

static void
test_source_rejects_symlinks_and_special_files(const char *directory)
{
	char regular[PATH_MAX], symlink_path[PATH_MAX], fifo_path[PATH_MAX];
	FILE *source;

	join(regular, sizeof(regular), directory, "source.lua");
	join(symlink_path, sizeof(symlink_path), directory, "source-link.lua");
	join(fifo_path, sizeof(fifo_path), directory, "source.fifo");
	write_text(regular, "regular config\n", 0644);

	source = xupnpd_config_source_open(regular);
	assert(source != NULL);
	assert(fclose(source) == 0);

	assert(symlink(regular, symlink_path) == 0);
	errno = 0;
	assert(xupnpd_config_source_open(symlink_path) == NULL);
	assert(errno == ELOOP);

	assert(mkfifo(fifo_path, 0600) == 0);
	errno = 0;
	assert(xupnpd_config_source_open(fifo_path) == NULL);
	assert(errno == EINVAL);

	assert(unlink(symlink_path) == 0);
	assert(unlink(fifo_path) == 0);
	assert(unlink(regular) == 0);
}

static void
test_storage_directory_rejects_symlinks_and_non_directories(const char *directory)
{
	char real_dir[PATH_MAX], link_path[PATH_MAX], regular[PATH_MAX];
	char missing[PATH_MAX];
	int fd;

	join(real_dir, sizeof(real_dir), directory, "storage-real");
	join(link_path, sizeof(link_path), directory, "storage-link");
	join(regular, sizeof(regular), directory, "storage-file");
	join(missing, sizeof(missing), directory, "storage-missing");

	assert(mkdir(real_dir, 0700) == 0);
	assert(xupnpd_config_directory_validate(real_dir) == 0);
	assert(symlink(real_dir, link_path) == 0);
	assert(xupnpd_config_directory_validate(link_path) == ELOOP);
	fd = open(regular, O_WRONLY | O_CREAT | O_EXCL, 0600);
	assert(fd >= 0 && close(fd) == 0);
	assert(xupnpd_config_directory_validate(regular) == ENOTDIR);
	assert(xupnpd_config_directory_validate(missing) == ENOENT);
	assert(xupnpd_config_directory_validate(NULL) == EINVAL);

	assert(unlink(link_path) == 0);
	assert(unlink(regular) == 0);
	assert(rmdir(real_dir) == 0);
}

int
main(void)
{
	char dir[] = "/tmp/xupnpd-temp.XXXXXX";
	char destination[PATH_MAX], fixed_temp[PATH_MAX], sentinel[PATH_MAX];
	char temp1[PATH_MAX], temp2[PATH_MAX], content[256], tiny[8];
	struct stat st;
	int fd1, fd2;
	const char *bad_names[] = {
		"", "eth0';os.execute('id')", "bad interface", "eth/0",
		"interface-name-too-long"
	};
	size_t i;

	assert(mkdtemp(dir) != NULL);
	join(destination, sizeof(destination), dir, "common.lua");
	join(fixed_temp, sizeof(fixed_temp), dir, "common.lua.tmp");
	join(sentinel, sizeof(sentinel), dir, "sentinel");
	write_text(destination, "last-good\n", 0600);
	write_text(sentinel, "must-not-change\n", 0644);
	assert(symlink(sentinel, fixed_temp) == 0);

	fd1 = xupnpd_config_tempfile_open(destination, temp1, sizeof(temp1));
	assert(fd1 >= 0);
	assert(strcmp(temp1, fixed_temp) != 0);
	assert(strncmp(temp1, destination, strlen(destination)) == 0);
	assert(fstat(fd1, &st) == 0 && (st.st_mode & 0777) == 0600);
	assert(write(fd1, "new-config\n", 11) == 11);
	assert(fsync(fd1) == 0);
	assert(close(fd1) == 0);
	assert(rename(temp1, destination) == 0);

	assert(lstat(fixed_temp, &st) == 0 && S_ISLNK(st.st_mode));
	read_text(sentinel, content, sizeof(content));
	assert(strcmp(content, "must-not-change\n") == 0);
	read_text(destination, content, sizeof(content));
	assert(strcmp(content, "new-config\n") == 0);

	join(destination, sizeof(destination), dir, "parallel.lua");
	write_text(destination, "old\n", 0644);
	fd1 = xupnpd_config_tempfile_open(destination, temp1, sizeof(temp1));
	fd2 = xupnpd_config_tempfile_open(destination, temp2, sizeof(temp2));
	assert(fd1 >= 0 && fd2 >= 0 && strcmp(temp1, temp2) != 0);
	assert(close(fd1) == 0 && close(fd2) == 0);
	assert(unlink(temp1) == 0 && unlink(temp2) == 0);

	assert(xupnpd_config_tempfile_open(destination, tiny, sizeof(tiny)) == -1);
	assert(errno == ENAMETOOLONG);
	for (i = 0; i < sizeof(bad_names) / sizeof(bad_names[0]); i++)
		assert(!xupnpd_config_valid_interface(bad_names[i]));
	assert(xupnpd_config_valid_interface("eth0.2"));
	assert(xupnpd_config_valid_interface("br0"));
	assert(xupnpd_config_valid_ipv4("192.168.1.1"));
	assert(!xupnpd_config_valid_ipv4("192.168.1.1';os.execute('id')"));
	assert(!xupnpd_config_valid_ipv4("999.1.1.1"));
	test_ensure_directory_creates_and_rejects_unsafe_paths(dir);
	test_subdirectory_requires_expected_source_link(dir);
	test_source_rejects_symlinks_and_special_files(dir);
	test_storage_directory_rejects_symlinks_and_non_directories(dir);

	assert(unlink(destination) == 0);
	join(destination, sizeof(destination), dir, "common.lua");
	assert(unlink(destination) == 0);
	assert(unlink(fixed_temp) == 0);
	assert(unlink(sentinel) == 0);
	assert(rmdir(dir) == 0);
	puts("XUPnPD storage directory preparation and config-file safety regression: PASS");
	return 0;
}
