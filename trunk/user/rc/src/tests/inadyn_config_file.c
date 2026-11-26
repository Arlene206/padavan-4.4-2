#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "inadyn_config_file.h"
#include "wan_nvram_status_policy.h"

static void
read_exact(const char *path, const char *expected)
{
	char buffer[256];
	FILE *fp;
	size_t length;

	fp = fopen(path, "r");
	assert(fp != NULL);
	length = fread(buffer, 1, sizeof(buffer) - 1, fp);
	buffer[length] = '\0';
	assert(strcmp(buffer, expected) == 0);
	assert(!ferror(fp));
	assert(fclose(fp) == 0);
}

static int
count_staged_files(const char *directory, const char *prefix)
{
	DIR *dir;
	struct dirent *entry;
	int count = 0;
	size_t prefix_length = strlen(prefix);

	dir = opendir(directory);
	assert(dir != NULL);
	while ((entry = readdir(dir)) != NULL) {
		if (strncmp(entry->d_name, prefix, prefix_length) == 0)
			++count;
	}
	assert(closedir(dir) == 0);
	return count;
}

static void
test_checked_integer_status_preserves_defaults_and_rejects_errors(void)
{
	int result;

	result = 77;
	assert(inadyn_config_file_read_integer_status(0, 0, NULL, 0, 0, 2, &result) == 0);
	assert(result == 0);
	result = 77;
	assert(inadyn_config_file_read_integer_status(0, 0, NULL, 1, 0, 2, &result) == 0);
	assert(result == 1);
	result = 77;
	assert(inadyn_config_file_read_integer_status(1, 0, "2", 0, 0, 2, &result) == 0);
	assert(result == 2);
	result = 77;
	assert(inadyn_config_file_read_integer_status(1, 0, " \t1 \r\n", 0, 0, 1, &result) == 0);
	assert(result == 1);
	result = 77;
	assert(inadyn_config_file_read_integer_status(1, 0, "", 0, 0, 1, &result) == 0);
	assert(result == 0);
	result = 77;
	assert(inadyn_config_file_read_integer_status(1, 0, " \t ", 1, 0, 2, &result) == 0);
	assert(result == 1);

	result = 77;
	assert(inadyn_config_file_read_integer_status(-1, EACCES, "1", 0, 0, 1, &result) == EACCES);
	assert(result == 77);
	assert(inadyn_config_file_read_integer_status(-1, 0, "1", 0, 0, 1, &result) == EIO);
	assert(inadyn_config_file_read_integer_status(2, 0, "1", 0, 0, 1, &result) == EIO);
	assert(inadyn_config_file_read_integer_status(1, 0, NULL, 0, 0, 1, &result) == EIO);
	assert(inadyn_config_file_read_integer_status(1, 0, "-1", 0, 0, 1, &result) == EINVAL);
	assert(inadyn_config_file_read_integer_status(1, 0, "+1", 0, 0, 1, &result) == EINVAL);
	assert(inadyn_config_file_read_integer_status(1, 0, "1x", 0, 0, 1, &result) == EINVAL);
	assert(inadyn_config_file_read_integer_status(1, 0, "1.0", 0, 0, 1, &result) == EINVAL);
	assert(inadyn_config_file_read_integer_status(1, 0, "2", 0, 0, 1, &result) == EINVAL);
	assert(inadyn_config_file_read_integer_status(1, 0, "999999999999999999999999999999999999", 0, 0, 1, &result) == ERANGE);
	assert(inadyn_config_file_read_integer_status(1, 0, "1", 2, 0, 1, &result) == EINVAL);
	assert(inadyn_config_file_read_integer_status(1, 0, "1", 0, 2, 1, &result) == EINVAL);
	assert(inadyn_config_file_read_integer_status(1, 0, "1", 0, 0, 1, NULL) == EINVAL);
	assert(result == 77);
}

static void
test_sensitive_snapshot_clear_wipes_every_byte(void)
{
	unsigned char secret[64];
	size_t i;

	memset(secret, 0xa5, sizeof(secret));
	inadyn_config_file_clear_sensitive(secret, sizeof(secret));
	for (i = 0; i < sizeof(secret); i++)
		assert(secret[i] == 0);

	inadyn_config_file_clear_sensitive(NULL, sizeof(secret));
}

static void
test_quoted_setting_escapes_configuration_delimiters(void)
{
	static const char input[] = "quote\" slash\\ line\ncarriage\r tab\t";
	static const char expected[] =
		"password = \"quote\\\" slash\\\\ line\\ncarriage\\r tab\\t\"\n";
	char buffer[128];
	size_t length;
	FILE *fp = tmpfile();

	assert(fp != NULL);
	assert(inadyn_config_file_write_quoted_setting(
		fp, "password = \"", input, "\"\n") == 0);
	assert(fflush(fp) == 0);
	assert(fseek(fp, 0, SEEK_SET) == 0);
	length = fread(buffer, 1, sizeof(buffer) - 1, fp);
	buffer[length] = '\0';
	assert(strcmp(buffer, expected) == 0);
	assert(!ferror(fp));
	assert(fclose(fp) == 0);
}

