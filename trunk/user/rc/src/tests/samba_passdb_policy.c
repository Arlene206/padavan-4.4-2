#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "samba_passdb_policy.h"

static void
test_account_list_snapshot_source(void)
{
	int use_snapshot = -1;

	assert(samba_passdb_account_list_source_status(0, 0, 16, &use_snapshot) == 0);
	assert(use_snapshot == 0);
	assert(samba_passdb_account_list_source_status(1, 0, 16, &use_snapshot) == 0);
	assert(use_snapshot == 1);
	assert(samba_passdb_account_list_source_status(1, 16, 16, &use_snapshot) == 0);
	assert(use_snapshot == 1);
	assert(samba_passdb_account_list_source_status(1, -1, 16, &use_snapshot) == EINVAL);
	assert(samba_passdb_account_list_source_status(1, 17, 16, &use_snapshot) == EINVAL);
	assert(samba_passdb_account_list_source_status(0, 1, 16, &use_snapshot) == EINVAL);
	assert(samba_passdb_account_list_source_status(0, 0, -1, &use_snapshot) == EINVAL);
	assert(samba_passdb_account_list_source_status(0, 0, 16, NULL) == EINVAL);
}

int
main(void)
{
	char max_password[128];
	char overlong_password[129];
	char utf8_password[] = "router-密码-2026";

	test_account_list_snapshot_source();

	memset(max_password, 'x', sizeof(max_password) - 1);
	max_password[sizeof(max_password) - 1] = '\0';
	memset(overlong_password, 'y', sizeof(overlong_password) - 1);
	overlong_password[sizeof(overlong_password) - 1] = '\0';

	assert(!samba_passdb_password_is_safe(NULL));
	assert(!samba_passdb_password_is_safe(""));
	assert(samba_passdb_password_is_safe("normal-password"));
	assert(samba_passdb_password_is_safe(utf8_password));
	assert(samba_passdb_password_is_safe(max_password));
	assert(!samba_passdb_password_is_safe(overlong_password));
	assert(!samba_passdb_password_is_safe("line\nbreak"));
	assert(!samba_passdb_password_is_safe("line\rbreak"));
	assert(!samba_passdb_password_is_safe("tab\tbreak"));
	assert(!samba_passdb_password_is_safe("control\001byte"));
	assert(!samba_passdb_password_is_safe("delete\177byte"));

	/* A failed account-count read must never masquerade as zero users. */
	{
		int count = 99;

		assert(samba_passdb_account_count_status(0, 0, 16, &count) == 0);
		assert(count == 0);
		assert(samba_passdb_account_count_status(0, 7, 16, &count) == 0);
		assert(count == 7);
		assert(samba_passdb_account_count_status(0, -1, 16, &count) == 0);
		assert(count == 0);
		assert(samba_passdb_account_count_status(0, 99, 16, &count) == 0);
		assert(count == 16);
		count = 99;
		assert(samba_passdb_account_count_status(EIO, 0, 16, &count) == EIO);
		assert(count == 0);
		assert(samba_passdb_account_count_status(EACCES, 8, 16, &count) == EACCES);
		assert(count == 0);
		assert(samba_passdb_account_count_status(-1, 8, 16, &count) == EIO);
		assert(count == 0);
		assert(samba_passdb_account_count_status(0, 8, -1, &count) == EINVAL);
		assert(samba_passdb_account_count_status(0, 8, 16, NULL) == EINVAL);
	}

	/* Transaction end is mandatory; retain a previous read error. */
	assert(samba_passdb_snapshot_end_status(0, 0, 0) == 0);
	assert(samba_passdb_snapshot_end_status(0, -1, EACCES) == EACCES);
	assert(samba_passdb_snapshot_end_status(0, -1, 0) == EIO);
	assert(samba_passdb_snapshot_end_status(0, EBUSY, 0) == EBUSY);
	assert(samba_passdb_snapshot_end_status(EACCES, -1, EIO) == EACCES);
	assert(samba_passdb_snapshot_end_status(-1, 0, 0) == EIO);

	/*
	 * A failed paired configuration publish must be able to restore the old
	 * passdb without copying credential hashes into another file.
	 */
	{
		char dir[] = "/tmp/samba-passdb-policy.XXXXXX";
		char passdb[256], backup_template[256], backup[256];
		char candidate[256], directory_path[256], symlink_path[256];
		char contents[128];
		FILE *fp;
		int had_original, n;

		assert(mkdtemp(dir) != NULL);
		n = snprintf(passdb, sizeof(passdb), "%s/smbpasswd", dir);
		assert(n > 0 && (size_t)n < sizeof(passdb));
		n = snprintf(backup_template, sizeof(backup_template),
			     "%s/.passdb-backup.XXXXXX", dir);
		assert(n > 0 && (size_t)n < sizeof(backup_template));
		n = snprintf(candidate, sizeof(candidate), "%s/new-passdb", dir);
		assert(n > 0 && (size_t)n < sizeof(candidate));
		n = snprintf(directory_path, sizeof(directory_path), "%s/not-a-file", dir);
		assert(n > 0 && (size_t)n < sizeof(directory_path));
		n = snprintf(symlink_path, sizeof(symlink_path), "%s/passdb-link", dir);
		assert(n > 0 && (size_t)n < sizeof(symlink_path));

		/* Existing database: snapshot, replace, then restore previous bytes/mode. */
		fp = fopen(passdb, "w");
		assert(fp != NULL);
		assert(fputs("old account hash\n", fp) >= 0);
		assert(fclose(fp) == 0);
		assert(chmod(passdb, 0600) == 0);
		assert(samba_passdb_backup_file(passdb, backup_template,
						backup, sizeof(backup),
						&had_original) == 0);
		assert(had_original == 1);
		assert(access(backup, F_OK) == 0);

		fp = fopen(candidate, "w");
		assert(fp != NULL);
		assert(fputs("new account hash\n", fp) >= 0);
		assert(fclose(fp) == 0);
		assert(rename(candidate, passdb) == 0);
		assert(samba_passdb_restore_file(passdb, backup, had_original) == 0);
		fp = fopen(passdb, "r");
		assert(fp != NULL);
		assert(fgets(contents, sizeof(contents), fp) != NULL);
		assert(fclose(fp) == 0);
		assert(strcmp(contents, "old account hash\n") == 0);
		assert(access(backup, F_OK) != 0);

		/* First start: rollback removes a database that did not exist before. */
		assert(samba_passdb_backup_file(candidate, backup_template,
						backup, sizeof(backup),
						&had_original) == 0);
		assert(had_original == 0);
		fp = fopen(candidate, "w");
		assert(fp != NULL);
		assert(fputs("new first-run database\n", fp) >= 0);
		assert(fclose(fp) == 0);
		assert(samba_passdb_restore_file(candidate, backup, had_original) == 0);
		assert(access(candidate, F_OK) != 0);

		/* Never snapshot a directory or symbolic-link database target. */
		assert(mkdir(directory_path, 0700) == 0);
		errno = 0;
		assert(samba_passdb_backup_file(directory_path, backup_template,
						backup, sizeof(backup),
						&had_original) == -1);
		assert(errno == EINVAL);
		assert(symlink(passdb, symlink_path) == 0);
		errno = 0;
		assert(samba_passdb_backup_file(symlink_path, backup_template,
						backup, sizeof(backup),
						&had_original) == -1);
		assert(errno == EINVAL);

		assert(unlink(symlink_path) == 0);
		assert(rmdir(directory_path) == 0);
		assert(unlink(passdb) == 0);
		assert(rmdir(dir) == 0);
	}

	puts("Samba passdb validation and rollback snapshot policy: PASS");
	return 0;
}
