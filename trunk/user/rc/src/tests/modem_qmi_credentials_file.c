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

/* Inject EINTR into the credential-file sync without linker wrapping. */
static int qmi_test_fsync_interrupt_call = -1;
static int qmi_test_fsync_call_count;

static int
qmi_test_fsync(int fd)
{
	qmi_test_fsync_call_count++;
	if (qmi_test_fsync_call_count == qmi_test_fsync_interrupt_call) {
		errno = EINTR;
		return -1;
	}
	return fsync(fd);
}

#define CONFIG_FILE_SYNC_FSYNC(fd) qmi_test_fsync(fd)
#include "../modem_qmi_credentials_file.h"
#include "../../../uqmi/uqmi-private-credentials.h"

static void
test_credentials_fsync_retries_eintr(void)
{
	struct modem_qmi_credentials_file file;

	qmi_test_fsync_call_count = 0;
	qmi_test_fsync_interrupt_call = 1;
	assert(modem_qmi_credentials_file_create(NULL, "alice", "secret", &file) == 0);
	qmi_test_fsync_interrupt_call = -1;
	assert(qmi_test_fsync_call_count == 2);
	assert(modem_qmi_credentials_file_cleanup(&file) == 0);
}

static void
test_private_qmi_credentials_round_trip_and_cleanup(void)
{
	struct modem_qmi_credentials_file file;
	struct uqmi_private_credentials credentials;
	struct stat directory_stat, file_stat;
	char directory[PATH_MAX];

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
	char directory[PATH_MAX];

	assert(modem_qmi_credentials_file_create(NULL, "alice", "secret", &file) == 0);
	snprintf(directory, sizeof(directory), "%s", file.directory);
	assert(access(file.path, F_OK) == 0);
	assert(modem_qmi_credentials_file_cleanup(&file) == 0);
	assert(access(directory, F_OK) == -1 && errno == ENOENT);
}

static void
test_cleanup_refuses_replacement_inode(void)
{
	struct modem_qmi_credentials_file file;
	struct stat original_stat, replacement_stat;
	char directory[PATH_MAX];
	char contents[32] = {0};
	int original_fd, replacement_fd;

	assert(modem_qmi_credentials_file_create(NULL, "alice", "secret", &file) == 0);
	snprintf(directory, sizeof(directory), "%s", file.directory);
	original_fd = open(file.path, O_RDONLY | O_NOFOLLOW);
	assert(original_fd >= 0);
	assert(fstat(original_fd, &original_stat) == 0);

	/* Model uqmi consuming the original path, followed by a path replacement. */
	assert(unlink(file.path) == 0);
	replacement_fd = open(file.path,
			      O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
	assert(replacement_fd >= 0);
	assert(write(replacement_fd, "replacement", 11) == 11);
	assert(fsync(replacement_fd) == 0);
	assert(close(replacement_fd) == 0);
	assert(stat(file.path, &replacement_stat) == 0);
	assert(original_stat.st_dev != replacement_stat.st_dev ||
	       original_stat.st_ino != replacement_stat.st_ino);

	assert(modem_qmi_credentials_file_cleanup(&file) == EPERM);
	assert(errno == EPERM);
	assert(access(directory, F_OK) == 0);
	replacement_fd = open(file.path, O_RDONLY | O_NOFOLLOW);
	assert(replacement_fd >= 0);
	assert(read(replacement_fd, contents, sizeof(contents) - 1) == 11);
	assert(strcmp(contents, "replacement") == 0);
	assert(close(replacement_fd) == 0);

	assert(close(original_fd) == 0);
	assert(unlink(file.path) == 0);
	assert(rmdir(directory) == 0);
}

int
main(void)
{
	test_credentials_fsync_retries_eintr();
	test_private_qmi_credentials_round_trip_and_cleanup();
	test_pin_only_and_auth_only_files();
	test_injection_and_incomplete_pairs_are_rejected();
	test_cleanup_removes_unconsumed_file();
	test_cleanup_refuses_replacement_inode();
	puts("modem QMI private credential staging and parser integration: PASS");
	return 0;
}
