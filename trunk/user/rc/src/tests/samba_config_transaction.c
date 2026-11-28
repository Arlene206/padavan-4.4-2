#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* One injected EINTR verifies every transaction durability boundary. */
static int transaction_fsync_interrupt_call = -1;
static int transaction_fsync_call_count;

static int
samba_transaction_test_fsync(int fd)
{
	transaction_fsync_call_count++;
	if (transaction_fsync_call_count == transaction_fsync_interrupt_call) {
		errno = EINTR;
		return -1;
	}
	return fsync(fd);
}

#define CONFIG_FILE_SYNC_FSYNC(fd) samba_transaction_test_fsync(fd)
#include "samba_config_transaction.h"

static void
write_text(const char *path, const char *text, mode_t mode)
{
	FILE *fp = fopen(path, "w");

	assert(fp != NULL);
	assert(fputs(text, fp) >= 0);
	assert(fclose(fp) == 0);
	assert(chmod(path, mode) == 0);
}

static void
replace_text(const char *path, const char *temporary,
             const char *text, mode_t mode)
{
	write_text(temporary, text, mode);
	assert(rename(temporary, path) == 0);
}

static void
assert_text(const char *path, const char *expected)
{
	char buffer[256];
	FILE *fp = fopen(path, "r");

	assert(fp != NULL);
	assert(fgets(buffer, sizeof(buffer), fp) != NULL);
	assert(fclose(fp) == 0);
	assert(strcmp(buffer, expected) == 0);
}

static void
make_path(char *path, size_t size, const char *directory, const char *leaf)
{
	int length = snprintf(path, size, "%s/%s", directory, leaf);

	assert(length > 0 && (size_t)length < size);
}

static void
test_transaction_sync_retries_eintr(void)
{
	char root[] = "/tmp/samba-config-eintr.XXXXXX";
	char directory[256], passdb[256], state[256];

	assert(mkdtemp(root) != NULL);
	make_path(directory, sizeof(directory), root, "transaction");
	make_path(passdb, sizeof(passdb), root, "smbpasswd");
	make_path(state, sizeof(state), directory, "state");
	assert(mkdir(directory, 0700) == 0);
	write_text(passdb, "private hashes\n", 0600);

	transaction_fsync_call_count = 0;
	transaction_fsync_interrupt_call = 1;
	assert(samba_config_transaction_sync_directory(directory) == 0);
	transaction_fsync_interrupt_call = -1;
	assert(transaction_fsync_call_count == 2);

	transaction_fsync_call_count = 0;
	transaction_fsync_interrupt_call = 1;
	assert(samba_config_transaction_sync_file(passdb) == 0);
	transaction_fsync_interrupt_call = -1;
	assert(transaction_fsync_call_count == 2);

	transaction_fsync_call_count = 0;
	transaction_fsync_interrupt_call = 1;
	assert(samba_config_transaction_prepare_parent_directory(directory) == 0);
	transaction_fsync_interrupt_call = -1;
	assert(transaction_fsync_call_count == 2);

	transaction_fsync_call_count = 0;
	transaction_fsync_interrupt_call = 1;
	assert(samba_config_transaction_write_state(directory, 1, 1, "PREPARED") == 0);
	transaction_fsync_interrupt_call = -1;
	assert(transaction_fsync_call_count == 3);
	assert(access(state, F_OK) == 0);

	assert(unlink(state) == 0);
	assert(unlink(passdb) == 0);
	assert(rmdir(directory) == 0);
	assert(rmdir(root) == 0);
}

