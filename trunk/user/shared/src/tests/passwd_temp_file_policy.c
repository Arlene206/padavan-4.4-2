#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "passwd_temp_file_policy.h"

static int rename_calls;
static int fail_rename_first;
static int fail_rename_second;

static int
injected_rename(const char *old_path, const char *new_path)
{
	rename_calls++;
	if (rename_calls == fail_rename_first ||
	    rename_calls == fail_rename_second) {
		errno = EIO;
		return -1;
	}
	return rename(old_path, new_path);
}

static void
write_text_file(const char *path, const char *contents)
{
	FILE *stream = fopen(path, "w");
	assert(stream != NULL);
	assert(fputs(contents, stream) >= 0);
	assert(fclose(stream) == 0);
}

static void
assert_text_file(const char *path, const char *expected)
{
	char buffer[128];
	FILE *stream = fopen(path, "r");
	assert(stream != NULL);
	assert(fgets(buffer, sizeof(buffer), stream) != NULL);
	assert(strcmp(buffer, expected) == 0);
	assert(fclose(stream) == 0);
}

struct passwd_transaction_fixture {
	char directory[256];
	char staged_passwd[320];
	char staged_group[320];
	char live_passwd[320];
	char live_group[320];
	char backup_passwd[320];
	char backup_group[320];
	passwd_file_update_t updates[2];
};

static void
fixture_init(struct passwd_transaction_fixture *fixture)
{
	char directory_template[] = "/tmp/.padavan-passwd-transaction.XXXXXX";
	char *directory;
	int had_original;

	directory = mkdtemp(directory_template);
	assert(directory != NULL);
	assert(strlen(directory) < sizeof(fixture->directory));
	strcpy(fixture->directory, directory);
	assert(snprintf(fixture->staged_passwd, sizeof(fixture->staged_passwd),
			"%s/passwd.new", fixture->directory) > 0);
	assert(snprintf(fixture->staged_group, sizeof(fixture->staged_group),
			"%s/group.new", fixture->directory) > 0);
	assert(snprintf(fixture->live_passwd, sizeof(fixture->live_passwd),
			"%s/passwd", fixture->directory) > 0);
	assert(snprintf(fixture->live_group, sizeof(fixture->live_group),
			"%s/group", fixture->directory) > 0);
	assert(snprintf(fixture->backup_passwd, sizeof(fixture->backup_passwd),
			"%s/.passwd.backup.XXXXXX", fixture->directory) > 0);
	assert(snprintf(fixture->backup_group, sizeof(fixture->backup_group),
			"%s/.group.backup.XXXXXX", fixture->directory) > 0);

	write_text_file(fixture->live_passwd, "old-passwd\n");
	write_text_file(fixture->live_group, "old-group\n");
	write_text_file(fixture->staged_passwd, "new-passwd\n");
	write_text_file(fixture->staged_group, "new-group\n");
	assert(passwd_temp_file_backup_create(fixture->live_passwd,
		fixture->backup_passwd, &had_original) == 0);
	assert(had_original == 1);
	assert(passwd_temp_file_backup_create(fixture->live_group,
		fixture->backup_group, &had_original) == 0);
	assert(had_original == 1);

	fixture->updates[0].staged_path = fixture->staged_passwd;
	fixture->updates[0].destination_path = fixture->live_passwd;
	fixture->updates[0].backup_path = fixture->backup_passwd;
	fixture->updates[0].had_original = 1;
	fixture->updates[1].staged_path = fixture->staged_group;
	fixture->updates[1].destination_path = fixture->live_group;
	fixture->updates[1].backup_path = fixture->backup_group;
	fixture->updates[1].had_original = 1;
}

static void
fixture_cleanup(struct passwd_transaction_fixture *fixture)
{
	if (fixture->backup_passwd[0])
		(void)unlink(fixture->backup_passwd);
	if (fixture->backup_group[0])
		(void)unlink(fixture->backup_group);
	(void)unlink(fixture->staged_passwd);
	(void)unlink(fixture->staged_group);
	(void)unlink(fixture->live_passwd);
	(void)unlink(fixture->live_group);
	assert(rmdir(fixture->directory) == 0);
}

static void
test_temp_file_is_exclusive_and_private(void)
{
	char first_path[] = "/tmp/.padavan-passwd-test.XXXXXX";
	char second_path[] = "/tmp/.padavan-passwd-test.XXXXXX";
	struct stat st;
	int first_fd, second_fd;
	first_fd = passwd_temp_file_open(first_path, 0600);
	assert(first_fd >= 0);
	assert(fstat(first_fd, &st) == 0);
	assert(S_ISREG(st.st_mode));
	assert((st.st_mode & 0777) == 0600);
	assert(st.st_nlink == 1);
	assert(close(first_fd) == 0);
	second_fd = passwd_temp_file_open(second_path, 0600);
	assert(second_fd >= 0);
	assert(strcmp(first_path, second_path) != 0);
	assert(fstat(second_fd, &st) == 0);
	assert((st.st_mode & 0777) == 0600);
	assert(close(second_fd) == 0);
	assert(unlink(first_path) == 0);
	assert(unlink(second_path) == 0);
}

