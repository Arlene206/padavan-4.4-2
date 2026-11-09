#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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

int
main(void)
{
	char root[] = "/tmp/samba-config-transaction.XXXXXX";
	char lock_path[256], transaction_dir[256], passdb[256], config[256];
	char passdb_next[256], config_next[256], passdb_link[256];
	int lock_fd;

	assert(mkdtemp(root) != NULL);
	make_path(lock_path, sizeof(lock_path), root, "lock");
	make_path(transaction_dir, sizeof(transaction_dir), root, ".transaction");
	make_path(passdb, sizeof(passdb), root, "smbpasswd");
	make_path(config, sizeof(config), root, "smb.conf");
	make_path(passdb_next, sizeof(passdb_next), root, "smbpasswd.next");
	make_path(config_next, sizeof(config_next), root, "smb.conf.next");
	make_path(passdb_link, sizeof(passdb_link), root, "passdb-link");

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

	puts("Samba paired passdb/config crash-recovery transaction: PASS");
	return 0;
}