static void
test_quoted_setting_rejects_control_bytes(void)
{
	static const char input[] = { 's', 'a', 'f', 'e', '\001', 'b', 'a', 'd', '\0' };
	FILE *fp = tmpfile();

	assert(fp != NULL);
	errno = 0;
	assert(inadyn_config_file_write_quoted_setting(
		fp, "hostname = \"", input, "\"\n") == -1);
	assert(errno == EINVAL);
	assert(fclose(fp) == 0);
}

static void
test_rejected_value_discards_partial_config(const char *config,
                                            const char *directory)
{
	static const unsigned char invalid[] = { 'u', 's', 'e', 'r', 1, 'b', 'a', 'd', 0 };
	char staged_path[512];
	FILE *fp;

	fp = inadyn_config_file_open(config, staged_path, sizeof(staged_path));
	assert(fp != NULL);
	assert(fputs("partial-config-before-rejected-value\n", fp) >= 0);
	errno = 0;
	assert(inadyn_config_file_write_quoted_setting(
		fp, "username = \"", (const char *)invalid, "\"\n") == -1);
	assert(errno == EINVAL);
	assert(inadyn_config_file_discard(fp, staged_path) == 0);
	assert(staged_path[0] == '\0');
	/* The preceding snapshot test deliberately committed this generation. */
	read_exact(config, "committed-generation\n");
	assert(count_staged_files(directory, "inadyn.conf.tmp.") == 0);
}

struct fake_nvram_snapshot {
	char shared_cache[64];
	unsigned int get_calls;
	int get_failure_at;
	int get_failure_status;
	int end_status;
	int begin_calls;
	int end_calls;
};

static int
fake_nvram_snapshot_begin(void *context)
{
	struct fake_nvram_snapshot *fake = (struct fake_nvram_snapshot *)context;
	fake->begin_calls++;
	return 0;
}

static int
fake_nvram_snapshot_get(void *context, const char *name, char **value)
{
	static const char *const contents[] = { "provider-a", "host.example", "secret-b" };
	struct fake_nvram_snapshot *fake = (struct fake_nvram_snapshot *)context;
	unsigned int index = fake->get_calls++;

	assert(name != NULL && name[0] != '\0');
	if ((int)index == fake->get_failure_at) {
		errno = fake->get_failure_status;
		return -1;
	}
	assert(index < sizeof(contents) / sizeof(contents[0]));
	snprintf(fake->shared_cache, sizeof(fake->shared_cache), "%s", contents[index]);
	*value = fake->shared_cache;
	return 1;
}

static int
fake_nvram_snapshot_end(void *context)
{
	struct fake_nvram_snapshot *fake = (struct fake_nvram_snapshot *)context;
	fake->end_calls++;
	if (fake->end_status != 0) {
		errno = fake->end_status;
		return -1;
	}
	return 0;
}

