#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int force_read_error;

static ssize_t scut_test_read(int fd, void *buffer, size_t length)
{
	if (force_read_error) {
		errno = EIO;
		return -1;
	}
	return read(fd, buffer, length);
}

#define SCUTCLIENT_CREDENTIALS_READ scut_test_read
#include "../../trunk/user/scutclient/patches/scutclient_credentials.c"

static char test_dir[] = "/tmp/scutclient-credentials.XXXXXX";
static unsigned int sequence_number;

static void fail(const char *message)
{
	fprintf(stderr, "SCUTClient credential file test: FAIL: %s\n", message);
	exit(1);
}

static void make_path(char *buffer, size_t length, const char *suffix)
{
	int count = snprintf(buffer, length, "%s/%u-%s", test_dir, ++sequence_number, suffix);
	if (count < 0 || (size_t)count >= length)
		fail("test path overflow");
}

static void create_file(const char *path, const char *data, mode_t mode)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
	size_t length = strlen(data);
	size_t offset = 0;

	if (fd < 0)
		fail("cannot create fixture");
	if (fchmod(fd, mode) < 0)
		fail("cannot set fixture mode");
	while (offset < length) {
		ssize_t written = write(fd, data + offset, length - offset);
		if (written <= 0)
			fail("cannot write fixture");
		offset += (size_t)written;
	}
	if (close(fd) < 0)
		fail("cannot close fixture");
}

static void expect_rejected(const char *path, int path_should_remain, const char *name)
{
	char *username = NULL;
	char *password = NULL;
	int result;

	errno = 0;
	result = scutclient_load_credentials_file(path, &username, &password);
	if (result == 0) {
		free(username);
		free(password);
		fail(name);
	}
	if (username || password)
		fail("rejected credentials were returned to caller");
	if (path_should_remain) {
		if (access(path, F_OK) != 0)
			fail("rejected pre-consumption path unexpectedly disappeared");
	} else if (access(path, F_OK) == 0 || errno != ENOENT) {
		fail("consumed credential path was not unlinked");
	}
}

