#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int read_fault;
static int read_calls;
static ssize_t scutclient_test_read(int fd, void *buffer, size_t size);
#define SCUTCLIENT_CREDENTIALS_READ scutclient_test_read
#include "../patches/scutclient_credentials.c"

static char test_directory[] = "/tmp/scutclient-credentials-test.XXXXXX";
static char symlink_directory[PATH_MAX];
static int assertions;

static int check(int condition, const char *description)
{
	++assertions;
	if (!condition) {
		fprintf(stderr, "FAIL: %s (errno=%d)\n", description, errno);
		return -1;
	}
	return 0;
}

static ssize_t scutclient_test_read(int fd, void *buffer, size_t size)
{
	if (read_fault == 1) {
		read_fault = 0;
		errno = EIO;
		return -1;
	}
	if (read_fault == 2 && read_calls++ == 0) {
		read_fault = 3;
		errno = EINTR;
		return -1;
	}
	if (read_fault == 3) {
		read_fault = 0;
		return read(fd, buffer, size);
	}
	if (read_fault == 4) {
		if (read_calls++ == 0)
			return read(fd, buffer, 1);
		return 0;
	}
	return read(fd, buffer, size);
}

static int make_file(const char *name, const void *data, size_t length,
                     mode_t mode, char *path, size_t path_size)
{
	int fd;
	size_t offset = 0;
	ssize_t amount;
	if (snprintf(path, path_size, "%s/%s", test_directory, name) >= (int)path_size)
		return -1;
	fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
	if (fd < 0)
		return -1;
	while (offset < length) {
		amount = write(fd, (const char *)data + offset, length - offset);
		if (amount < 0 && errno == EINTR)
			continue;
		if (amount <= 0) {
			close(fd);
			unlink(path);
			return -1;
		}
		offset += (size_t)amount;
	}
	if (fchmod(fd, mode) != 0 || close(fd) != 0) {
		unlink(path);
		return -1;
	}
	return 0;
}

static int expect_rejected(const char *path, int absent, const char *description)
{
	char *username = NULL;
	char *password = NULL;
	int result = scutclient_load_credentials_file(path, &username, &password);
	struct stat st;
	int exists = lstat(path, &st) == 0;
	int ok = check(result != 0, description);
	if (ok != 0)
		return -1;
	if (check(username == NULL && password == NULL, "failure must not publish partial credentials") != 0)
		return -1;
	if (check(exists == !absent, "credential pathname postcondition differs from policy") != 0)
		return -1;
	free(username);
	free(password);
	return 0;
}

static int test_success(void)
{
	char path[PATH_MAX];
	char *username = NULL;
	char *password = NULL;
	const char data[] = "username=alice&literal\\value\npassword=secret% value\n";
	if (make_file("valid", data, sizeof(data) - 1, 0600, path, sizeof(path)) != 0)
		return check(0, "create valid fixture");
	if (check(scutclient_load_credentials_file(path, &username, &password) == 0,
	          "valid file should load") != 0)
		return -1;
	if (check(strcmp(username, "alice&literal\\value") == 0,
	          "username bytes should be preserved") != 0 ||
	    check(strcmp(password, "secret% value") == 0,
	          "password bytes should be preserved") != 0 ||
	    check(access(path, F_OK) != 0 && errno == ENOENT,
	          "valid credential pathname should be unlinked before returning") != 0)
		return -1;
	free(username);
	free(password);
	return 0;
}

static int test_empty_values(void)
{
	char path[PATH_MAX];
	char *username = NULL;
	char *password = NULL;
	const char data[] = "username=\npassword=\n";
	if (make_file("empty", data, sizeof(data) - 1, 0600, path, sizeof(path)) != 0)
		return check(0, "create empty-value fixture");
	if (check(scutclient_load_credentials_file(path, &username, &password) == 0,
	          "empty legacy credentials should load") != 0)
		return -1;
	if (check(username && username[0] == '\0' && password && password[0] == '\0',
	          "empty legacy credential values should be preserved") != 0)
		return -1;
	free(username);
	free(password);
	return 0;
}