static void
test_invalid_template_fails_closed(void)
{
	errno = 0;
	assert(passwd_temp_file_open(NULL, 0600) == -1);
	assert(errno == EINVAL);
}

static void
test_requested_mode_is_applied(void)
{
	char path[] = "/tmp/.padavan-passwd-test-mode.XXXXXX";
	struct stat st;
	int fd = passwd_temp_file_open(path, 0640);
	assert(fd >= 0);
	assert(fstat(fd, &st) == 0);
	assert((st.st_mode & 0777) == 0640);
	assert(close(fd) == 0);
	assert(unlink(path) == 0);
}

static void
test_publish_success_removes_backups(void)
{
	struct passwd_transaction_fixture fixture;
	int rollback_failed = 0;

	fixture_init(&fixture);
	rename_calls = 0;
	fail_rename_first = 0;
	fail_rename_second = 0;
	assert(passwd_file_publish_transaction(fixture.updates, 2,
		injected_rename, unlink, &rollback_failed) == 0);
	assert(rollback_failed == 0);
	assert_text_file(fixture.live_passwd, "new-passwd\n");
	assert_text_file(fixture.live_group, "new-group\n");
	assert(fixture.backup_passwd[0] == '\0');
	assert(fixture.backup_group[0] == '\0');
	assert(access(fixture.staged_passwd, F_OK) != 0);
	assert(access(fixture.staged_group, F_OK) != 0);
	fixture_cleanup(&fixture);
}

static void
test_second_rename_failure_restores_first_file(void)
{
	struct passwd_transaction_fixture fixture;
	int rollback_failed = 0;

	fixture_init(&fixture);
	rename_calls = 0;
	fail_rename_first = 2;
	fail_rename_second = 0;
	errno = 0;
	assert(passwd_file_publish_transaction(fixture.updates, 2,
		injected_rename, unlink, &rollback_failed) == -1);
	assert(errno == EIO);
	assert(rollback_failed == 0);
	assert_text_file(fixture.live_passwd, "old-passwd\n");
	assert_text_file(fixture.live_group, "old-group\n");
	assert(fixture.backup_passwd[0] == '\0');
	assert(fixture.backup_group[0] == '\0');
	assert(access(fixture.staged_group, F_OK) == 0);
	fixture_cleanup(&fixture);
}

static void
test_failed_rollback_preserves_recovery_link(void)
{
	struct passwd_transaction_fixture fixture;
	int rollback_failed = 0;

	fixture_init(&fixture);
	rename_calls = 0;
	fail_rename_first = 2;
	fail_rename_second = 3;
	errno = 0;
	assert(passwd_file_publish_transaction(fixture.updates, 2,
		injected_rename, unlink, &rollback_failed) == -1);
	assert(errno == EIO);
	assert(rollback_failed == 1);
	assert_text_file(fixture.live_passwd, "new-passwd\n");
	assert_text_file(fixture.backup_passwd, "old-passwd\n");
	assert_text_file(fixture.live_group, "old-group\n");
	assert(fixture.backup_group[0] == '\0');

	/* Simulate the documented manual recovery from the retained hard link. */
	assert(rename(fixture.backup_passwd, fixture.live_passwd) == 0);
	fixture.backup_passwd[0] = '\0';
	assert_text_file(fixture.live_passwd, "old-passwd\n");
	fixture_cleanup(&fixture);
}

static void
test_missing_original_is_removed_on_rollback(void)
{
	struct passwd_transaction_fixture fixture;
	int rollback_failed = 0, had_original;

	fixture_init(&fixture);
	/* Drop fixture_init's original hard link before simulating a missing file. */
	assert(unlink(fixture.backup_passwd) == 0);
	fixture.backup_passwd[0] = '\0';
	assert(snprintf(fixture.backup_passwd, sizeof(fixture.backup_passwd),
			"%s/.passwd-missing.XXXXXX", fixture.directory) > 0);
	assert(unlink(fixture.live_passwd) == 0);
	assert(passwd_temp_file_backup_create(fixture.live_passwd,
		fixture.backup_passwd, &had_original) == 0);
	assert(had_original == 0);
	assert(fixture.backup_passwd[0] == '\0');
	rename_calls = 0;
	fail_rename_first = 2;
	fail_rename_second = 0;
	assert(passwd_file_publish_transaction(fixture.updates, 2,
		injected_rename, unlink, &rollback_failed) == -1);
	assert(rollback_failed == 0);
	assert(access(fixture.live_passwd, F_OK) != 0);
	assert_text_file(fixture.live_group, "old-group\n");
	fixture_cleanup(&fixture);
}

int
main(void)
{
	test_temp_file_is_exclusive_and_private();
	test_invalid_template_fails_closed();
	test_requested_mode_is_applied();
	test_publish_success_removes_backups();
	test_second_rename_failure_restores_first_file();
	test_failed_rollback_preserves_recovery_link();
	test_missing_original_is_removed_on_rollback();
	puts("Secure password temporary file and account replacement policy: PASS");
	return 0;
}