int main(void)
{
	char path[512];
	char target[512];
	char alias[512];
	char *username = NULL;
	char *password = NULL;
	int result;

	if (!mkdtemp(test_dir))
		fail("cannot create private test directory");
	if (chmod(test_dir, 0700) < 0)
		fail("cannot secure test directory");

	make_path(path, sizeof(path), "valid");
	create_file(path,
		"username=alice&literal\npassword=pass\\word%=tail\n", 0600);
	result = scutclient_load_credentials_file(path, &username, &password);
	if (result != 0 || !username || !password)
		fail("valid private credentials were rejected");
	if (strcmp(username, "alice&literal") != 0 ||
	    strcmp(password, "pass\\word%=tail") != 0)
		fail("valid credential values were changed");
	if (access(path, F_OK) == 0 || errno != ENOENT)
		fail("valid credential file was not unlinked");
	free(username);
	free(password);
	username = NULL;
	password = NULL;

	make_path(target, sizeof(target), "symlink-target");
	make_path(path, sizeof(path), "symlink");
	create_file(target, "username=alice\npassword=secret\n", 0600);
	if (symlink(target, path) < 0)
		fail("cannot create symlink fixture");
	expect_rejected(path, 1, "symlink credential file was accepted");
	if (access(target, F_OK) != 0)
		fail("symlink rejection changed target file");
	unlink(path);
	unlink(target);

	make_path(path, sizeof(path), "unsafe-mode");
	create_file(path, "username=alice\npassword=secret\n", 0644);
	expect_rejected(path, 1, "world-readable credential file was accepted");
	unlink(path);

	make_path(path, sizeof(path), "hardlink");
	make_path(alias, sizeof(alias), "hardlink-alias");
	create_file(path, "username=alice\npassword=secret\n", 0600);
	if (link(path, alias) < 0)
		fail("cannot create hard-link fixture");
	expect_rejected(path, 1, "multiply-linked credential file was accepted");
	unlink(path);
	unlink(alias);

	make_path(path, sizeof(path), "malformed-extra-line");
	create_file(path, "username=alice\npassword=secret\nextra=1\n", 0600);
	expect_rejected(path, 0, "credential file with extra line was accepted");

	make_path(path, sizeof(path), "empty-credentials");
	create_file(path, "username=\npassword=\n", 0600);
	result = scutclient_load_credentials_file(path, &username, &password);
	if (result != 0 || !username || !password)
		fail("empty legacy-compatible credential values were rejected");
	if (username[0] != '\0' || password[0] != '\0')
		fail("empty legacy-compatible credential values were changed");
	if (access(path, F_OK) == 0 || errno != ENOENT)
		fail("empty credential file was not unlinked");
	free(username);
	free(password);
	username = NULL;
	password = NULL;

	make_path(path, sizeof(path), "missing-final-newline");
	create_file(path, "username=alice\npassword=secret", 0600);
	expect_rejected(path, 0, "unterminated credential file was accepted");

	make_path(path, sizeof(path), "carriage-return");
	create_file(path, "username=alice\npassword=secret\r\n", 0600);
	expect_rejected(path, 0, "carriage-return credential value was accepted");

	make_path(path, sizeof(path), "read-error");
	create_file(path, "username=alice\npassword=secret\n", 0600);
	force_read_error = 1;
	errno = 0;
	result = scutclient_load_credentials_file(path, &username, &password);
	force_read_error = 0;
	if (result == 0 || errno != EIO || username || password)
		fail("credential file read failure was not rejected cleanly");
	if (access(path, F_OK) == 0 || errno != ENOENT)
		fail("read-failed credential path was not unlinked");

	make_path(path, sizeof(path), "oversized");
	create_file(path, "username=alice\npassword=secret\n", 0600);
	{
		int fd = open(path, O_WRONLY | O_APPEND);
		char padding[4096];
		memset(padding, 'x', sizeof(padding));
		if (fd < 0 || write(fd, padding, sizeof(padding)) != (ssize_t)sizeof(padding))
			fail("cannot create oversized credential fixture");
		close(fd);
	}
	expect_rejected(path, 1, "oversized credential file was accepted");
	unlink(path);


	{
		char public_dir[512];
		if (snprintf(public_dir, sizeof(public_dir), "%s/public-dir", test_dir) >= (int)sizeof(public_dir))
			fail("public directory test path overflow");
		if (mkdir(public_dir, 0755) < 0)
			fail("cannot create public directory fixture");
		if (snprintf(path, sizeof(path), "%s/credentials", public_dir) >= (int)sizeof(path))
			fail("public credential test path overflow");
		create_file(path, "username=alice\npassword=secret\n", 0600);
		expect_rejected(path, 1, "credential file in non-private directory was accepted");
		unlink(path);
		if (rmdir(public_dir) < 0)
			fail("public directory fixture was not cleaned");
	}

	{
		char target_dir[512];
		char link_dir[512];
		if (snprintf(target_dir, sizeof(target_dir), "%s/private-target", test_dir) >= (int)sizeof(target_dir) ||
		    snprintf(link_dir, sizeof(link_dir), "%s/private-link", test_dir) >= (int)sizeof(link_dir))
			fail("symlink directory test path overflow");
		if (mkdir(target_dir, 0700) < 0)
			fail("cannot create private target directory");
		if (snprintf(path, sizeof(path), "%s/credentials", target_dir) >= (int)sizeof(path))
			fail("symlink directory credential path overflow");
		create_file(path, "username=alice\npassword=secret\n", 0600);
		if (symlink(target_dir, link_dir) < 0)
			fail("cannot create directory symlink fixture");
		{
			char link_path[512];
			if (snprintf(link_path, sizeof(link_path), "%s/credentials", link_dir) >= (int)sizeof(link_path))
				fail("symlink directory path overflow");
			expect_rejected(link_path, 1, "symlink credential directory was accepted");
		}
		unlink(path);
		if (rmdir(target_dir) < 0)
			fail("private target directory was not cleaned");
		unlink(link_dir);
	}

	if (rmdir(test_dir) < 0)
		fail("credential test directory was not clean");
	puts("SCUTClient private credential file validation: PASS");
	return 0;
}