static void
test_private_file_sync_helpers(void)
{
	char root[] = "/tmp/samba-config-file-sync.XXXXXX";
	char passdb[256], directory[256], public_file[256], link_path[256];
	char missing_path[256];

	assert(mkdtemp(root) != NULL);
	make_path(passdb, sizeof(passdb), root, "smbpasswd");
	make_path(directory, sizeof(directory), root, "directory");
	make_path(public_file, sizeof(public_file), root, "public-file");
	make_path(link_path, sizeof(link_path), root, "passdb-link");
	make_path(missing_path, sizeof(missing_path), root, "missing");
	assert(mkdir(directory, 0700) == 0);

	write_text(passdb, "private credential hashes\n", 0600);
	assert(samba_config_transaction_sync_file(passdb) == 0);

	/* Only a private regular file may be synchronized as passdb state. */
	errno = 0;
	assert(samba_config_transaction_sync_file(directory) == -1);
	assert(errno == EISDIR || errno == EACCES);
	write_text(public_file, "not private\n", 0644);
	errno = 0;
	assert(samba_config_transaction_sync_file(public_file) == -1);
	assert(errno == EACCES);
	/* Special permission bits are not part of the private 0600 contract. */
	assert(chmod(public_file, 04600) == 0);
	errno = 0;
	assert(samba_config_transaction_sync_file(public_file) == -1);
	assert(errno == EACCES);
	assert(symlink(passdb, link_path) == 0);
	errno = 0;
	assert(samba_config_transaction_sync_file(link_path) == -1);
	assert(errno == ELOOP || errno == ENOTSUP);
	errno = 0;
	assert(samba_config_transaction_sync_file(missing_path) == -1);
	assert(errno == ENOENT);

	assert(unlink(link_path) == 0);
	assert(unlink(public_file) == 0);
	assert(unlink(passdb) == 0);
	assert(rmdir(directory) == 0);
	assert(rmdir(root) == 0);
}


static void
test_directory_sync_helpers(void)
{
	char root[] = "/tmp/samba-config-sync.XXXXXX";
	char directory[256], regular_file[256], directory_link[256];
	char linked_child[256], missing_child[256];
	FILE *fp;

	assert(mkdtemp(root) != NULL);
	make_path(directory, sizeof(directory), root, "private");
	make_path(regular_file, sizeof(regular_file), root, "state-file");
	make_path(directory_link, sizeof(directory_link), root, "directory-link");
	assert(mkdir(directory, 0700) == 0);
	assert(samba_config_transaction_sync_directory(root) == 0);
	assert(samba_config_transaction_sync_directory(directory) == 0);
	assert(samba_config_transaction_sync_parent(directory) == 0);

	fp = fopen(regular_file, "w");
	assert(fp != NULL);
	assert(fputs("durable parent\n", fp) >= 0);
	assert(fclose(fp) == 0);
	assert(samba_config_transaction_sync_parent(regular_file) == 0);
	errno = 0;
	assert(samba_config_transaction_sync_directory(regular_file) == -1);
	assert(errno == ENOTDIR || errno == EINVAL);

	assert(symlink(directory, directory_link) == 0);
	errno = 0;
	assert(samba_config_transaction_sync_directory(directory_link) == -1);
	make_path(linked_child, sizeof(linked_child), directory_link, "state");
	make_path(missing_child, sizeof(missing_child), root, "missing/state");
	errno = 0;
	assert(samba_config_transaction_sync_parent(linked_child) == -1);
	assert(errno == ELOOP || errno == ENOTDIR);
	assert(samba_config_transaction_sync_parent(missing_child) == -1);
	assert(errno == ENOENT);

	assert(unlink(directory_link) == 0);
	assert(unlink(regular_file) == 0);
	assert(rmdir(directory) == 0);
	assert(rmdir(root) == 0);
}

static void
test_parent_directory_permissions(void)
{
	char root[] = "/tmp/samba-config-parent.XXXXXX";
	char directory[256], regular_file[256], link_path[256];
	struct stat st;
	mode_t old_umask;

	assert(mkdtemp(root) != NULL);
	make_path(directory, sizeof(directory), root, "samba");
	make_path(regular_file, sizeof(regular_file), root, "not-a-directory");
	make_path(link_path, sizeof(link_path), root, "directory-link");

	/* PID 1 uses a permissive umask; private state parents must still be 0700. */
	old_umask = umask(0000);
	assert(samba_config_transaction_prepare_parent_directory(directory) == 0);
	umask(old_umask);
	assert(lstat(directory, &st) == 0);
	assert(S_ISDIR(st.st_mode));
	assert(st.st_uid == geteuid());
	assert((st.st_mode & 07777) == 0700);

	/* Repair an existing legacy 0777 directory without broadening access. */
	assert(chmod(directory, 0777) == 0);
	assert(samba_config_transaction_prepare_parent_directory(directory) == 0);
	assert(lstat(directory, &st) == 0);
	assert((st.st_mode & 07777) == 0700);

	/* Remove even execute-only access to private passdb/recovery state. */
	assert(chmod(directory, 0711) == 0);
	assert(samba_config_transaction_prepare_parent_directory(directory) == 0);
	assert(lstat(directory, &st) == 0);
	assert((st.st_mode & 07777) == 0700);

	write_text(regular_file, "not a directory\n", 0600);
	assert(samba_config_transaction_prepare_parent_directory(regular_file) == -1);
	assert(symlink(directory, link_path) == 0);
	assert(samba_config_transaction_prepare_parent_directory(link_path) == -1);

	assert(unlink(link_path) == 0);
	assert(unlink(regular_file) == 0);
	assert(rmdir(directory) == 0);
	assert(rmdir(root) == 0);
}