static int test_malformed(void)
{
	static const char *const cases[] = {
		"user=alice\npassword=p\n",
		"username=alice\n",
		"username=alice\npassword=p\nextra=x\n",
		"username=alice\npassword=p",
		"username=alice\r\npassword=p\n",
		"password=p\nusername=alice\n",
		"username=alice\npassword=p\npassword=extra\n"
	};
	size_t index;
	char filename[64];
	char path[PATH_MAX];
	for (index = 0; index < sizeof(cases) / sizeof(cases[0]); ++index) {
		snprintf(filename, sizeof(filename), "malformed-%lu", (unsigned long)index);
		if (make_file(filename, cases[index], strlen(cases[index]), 0600,
		              path, sizeof(path)) != 0)
			return check(0, "create malformed fixture");
		if (expect_rejected(path, 1, "malformed credential format must be rejected") != 0)
			return -1;
	}
	{
		const unsigned char nul_case[] = {
			'u','s','e','r','n','a','m','e','=','a','\0','b','\n',
			'p','a','s','s','w','o','r','d','=','p','\n'
		};
		if (make_file("malformed-nul", nul_case, sizeof(nul_case), 0600,
		              path, sizeof(path)) != 0)
			return check(0, "create NUL fixture");
		if (expect_rejected(path, 1, "embedded NUL must be rejected") != 0)
			return -1;
	}
	return 0;
}

static int test_symlink(void)
{
	char target[PATH_MAX];
	char link_path[PATH_MAX];
	const char data[] = "username=a\npassword=b\n";
	struct stat st;
	if (make_file("symlink-target", data, sizeof(data) - 1, 0600,
	              target, sizeof(target)) != 0)
		return check(0, "create symlink target");
	snprintf(link_path, sizeof(link_path), "%s/symlink", test_directory);
	if (symlink(target, link_path) != 0)
		return check(0, "create symlink fixture");
	if (expect_rejected(link_path, 0, "symlink credential source must be rejected") != 0)
		return -1;
	if (check(lstat(link_path, &st) == 0 && S_ISLNK(st.st_mode),
	          "rejected credential symlink should remain untouched") != 0 ||
	    check(access(target, F_OK) == 0, "symlink target should remain untouched") != 0)
		return -1;
	unlink(link_path);
	unlink(target);
	return 0;
}

static int test_unsafe_modes_and_links(void)
{
	char path[PATH_MAX];
	const char data[] = "username=a\npassword=b\n";
	char hardlink[PATH_MAX];
	if (make_file("unsafe-mode", data, sizeof(data) - 1, 0644,
	              path, sizeof(path)) != 0)
		return check(0, "create unsafe-mode fixture");
	if (expect_rejected(path, 0, "group/world-readable credential file must be rejected") != 0)
		return -1;
	unlink(path);
	if (make_file("hardlink-source", data, sizeof(data) - 1, 0600,
	              path, sizeof(path)) != 0)
		return check(0, "create hardlink source");
	snprintf(hardlink, sizeof(hardlink), "%s/hardlink-alias", test_directory);
	if (link(path, hardlink) != 0)
		return check(0, "create hardlink fixture");
	if (expect_rejected(path, 0, "multiply linked credential file must be rejected") != 0)
		return -1;
	if (check(access(hardlink, F_OK) == 0, "hardlink alias must remain untouched") != 0)
		return -1;
	unlink(path);
	unlink(hardlink);
	return 0;
}