static void
test_nvram_snapshot_copies_rotating_cache_and_invalidates_failed_tuple(void)
{
	struct fake_nvram_snapshot fake;
	struct rc_wan_nvram_snapshot_field fields[3];
	char values[3][64];

	memset(&fake, 0, sizeof(fake));
	fake.get_failure_at = -1;
	fields[0].param_name = "ddns_server_x";
	fields[0].destination = values[0];
	fields[0].destination_size = sizeof(values[0]);
	fields[1].param_name = "ddns_hostname_x";
	fields[1].destination = values[1];
	fields[1].destination_size = sizeof(values[1]);
	fields[2].param_name = "ddns_passwd_x";
	fields[2].destination = values[2];
	fields[2].destination_size = sizeof(values[2]);

	assert(rc_wan_nvram_copy_snapshot(fields, 3, fake_nvram_snapshot_begin,
					   fake_nvram_snapshot_get,
					   fake_nvram_snapshot_end, &fake) == 0);
	assert(fake.begin_calls == 1 && fake.get_calls == 3 && fake.end_calls == 1);
	assert(strcmp(values[0], "provider-a") == 0);
	assert(strcmp(values[1], "host.example") == 0);
	assert(strcmp(values[2], "secret-b") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.get_failure_at = -1;
	fake.end_status = EBUSY;
	memset(values, 'x', sizeof(values));
	assert(rc_wan_nvram_copy_snapshot(fields, 3, fake_nvram_snapshot_begin,
					   fake_nvram_snapshot_get,
					   fake_nvram_snapshot_end, &fake) == EBUSY);
	assert(fake.end_calls == 1);
	assert(values[0][0] == '\0' && values[1][0] == '\0' &&
	       values[2][0] == '\0');

	memset(&fake, 0, sizeof(fake));
	fake.get_failure_at = 1;
	fake.get_failure_status = EACCES;
	memset(values, 'x', sizeof(values));
	assert(rc_wan_nvram_copy_snapshot(fields, 3, fake_nvram_snapshot_begin,
					   fake_nvram_snapshot_get,
					   fake_nvram_snapshot_end, &fake) == EACCES);
	assert(fake.get_calls == 2 && fake.end_calls == 1);
	assert(values[0][0] == '\0' && values[1][0] == '\0' &&
	       values[2][0] == '\0');
}

static void
test_config_snapshot_restore_and_commit(const char *config,
                                        const char *directory,
                                        const char *new_config)
{
	struct inadyn_config_file_snapshot snapshot;
	struct stat st;
	char staged[512];
	FILE *fp;
	assert(inadyn_config_file_snapshot_begin(config, &snapshot) == 0);
	assert(snapshot.active && snapshot.had_original);
	assert(lstat(config, &st) == 0 && st.st_nlink == 2);
	assert(count_staged_files(directory, "inadyn.conf.rollback.") == 1);
	fp = inadyn_config_file_open(config, staged, sizeof(staged));
	assert(fp != NULL && fputs("failed-generation\n", fp) >= 0);
	assert(inadyn_config_file_publish(fp, staged, config) == 0);
	read_exact(config, "failed-generation\n");
	assert(inadyn_config_file_snapshot_restore(config, &snapshot) == 0);
	assert(!snapshot.active && !snapshot.backup_path[0]);
	read_exact(config, "new-user\nnew-password\n");
	assert(stat(config, &st) == 0 && st.st_nlink == 1);
	assert(count_staged_files(directory, "inadyn.conf.rollback.") == 0);

	assert(inadyn_config_file_snapshot_begin(new_config, &snapshot) == 0);
	assert(snapshot.active && !snapshot.had_original);
	fp = inadyn_config_file_open(new_config, staged, sizeof(staged));
	assert(fp != NULL && fputs("first-generation\n", fp) >= 0);
	assert(inadyn_config_file_publish(fp, staged, new_config) == 0);
	assert(inadyn_config_file_snapshot_restore(new_config, &snapshot) == 0);
	errno = 0;
	assert(lstat(new_config, &st) == -1 && errno == ENOENT);

	assert(inadyn_config_file_snapshot_begin(config, &snapshot) == 0);
	fp = inadyn_config_file_open(config, staged, sizeof(staged));
	assert(fp != NULL && fputs("committed-generation\n", fp) >= 0);
	assert(inadyn_config_file_publish(fp, staged, config) == 0);
	assert(inadyn_config_file_snapshot_commit(config, &snapshot) == 0);
	assert(!snapshot.active && !snapshot.backup_path[0]);
	read_exact(config, "committed-generation\n");
	assert(stat(config, &st) == 0 && st.st_nlink == 1);
	assert(count_staged_files(directory, "inadyn.conf.rollback.") == 0);
}

static void
test_config_snapshot_rejects_symlink(const char *link_path)
{
	struct inadyn_config_file_snapshot snapshot;
	errno = 0;
	assert(inadyn_config_file_snapshot_begin(link_path, &snapshot) == -1);
	assert(errno == EPERM && !snapshot.active && !snapshot.backup_path[0]);
}

static void
test_config_directory_sync_rejects_symlink(const char *directory)
{
	char link_path[256], child[320];
	assert(inadyn_config_file_sync_directory(directory) == 0);
	assert(inadyn_config_file_sync_parent(directory) == 0);
	assert(snprintf(link_path, sizeof(link_path), "%s/dir-link", directory) > 0);
	assert(symlink(directory, link_path) == 0);
	errno = 0;
	assert(inadyn_config_file_sync_directory(link_path) == -1);
	assert(errno == ELOOP || errno == ENOTDIR);
	assert(snprintf(child, sizeof(child), "%s/child", link_path) > 0);
	errno = 0;
	assert(inadyn_config_file_sync_parent(child) == -1);
	assert(errno == ELOOP || errno == ENOTDIR);
	assert(unlink(link_path) == 0);
}

int
main(void)
{
	char directory[] = "/tmp/inadyn-config-test.XXXXXX";
	char config[256], new_config[256], victim[256], link_path[256], blocked[256], sentinel[256];
	char staged_path[512];
	struct stat before, after;
	mode_t old_umask;
	FILE *fp;

	assert(mkdtemp(directory) != NULL);
	assert(snprintf(config, sizeof(config), "%s/inadyn.conf", directory) > 0);
	assert(snprintf(new_config, sizeof(new_config), "%s/new-inadyn.conf", directory) > 0);
	assert(snprintf(victim, sizeof(victim), "%s/victim", directory) > 0);
	assert(snprintf(link_path, sizeof(link_path), "%s/config-link", directory) > 0);
	assert(snprintf(blocked, sizeof(blocked), "%s/blocked", directory) > 0);
	assert(snprintf(sentinel, sizeof(sentinel), "%s/blocked/sentinel", directory) > 0);

	old_umask = umask(0000);

	/* Reproduce the original fopen("w") mode under PID 1's zero umask. */
	fp = fopen(config, "w");
	assert(fp != NULL);
	assert(fputs("old-user\nold-password\n", fp) >= 0);
	assert(fclose(fp) == 0);
	assert(stat(config, &before) == 0);
	assert((before.st_mode & 0777) == 0666);

	/* The staged inode is private before any DDNS credential is written. */
	fp = inadyn_config_file_open(config, staged_path, sizeof(staged_path));
	assert(fp != NULL && staged_path[0] != '\0');
	assert(stat(staged_path, &after) == 0);
	assert((after.st_mode & 0777) == 0600);
	assert(fputs("new-user\nnew-password\n", fp) >= 0);
	assert(inadyn_config_file_publish(fp, staged_path, config) == 0);
	assert(staged_path[0] == '\0');
	assert(stat(config, &after) == 0);
	assert((after.st_mode & 0777) == 0600);
	assert(after.st_nlink == 1);
	assert(before.st_ino != after.st_ino);
	read_exact(config, "new-user\nnew-password\n");
	test_config_directory_sync_rejects_symlink(directory);
	test_config_snapshot_restore_and_commit(config, directory, new_config);

	/* Atomic publication replaces a destination symlink, not its target. */
	fp = fopen(victim, "w");
	assert(fp != NULL);
	assert(fputs("keep-victim\n", fp) >= 0);
	assert(fclose(fp) == 0);
	assert(symlink(victim, link_path) == 0);
	test_config_snapshot_rejects_symlink(link_path);
	fp = inadyn_config_file_open(link_path, staged_path, sizeof(staged_path));
	assert(fp != NULL);
	assert(fputs("private-link-replacement\n", fp) >= 0);
	assert(inadyn_config_file_publish(fp, staged_path, link_path) == 0);
	assert(lstat(link_path, &after) == 0 && S_ISREG(after.st_mode));
	assert((after.st_mode & 0777) == 0600);
	read_exact(victim, "keep-victim\n");
	read_exact(link_path, "private-link-replacement\n");

	/* Aborting a partial build leaves the previous config untouched. */
	fp = inadyn_config_file_open(config, staged_path, sizeof(staged_path));
	assert(fp != NULL);
	assert(fputs("partial-secret\n", fp) >= 0);
	assert(inadyn_config_file_discard(fp, staged_path) == 0);
	assert(staged_path[0] == '\0');
	read_exact(config, "new-user\nnew-password\n");
	assert(count_staged_files(directory, "inadyn.conf.tmp.") == 0);

	/* A rename failure cleans the stage and preserves the existing directory. */
	assert(mkdir(blocked, 0700) == 0);
	fp = fopen(sentinel, "w");
	assert(fp != NULL);
	assert(fputs("sentinel\n", fp) >= 0);
	assert(fclose(fp) == 0);
	fp = inadyn_config_file_open(blocked, staged_path, sizeof(staged_path));
	assert(fp != NULL);
	assert(fputs("unpublishable\n", fp) >= 0);
	errno = 0;
	assert(inadyn_config_file_publish(fp, staged_path, blocked) == -1);
	assert(errno != 0);
	assert(staged_path[0] == '\0');
	read_exact(sentinel, "sentinel\n");
	assert(count_staged_files(directory, "blocked.tmp.") == 0);

	staged_path[0] = 'x';
	errno = 0;
	assert(inadyn_config_file_open(NULL, staged_path, sizeof(staged_path)) == NULL);
	assert(errno == EINVAL && staged_path[0] == '\0');

	test_rejected_value_discards_partial_config(config, directory);

	assert(umask(old_umask) == 0000);
	assert(unlink(config) == 0);
	assert(unlink(victim) == 0);
	assert(unlink(link_path) == 0);
	assert(unlink(sentinel) == 0);
	assert(rmdir(blocked) == 0);
	assert(rmdir(directory) == 0);

	test_nvram_snapshot_copies_rotating_cache_and_invalidates_failed_tuple();
	test_checked_integer_status_preserves_defaults_and_rejects_errors();
	test_sensitive_snapshot_clear_wipes_every_byte();
	test_quoted_setting_escapes_configuration_delimiters();
	test_quoted_setting_rejects_control_bytes();
	puts("Inadyn config snapshot rollback/commit and private atomic publication: PASS");
	return 0;
}