static void
test_account_files_share_the_passdb_config_transaction(void)
{
	char root[] = "/tmp/samba-config-accounts.XXXXXX";
	char transaction[256], passdb[256], config[256];
	char passwd[256], group[256], passdb_next[256], config_next[256];
	char passwd_next[256], group_next[256];

	assert(mkdtemp(root) != NULL);
	make_path(transaction, sizeof(transaction), root, ".transaction");
	make_path(passdb, sizeof(passdb), root, "smbpasswd");
	make_path(config, sizeof(config), root, "smb.conf");
	make_path(passwd, sizeof(passwd), root, "passwd");
	make_path(group, sizeof(group), root, "group");
	make_path(passdb_next, sizeof(passdb_next), root, "smbpasswd.next");
	make_path(config_next, sizeof(config_next), root, "smb.conf.next");
	make_path(passwd_next, sizeof(passwd_next), root, "passwd.next");
	make_path(group_next, sizeof(group_next), root, "group.next");

	write_text(passdb, "old passdb\n", 0600);
	write_text(config, "old config\n", 0644);
	write_text(passwd, "old passwd\n", 0644);
	write_text(group, "old group\n", 0644);

	/* A failure after all four replacements rolls every store back. */
	assert(samba_config_transaction_begin_with_accounts(
		transaction, passdb, config, passwd, group) == 0);
	replace_text(passdb, passdb_next, "new passdb\n", 0600);
	replace_text(config, config_next, "new config\n", 0644);
	replace_text(passwd, passwd_next, "new passwd\n", 0644);
	replace_text(group, group_next, "new group\n", 0644);
	assert(samba_config_transaction_abort_with_accounts(
		transaction, passdb, config, passwd, group) == 0);
	assert_text(passdb, "old passdb\n");
	assert_text(config, "old config\n");
	assert_text(passwd, "old passwd\n");
	assert_text(group, "old group\n");
	assert(access(transaction, F_OK) != 0);

	/* COMMITTED makes all four newly published files authoritative. */
	assert(samba_config_transaction_begin_with_accounts(
		transaction, passdb, config, passwd, group) == 0);
	replace_text(passdb, passdb_next, "committed passdb\n", 0600);
	replace_text(config, config_next, "committed config\n", 0644);
	replace_text(passwd, passwd_next, "committed passwd\n", 0644);
	replace_text(group, group_next, "committed group\n", 0644);
	assert(samba_config_transaction_commit_with_accounts(
		transaction, passdb, config, passwd, group) == 0);
	assert_text(passdb, "committed passdb\n");
	assert_text(config, "committed config\n");
	assert_text(passwd, "committed passwd\n");
	assert_text(group, "committed group\n");
	assert(samba_config_transaction_recover_with_accounts(
		transaction, passdb, config, passwd, group) == 0);

	/* A visible COMMITTED marker survives a crash before snapshot cleanup. */
	assert(samba_config_transaction_begin_with_accounts(
		transaction, passdb, config, passwd, group) == 0);
	replace_text(passdb, passdb_next, "marker passdb\n", 0600);
	replace_text(config, config_next, "marker config\n", 0644);
	replace_text(passwd, passwd_next, "marker passwd\n", 0644);
	replace_text(group, group_next, "marker group\n", 0644);
	assert(samba_config_transaction_write_state_with_accounts(
		transaction, 1, 1, 1, 1, "COMMITTED") == 0);
	assert(samba_config_transaction_recover_with_accounts(
		transaction, passdb, config, passwd, group) == 0);
	assert_text(passdb, "marker passdb\n");
	assert_text(config, "marker config\n");
	assert_text(passwd, "marker passwd\n");
	assert_text(group, "marker group\n");

	/* Missing original Unix files are removed on an interrupted first boot. */
	assert(unlink(group) == 0);
	assert(samba_config_transaction_begin_with_accounts(
		transaction, passdb, config, passwd, group) == 0);
	replace_text(passdb, passdb_next, "uncommitted passdb\n", 0600);
	replace_text(config, config_next, "uncommitted config\n", 0644);
	replace_text(passwd, passwd_next, "uncommitted passwd\n", 0644);
	write_text(group, "new first-boot group\n", 0644);
	assert(samba_config_transaction_recover_with_accounts(
		transaction, passdb, config, passwd, group) == 0);
	assert_text(passdb, "marker passdb\n");
	assert_text(config, "marker config\n");
	assert_text(passwd, "marker passwd\n");
	assert(access(group, F_OK) != 0);

	/* The new entry point must still recover an interrupted legacy V1 pair. */
	assert(samba_config_transaction_begin(
		transaction, passdb, config) == 0);
	replace_text(passdb, passdb_next, "legacy uncommitted passdb\n", 0600);
	replace_text(config, config_next, "legacy uncommitted config\n", 0644);
	assert(samba_config_transaction_recover_with_accounts(
		transaction, passdb, config, passwd, group) == 0);
	assert_text(passdb, "marker passdb\n");
	assert_text(config, "marker config\n");

	assert(unlink(passdb) == 0);
	assert(unlink(config) == 0);
	assert(unlink(passwd) == 0);
	assert(rmdir(root) == 0);
}

