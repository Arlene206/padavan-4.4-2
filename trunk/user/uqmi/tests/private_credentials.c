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

#include "../uqmi-private-credentials.h"

static void
make_path(char *path, size_t size, const char *directory, const char *name)
{
	int length = snprintf(path, size, "%s/%s", directory, name);
	assert(length > 0 && (size_t)length < size);
}

static void
write_file(const char *path, const char *contents, mode_t mode)
{
	int fd;
	size_t length = strlen(contents), offset = 0;
	ssize_t count;

	fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
	assert(fd >= 0);
	while (offset < length) {
		count = write(fd, contents + offset, length - offset);
		assert(count > 0);
		offset += (size_t)count;
	}
	assert(fchmod(fd, mode) == 0);
	assert(close(fd) == 0);
}

static void
expect_rejected(const char *directory, const char *name,
		const char *contents, mode_t mode, int consumed)
{
	char path[512];
	struct uqmi_private_credentials credentials;

	make_path(path, sizeof(path), directory, name);
	write_file(path, contents, mode);
	memset(&credentials, 0xa5, sizeof(credentials));
	assert(uqmi_private_credentials_load(path, &credentials) == -1);
	assert(credentials.pin[0] == '\0');
	assert(credentials.username[0] == '\0');
	assert(credentials.password[0] == '\0');
	if (consumed)
		assert(access(path, F_OK) == -1 && errno == ENOENT);
	else
		assert(access(path, F_OK) == 0);
	uqmi_private_credentials_clear(&credentials);
}

static void
test_pin_and_auth_files_are_consumed_before_return(const char *directory)
{
	char path[512];
	struct uqmi_private_credentials credentials;

	make_path(path, sizeof(path), directory, "pin");
	write_file(path, "pin=1234\n", 0600);
	assert(uqmi_private_credentials_load(path, &credentials) == 0);
	assert(credentials.has_pin == 1);
	assert(strcmp(credentials.pin, "1234") == 0);
	assert(!credentials.has_username && !credentials.has_password);
	assert(access(path, F_OK) == -1 && errno == ENOENT);
	uqmi_private_credentials_clear(&credentials);

	make_path(path, sizeof(path), directory, "auth");
	write_file(path, "username=alice\npassword=secret\n", 0600);
	assert(uqmi_private_credentials_load(path, &credentials) == 0);
	assert(!credentials.has_pin);
	assert(credentials.has_username && credentials.has_password);
	assert(strcmp(credentials.username, "alice") == 0);
	assert(strcmp(credentials.password, "secret") == 0);
	assert(access(path, F_OK) == -1 && errno == ENOENT);
	uqmi_private_credentials_clear(&credentials);
}

static void
test_empty_legacy_auth_values_are_preserved(const char *directory)
{
	char path[512];
	struct uqmi_private_credentials credentials;

	make_path(path, sizeof(path), directory, "empty-auth");
	write_file(path, "username=\npassword=\n", 0600);
	assert(uqmi_private_credentials_load(path, &credentials) == 0);
	assert(credentials.has_username && credentials.has_password);
	assert(credentials.username[0] == '\0');
	assert(credentials.password[0] == '\0');
	assert(access(path, F_OK) == -1 && errno == ENOENT);
	uqmi_private_credentials_clear(&credentials);
}

static void
test_malformed_files_are_consumed_but_not_accepted(const char *directory)
{
	expect_rejected(directory, "duplicate", "pin=1234\npin=5678\n", 0600, 1);
	expect_rejected(directory, "unknown", "token=secret\n", 0600, 1);
	expect_rejected(directory, "one-half", "username=alice\n", 0600, 1);
	expect_rejected(directory, "mixed-services", "pin=1234\nusername=alice\npassword=secret\n", 0600, 1);
	expect_rejected(directory, "missing-lf", "pin=1234", 0600, 1);
	expect_rejected(directory, "cr", "pin=12\r34\n", 0600, 1);
	expect_rejected(directory, "empty-pin", "pin=\n", 0600, 1);
	expect_rejected(directory, "extra", "pin=1234\nignored\n", 0600, 1);
}

static void
test_file_mode_and_link_count_are_checked(const char *directory)
{
	char path[512], second[512];

	make_path(path, sizeof(path), directory, "wrong-mode");
	write_file(path, "pin=1234\n", 0644);
	{
		struct uqmi_private_credentials credentials;
		assert(uqmi_private_credentials_load(path, &credentials) == -1);
		assert(access(path, F_OK) == 0);
		uqmi_private_credentials_clear(&credentials);
	}
	assert(unlink(path) == 0);

	make_path(path, sizeof(path), directory, "hardlink");
	make_path(second, sizeof(second), directory, "hardlink-alias");
	write_file(path, "pin=1234\n", 0600);
	assert(link(path, second) == 0);
	{
		struct uqmi_private_credentials credentials;
		assert(uqmi_private_credentials_load(path, &credentials) == -1);
		assert(access(path, F_OK) == 0);
		assert(access(second, F_OK) == 0);
		uqmi_private_credentials_clear(&credentials);
	}
	assert(unlink(path) == 0);
	assert(unlink(second) == 0);
}

static void
test_symlinks_are_not_followed(const char *directory)
{
	char path[512], target[512];
	struct uqmi_private_credentials credentials;

	make_path(path, sizeof(path), directory, "symlink");
	make_path(target, sizeof(target), directory, "symlink-target");
	write_file(target, "pin=1234\n", 0600);
	assert(symlink(target, path) == 0);
	assert(uqmi_private_credentials_load(path, &credentials) == -1);
	assert(access(target, F_OK) == 0);
	assert(unlink(path) == 0);
	assert(unlink(target) == 0);
	uqmi_private_credentials_clear(&credentials);
}

static void
test_parent_directory_mode_is_checked(void)
{
	char child[] = "/tmp/uqmi-private-test.XXXXXX";
	char path[512];
	struct uqmi_private_credentials credentials;
	char *created = mkdtemp(child);

	assert(created != NULL);
	assert(chmod(created, 0755) == 0);
	make_path(path, sizeof(path), created, "credentials");
	write_file(path, "pin=1234\n", 0600);
	assert(uqmi_private_credentials_load(path, &credentials) == -1);
	assert(access(path, F_OK) == 0);
	uqmi_private_credentials_clear(&credentials);
	assert(unlink(path) == 0);
	assert(rmdir(created) == 0);
}

int
main(void)
{
	char template[] = "/tmp/uqmi-private-credentials.XXXXXX";
	char *directory = mkdtemp(template);

	assert(directory != NULL);
	assert(chmod(directory, 0700) == 0);
	test_pin_and_auth_files_are_consumed_before_return(directory);
	test_empty_legacy_auth_values_are_preserved(directory);
	test_malformed_files_are_consumed_but_not_accepted(directory);
	test_file_mode_and_link_count_are_checked(directory);
	test_symlinks_are_not_followed(directory);
	test_parent_directory_mode_is_checked();
	assert(rmdir(directory) == 0);
	puts("uqmi private one-shot credentials parser: PASS");
	return 0;
}
