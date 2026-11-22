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

#include "../modem_qmi_credentials_file.h"
#include "../../../uqmi/uqmi-private-credentials.h"

static void
test_private_qmi_credentials_round_trip_and_cleanup(void)
{
	struct modem_qmi_credentials_file file;
	struct uqmi_private_credentials credentials;
	struct stat directory_stat, file_stat;
	char directory[512];

	assert(modem_qmi_credentials_file_create(NULL, "alice", "secret",
						 &file) == 0);
	assert(file.directory[0] && file.path[0]);
	snprintf(directory, sizeof(directory), "%s", file.directory);
	assert(stat(file.directory, &directory_stat) == 0);
	assert(S_ISDIR(directory_stat.st_mode));
	assert(directory_stat.st_uid == geteuid());
	assert((directory_stat.st_mode & 07777) == 0700);
	assert(stat(file.path, &file_stat) == 0);
	assert(S_ISREG(file_stat.st_mode));
	assert(file_stat.st_uid == geteuid());
	assert((file_stat.st_mode & 07777) == 0600);
	assert(file_stat.st_nlink == 1);

	assert(uqmi_private_credentials_load(file.path, &credentials) == 0);
	assert(!credentials.has_pin);
	assert(credentials.has_username && strcmp(credentials.username, "alice") == 0);
	assert(credentials.has_password && strcmp(credentials.password, "secret") == 0);
	assert(access(file.path, F_OK) == -1 && errno == ENOENT);
	uqmi_private_credentials_clear(&credentials);
	assert(modem_qmi_credentials_file_cleanup(&file) == 0);
	assert(access(directory, F_OK) == -1 && errno == ENOENT);
}

static void
test_pin_only_and_auth_only_files(void)
{
	struct modem_qmi_credentials_file file;
	struct uqmi_private_credentials credentials;

	assert(modem_qmi_credentials_file_create("4321", NULL, NULL, &file) == 0);
	assert(uqmi_private_credentials_load(file.path, &credentials) == 0);
	assert(credentials.has_pin && !credentials.has_username && !credentials.has_password);
	uqmi_private_credentials_clear(&credentials);
	assert(modem_qmi_credentials_file_cleanup(&file) == 0);

	assert(modem_qmi_credentials_file_create(NULL, "bob", "pass", &file) == 0);
	assert(uqmi_private_credentials_load(file.path, &credentials) == 0);
	assert(!credentials.has_pin && credentials.has_username && credentials.has_password);
	assert(strcmp(credentials.username, "bob") == 0);
	assert(strcmp(credentials.password, "pass") == 0);
	uqmi_private_credentials_clear(&credentials);
	assert(modem_qmi_credentials_file_cleanup(&file) == 0);
}

static void
test_injection_and_incomplete_pairs_are_rejected(void)
{
	struct modem_qmi_credentials_file file;

	assert(modem_qmi_credentials_file_create("1234\n--reset-dms", NULL, NULL,
						 &file) == -1);
	assert(errno == EINVAL);
	assert(modem_qmi_credentials_file_create(NULL, "alice\r--reset-dms", "secret",
						 &file) == -1);
	assert(errno == EINVAL);
	assert(modem_qmi_credentials_file_create(NULL, "alice", NULL, &file) == -1);
	assert(errno == EINVAL);
	assert(modem_qmi_credentials_file_create(NULL, NULL, NULL, &file) == -1);
	assert(errno == EINVAL);
}

static void
test_cleanup_removes_unconsumed_file(void)
{
	struct modem_qmi_credentials_file file;
	char directory[512];

	assert(modem_qmi_credentials_file_create(NULL, "alice", "secret", &file) == 0);
	snprintf(directory, sizeof(directory), "%s", file.directory);
	assert(access(file.path, F_OK) == 0);
	assert(modem_qmi_credentials_file_cleanup(&file) == 0);
	assert(access(directory, F_OK) == -1 && errno == ENOENT);
}

int
main(void)
{
	test_private_qmi_credentials_round_trip_and_cleanup();
	test_pin_only_and_auth_only_files();
	test_injection_and_incomplete_pairs_are_rejected();
	test_cleanup_removes_unconsumed_file();
	puts("modem QMI private credential staging and parser integration: PASS");
	return 0;
}