int
main(void)
{
	test_transaction_sync_retries_eintr();
	test_private_file_sync_helpers();
	test_directory_sync_helpers();
	test_parent_directory_permissions();
	test_account_files_share_the_passdb_config_transaction();

	char root[] = "/tmp/samba-config-transaction.XXXXXX";
	char lock_path[256], transaction_dir[256], passdb[256], config[256];
	char passdb_next[256], config_next[256], passdb_link[256];
	char state_path[256], passdb_snapshot[256], config_snapshot[256];
	char passdb_restore[256], config_restore[256];
	int lock_fd;

	assert(mkdtemp(root) != NULL);
	make_path(lock_path, sizeof(lock_path), root, "lock");
	make_path(transaction_dir, sizeof(transaction_dir), root, ".transaction");
	make_path(passdb, sizeof(passdb), root, "smbpasswd");
	make_path(config, sizeof(config), root, "smb.conf");
	make_path(passdb_next, sizeof(passdb_next), root, "smbpasswd.next");
	make_path(config_next, sizeof(config_next), root, "smb.conf.next");
	make_path(passdb_link, sizeof(passdb_link), root, "passdb-link");
	make_path(state_path, sizeof(state_path), transaction_dir, "state");
	make_path(passdb_snapshot, sizeof(passdb_snapshot), transaction_dir, "passdb.old");
	make_path(config_snapshot, sizeof(config_snapshot), transaction_dir, "config.old");
	make_path(passdb_restore, sizeof(passdb_restore), transaction_dir, "passdb.restore");
	make_path(config_restore, sizeof(config_restore), transaction_dir, "config.restore");

	lock_fd = samba_config_transaction_lock_acquire(lock_path);
	assert(lock_fd >= 0);
	assert(samba_config_transaction_lock_release(lock_fd) == 0);

	/* Abrupt exit after passdb replacement: recover both original files. */
	write_text(passdb, "old passdb\n", 0600);
	write_text(config, "old config\n", 0644);
	assert(samba_config_transaction_begin(transaction_dir, passdb, config) == 0);
	replace_text(passdb, passdb_next, "new passdb\n", 0600);
	assert(samba_config_transaction_recover(transaction_dir, passdb, config) == 0);
	assert_text(passdb, "old passdb\n");
	assert_text(config, "old config\n");
	assert(access(transaction_dir, F_OK) != 0);

	/* Abrupt exit after both renames but before COMMITTED: roll back the pair. */
	assert(samba_config_transaction_begin(transaction_dir, passdb, config) == 0);
	replace_text(passdb, passdb_next, "new passdb\n", 0600);
	replace_text(config, config_next, "new config\n", 0644);
	assert(samba_config_transaction_recover(transaction_dir, passdb, config) == 0);
	assert_text(passdb, "old passdb\n");
	assert_text(config, "old config\n");

	/* A committed pair is retained; recovery only removes snapshots. */
	assert(samba_config_transaction_begin(transaction_dir, passdb, config) == 0);
	replace_text(passdb, passdb_next, "committed passdb\n", 0600);
	replace_text(config, config_next, "committed config\n", 0644);
	assert(samba_config_transaction_commit(transaction_dir, passdb, config) == 0);
	assert_text(passdb, "committed passdb\n");
	assert_text(config, "committed config\n");
	assert(samba_config_transaction_recover(transaction_dir, passdb, config) == 0);
	assert_text(passdb, "committed passdb\n");
	assert_text(config, "committed config\n");

	/* First passdb creation: recovery removes the new file and restores config. */
	assert(unlink(passdb) == 0);
	assert(samba_config_transaction_begin(transaction_dir, passdb, config) == 0);
	write_text(passdb, "new first-run passdb\n", 0600);
	replace_text(config, config_next, "uncommitted config\n", 0644);
	assert(samba_config_transaction_recover(transaction_dir, passdb, config) == 0);
	assert(access(passdb, F_OK) != 0);
	assert_text(config, "committed config\n");

	/*
	 * Simulate termination during cleanup after both original files have been
	 * restored and PREPARED is removed, but before any snapshot is deleted.
	 */
	write_text(passdb, "cleanup old passdb\n", 0600);
	write_text(config, "cleanup old config\n", 0644);
	assert(samba_config_transaction_begin(transaction_dir, passdb, config) == 0);
	replace_text(passdb, passdb_next, "cleanup new passdb\n", 0600);
	replace_text(config, config_next, "cleanup new config\n", 0644);
	assert(samba_config_transaction_restore_file(
		passdb, passdb_snapshot, passdb_restore, 1) == 0);
	assert(samba_config_transaction_restore_file(
		config, config_snapshot, config_restore, 1) == 0);
	assert(unlink(state_path) == 0);
	assert(samba_config_transaction_recover(transaction_dir, passdb, config) == 0);
	assert_text(passdb, "cleanup old passdb\n");
	assert_text(config, "cleanup old config\n");
	assert(access(transaction_dir, F_OK) != 0);

	/* A partial rollback can be retried because the snapshots are retained. */
	write_text(passdb, "retry old passdb\n", 0600);
	write_text(config, "retry old config\n", 0644);
	assert(samba_config_transaction_begin(transaction_dir, passdb, config) == 0);
	replace_text(passdb, passdb_next, "retry new passdb\n", 0600);
	replace_text(config, config_next, "retry new config\n", 0644);
	assert(unlink(config) == 0);
	assert(mkdir(config, 0700) == 0);
	assert(samba_config_transaction_recover(transaction_dir, passdb, config) == -1);
	assert_text(passdb, "retry old passdb\n");
	assert(rmdir(config) == 0);
	assert(samba_config_transaction_recover(transaction_dir, passdb, config) == 0);
	assert_text(passdb, "retry old passdb\n");
	assert_text(config, "retry old config\n");

	/* Refuse a symlink target rather than snapshotting its referent. */
	assert(symlink(passdb, passdb_link) == 0);
	assert(samba_config_transaction_begin(transaction_dir, passdb_link, config) == -1);
	assert(access(transaction_dir, F_OK) != 0);

	assert(unlink(passdb_link) == 0);
	assert(unlink(passdb) == 0);
	assert(unlink(config) == 0);
	assert(unlink(lock_path) == 0);
	assert(rmdir(root) == 0);

	puts("Samba four-file account/passdb/config crash-recovery transaction: PASS");
	return 0;
}