static int test_unsafe_parent(void)
{
	char path[PATH_MAX];
	const char data[] = "username=a\npassword=b\n";
	if (make_file("unsafe-parent", data, sizeof(data) - 1, 0600,
	              path, sizeof(path)) != 0)
		return check(0, "create unsafe-parent fixture");
	if (chmod(test_directory, 0755) != 0)
		return check(0, "make parent directory unsafe");
	if (expect_rejected(path, 0, "non-private credential parent must be rejected") != 0) {
		chmod(test_directory, 0700);
		return -1;
	}
	if (chmod(test_directory, 0700) != 0)
		return check(0, "restore private parent permissions");
	unlink(path);
	return 0;
}

static int test_read_failures(void)
{
	char path[PATH_MAX];
	const char data[] = "username=a\npassword=b\n";
	char *username = NULL;
	char *password = NULL;
	if (make_file("read-error", data, sizeof(data) - 1, 0600,
	              path, sizeof(path)) != 0)
		return check(0, "create read-error fixture");
	read_fault = 1;
	read_calls = 0;
	if (expect_rejected(path, 1, "read I/O failure must be rejected after unlink") != 0)
		return -1;
	if (make_file("read-eintr", data, sizeof(data) - 1, 0600,
	              path, sizeof(path)) != 0)
		return check(0, "create EINTR fixture");
	read_fault = 2;
	read_calls = 0;
	if (check(scutclient_load_credentials_file(path, &username, &password) == 0,
	          "EINTR should retry without losing credentials") != 0)
		return -1;
	if (check(strcmp(username, "a") == 0 && strcmp(password, "b") == 0,
	          "EINTR retry should preserve complete credential bytes") != 0)
		return -1;
	free(username);
	free(password);
	username = NULL;
	password = NULL;
	if (make_file("read-short", data, sizeof(data) - 1, 0600,
	              path, sizeof(path)) != 0)
		return check(0, "create short-read fixture");
	read_fault = 4;
	read_calls = 0;
	if (expect_rejected(path, 1, "truncated read must be rejected") != 0)
		return -1;
	return 0;
}

static int test_oversized(void)
{
	char path[PATH_MAX];
	char data[4097];
	memset(data, 'x', sizeof(data));
	if (make_file("oversized", data, sizeof(data), 0600, path, sizeof(path)) != 0)
		return check(0, "create oversized fixture");
	if (expect_rejected(path, 0, "oversized credentials must be rejected before unlink") != 0)
		return -1;
	unlink(path);
	return 0;
}

static int test_parent_symlink(void)
{
	char path[PATH_MAX * 2];
	char target[PATH_MAX];
	const char data[] = "username=a\npassword=b\n";
	if (make_file("parent-link-target", data, sizeof(data) - 1, 0600,
	              target, sizeof(target)) != 0)
		return check(0, "create parent symlink target");
	snprintf(symlink_directory, sizeof(symlink_directory), "%s/dir-link", test_directory);
	if (symlink(test_directory, symlink_directory) != 0)
		return check(0, "create parent symlink fixture");
	snprintf(path, sizeof(path), "%s/parent-link-target", symlink_directory);
	if (expect_rejected(path, 0, "symlinked credential parent must be rejected") != 0)
		return -1;
	unlink(symlink_directory);
	unlink(target);
	return 0;
}

int main(void)
{
	if (!mkdtemp(test_directory)) {
		perror("mkdtemp");
		return 1;
	}
	if (chmod(test_directory, 0700) != 0) {
		perror("chmod test directory");
		return 1;
	}
	if (test_success() != 0 || test_empty_values() != 0 || test_malformed() != 0 ||
	    test_symlink() != 0 || test_unsafe_modes_and_links() != 0 ||
	    test_unsafe_parent() != 0 || test_read_failures() != 0 ||
	    test_oversized() != 0 || test_parent_symlink() != 0) {
		fprintf(stderr, "SCUTClient credential parser: FAIL after %d assertions\n", assertions);
		rmdir(test_directory);
		return 1;
	}
	rmdir(test_directory);
	printf("SCUTClient credential file parser security regression: PASS (%d assertions)\n", assertions);
	return 0;
}
