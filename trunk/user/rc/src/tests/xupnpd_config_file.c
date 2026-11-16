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
	test_source_rejects_symlinks_and_special_files(dir);

	assert(unlink(destination) == 0);
	join(destination, sizeof(destination), dir, "common.lua");
	assert(unlink(destination) == 0);
	assert(unlink(fixed_temp) == 0);
	assert(unlink(sentinel) == 0);
	assert(rmdir(dir) == 0);
	puts("XUPnPD unique temporary-file and input validation regression: PASS");
	return 0;
}
