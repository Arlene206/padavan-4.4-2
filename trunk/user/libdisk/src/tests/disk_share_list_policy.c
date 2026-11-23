#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "disk_share_list_policy.h"
#include "disk_share_account_nvram_policy.h"
#include "router_account_policy.h"

static void
test_invalid_mount_paths_clear_outputs(void)
{
	char *sentinel = "stale";
	char **folder_list = &sentinel;
	int sh_num = 17;

	assert(disk_share_list_prepare_outputs(NULL, &sh_num, &folder_list) == -1);
	assert(sh_num == 0);
	assert(folder_list == NULL);

	sh_num = 9;
	folder_list = &sentinel;
	assert(disk_share_list_prepare_outputs("", &sh_num, &folder_list) == -1);
	assert(sh_num == 0);
	assert(folder_list == NULL);
}

static void
test_missing_output_parameters_are_safe(void)
{
	char *sentinel = "stale";
	char **folder_list = &sentinel;
	int sh_num = 13;

	assert(disk_share_list_prepare_outputs("/mnt/share", NULL,
	                                       &folder_list) == -1);
	assert(folder_list == NULL);

	folder_list = &sentinel;
	assert(disk_share_list_prepare_outputs("/mnt/share", &sh_num, NULL) == -1);
	assert(sh_num == 0);
	assert(disk_share_list_prepare_outputs("/mnt/share", NULL, NULL) == -1);
}

static void
test_valid_mount_path_starts_with_empty_outputs(void)
{
	char *sentinel = "stale";
	char **folder_list = &sentinel;
	int sh_num = 23;

	assert(disk_share_list_prepare_outputs("/mnt/share", &sh_num,
	                                       &folder_list) == 0);
	assert(sh_num == 0);
	assert(folder_list == NULL);
}

enum {
	FAKE_READ_ENTRY,
	FAKE_READ_EOF,
	FAKE_READ_ERROR
};


struct fake_account_rename_state {
	const char *paths[4];
	int calls;
	int fail_call;
	int rollback_fail_call;
};

static struct fake_account_rename_state *fake_account_rename_state;

static int
fake_account_rename(const char *source, const char *destination)
{
	int i;
	struct fake_account_rename_state *state = fake_account_rename_state;

	++state->calls;
	if (state->calls == state->fail_call) {
		errno = EACCES;
		return -1;
	}
	if (state->calls == state->rollback_fail_call) {
		errno = EROFS;
		return -1;
	}
	for (i = 0; i < 4; ++i) {
		if (state->paths[i] && strcmp(state->paths[i], source) == 0) {
			state->paths[i] = destination;
			return 0;
		}
	}
	errno = ENOENT;
	return -1;
}

static int fake_readdir_mode;
static struct dirent fake_dirent;

static struct dirent *
fake_readdir(DIR *directory)
{
	(void)directory;
	if (fake_readdir_mode == FAKE_READ_ENTRY)
		return &fake_dirent;
	if (fake_readdir_mode == FAKE_READ_ERROR) {
		errno = EIO;
		return NULL;
	}
	return NULL;
}

static void
test_readdir_entry_and_clean_eof(void)
{
	struct dirent *entry = NULL;
	int read_error = EINVAL;

	fake_readdir_mode = FAKE_READ_ENTRY;
	errno = EBUSY;
	assert(disk_share_list_next_entry(NULL, fake_readdir,
	                                  &entry, &read_error) == 1);
	assert(entry == &fake_dirent);
	assert(read_error == 0);

	fake_readdir_mode = FAKE_READ_EOF;
	entry = &fake_dirent;
	read_error = EBUSY;
	errno = EBUSY;
	assert(disk_share_list_next_entry(NULL, fake_readdir,
	                                  &entry, &read_error) == 0);
	assert(entry == NULL);
	assert(read_error == 0);
}

static void
test_readdir_error_is_not_clean_eof(void)
{
	struct dirent *entry = &fake_dirent;
	int read_error = 0;

	fake_readdir_mode = FAKE_READ_ERROR;
	assert(disk_share_list_next_entry(NULL, fake_readdir,
	                                  &entry, &read_error) == -1);
	assert(entry == NULL);
	assert(read_error == EIO);
}

static void
test_readdir_rejects_missing_adapters(void)
{
	struct dirent *entry = &fake_dirent;
	int read_error = 0;

	assert(disk_share_list_next_entry(NULL, NULL,
	                                  &entry, &read_error) == -1);
	assert(entry == NULL);
	assert(read_error == EINVAL);
}

static void
test_snapshot_write_flushes_complete_content(void)
{
	char *folder_list[] = { "Public", "Media" };
	char buffer[64];
	FILE *stream = tmpfile();

	assert(stream != NULL);
	assert(disk_share_list_write_snapshot(stream, 2, folder_list) == 0);
	rewind(stream);
	assert(fgets(buffer, sizeof(buffer), stream) != NULL);
	assert(strcmp(buffer, "sh_num=2\n") == 0);
	assert(fgets(buffer, sizeof(buffer), stream) != NULL);
	assert(strcmp(buffer, "sh_name0=Public\n") == 0);
	assert(fgets(buffer, sizeof(buffer), stream) != NULL);
	assert(strcmp(buffer, "sh_name1=Media\n") == 0);
	assert(fclose(stream) == 0);
}

static void
test_snapshot_write_reports_io_failure(void)
{
	char *folder_list[] = { "Public" };
	FILE *stream = fopen("/dev/full", "w");

	assert(stream != NULL);
	errno = 0;
	assert(disk_share_list_write_snapshot(stream, 1, folder_list) == -1);
	assert(errno != 0);
	(void)fclose(stream);
}

static void
test_snapshot_write_rejects_invalid_data(void)
{
	FILE *stream = tmpfile();

	assert(stream != NULL);
	assert(disk_share_list_write_snapshot(stream, 1, NULL) == -1);
	assert(errno == EINVAL);
	(void)fclose(stream);
}

static void
test_account_defaults_write_complete_content(void)
{
	char *folders[] = { "Public", "Media" };
	char buffer[64];
	FILE *stream = tmpfile();

	assert(stream != NULL);
	assert(disk_share_write_account_defaults(stream, 2, folders, 3, 2, 1) == 0);
	rewind(stream);
	assert(fgets(buffer, sizeof(buffer), stream) != NULL);
	assert(strcmp(buffer, "*Public=321\n") == 0);
	assert(fgets(buffer, sizeof(buffer), stream) != NULL);
	assert(strcmp(buffer, "*Media=321\n") == 0);
	assert(fclose(stream) == 0);
}

static void
test_account_defaults_write_reports_io_failure(void)
{
	char *folders[] = { "Public" };
	FILE *stream = fopen("/dev/full", "w");

	assert(stream != NULL);
	errno = 0;
	assert(disk_share_write_account_defaults(stream, 1, folders, 3, 2, 1) == -1);
	assert(errno != 0);
	(void)fclose(stream);
}

static void
test_account_defaults_write_rejects_invalid_data(void)
{
	char *folders[] = { NULL };
	FILE *stream = tmpfile();

	assert(stream != NULL);
	assert(disk_share_write_account_defaults(stream, 1, NULL, 3, 2, 1) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(disk_share_write_account_defaults(stream, 1, folders, 3, 2, 1) == -1);
	assert(errno == EINVAL);
	(void)fclose(stream);
}

static void
test_permission_rewrite_removes_complete_record(void)
{
	const char *source = "*Public=331\n*Media=321\n*Backup=301";
	char *updated = NULL;
	size_t updated_len = 0;

	assert(disk_share_permission_rewrite(source, "*Public=", NULL,
	                                     &updated, &updated_len) == 0);
	assert(updated_len == strlen("*Media=321\n*Backup=301"));
	assert(strcmp(updated, "*Media=321\n*Backup=301") == 0);
	free(updated);
}

static void
test_permission_rewrite_renames_key_and_preserves_rights(void)
{
	const char *source = "*Public=331\n*Media=321\n";
	char *updated = NULL;
	size_t updated_len = 0;

	assert(disk_share_permission_rewrite(source, "*public=", "*Archive=",
	                                     &updated, &updated_len) == 0);
	assert(updated_len == strlen("*Archive=331\n*Media=321\n"));
	assert(strcmp(updated, "*Archive=331\n*Media=321\n") == 0);
	free(updated);
}

static void
test_permission_rewrite_reports_missing_record_and_clears_outputs(void)
{
	char sentinel;
	char *updated = &sentinel;
	size_t updated_len = 17;

	assert(disk_share_permission_rewrite("*Media=321\n", "*Public=", NULL,
	                                     &updated, &updated_len) == -1);
	assert(errno == ENOENT);
	assert(updated == NULL);
	assert(updated_len == 0);
}

static void
test_account_rename_status_accepts_success_and_missing_source(void)
{
	assert(disk_share_account_rename_status(0, EACCES) == 0);
	assert(disk_share_account_rename_status(-1, ENOENT) == 0);
}

static void
test_account_rename_status_preserves_real_errors(void)
{
	assert(disk_share_account_rename_status(-1, EACCES) == EACCES);
	assert(disk_share_account_rename_status(-1, 0) == EIO);
}

static void
test_account_var_path_rejects_truncation(void)
{
	char path[128];
	char short_path[8];

	assert(disk_share_format_account_var_path(path, sizeof(path),
	                                          "/mnt/usb1", "admin") == 0);
	assert(strcmp(path, "/mnt/usb1/.__admin_var.txt") == 0);

	errno = 0;
	assert(disk_share_format_account_var_path(short_path, sizeof(short_path),
	                                          "/mnt/usb1", "admin") == -1);
	assert(errno == ENAMETOOLONG);

	errno = 0;
	assert(disk_share_format_account_var_path(path, sizeof(path),
	                                          NULL, "admin") == -1);
	assert(errno == EINVAL);
}


struct fake_account_nvram {
	const char *count;
	const char *http_username;
	const char *users[4];
	const char *fail_key;
	const char *invalid_status_key;
	int invalid_status;
	int begin_error;
	int end_error;
	int begin_calls;
	int get_calls;
	int end_calls;
	const char *passwords[4];
};

static int
fake_account_nvram_begin(void *context)
{
	struct fake_account_nvram *fake = (struct fake_account_nvram *)context;
	++fake->begin_calls;
	if (fake->begin_error) {
		errno = fake->begin_error;
		return -1;
	}
	return 0;
}

static int
fake_account_nvram_get(void *context, const char *name, char **value)
{
	struct fake_account_nvram *fake = (struct fake_account_nvram *)context;
	int index;

	++fake->get_calls;
	*value = NULL;
	if (fake->fail_key && strcmp(name, fake->fail_key) == 0) {
		errno = EIO;
		return -1;
	}
	if (fake->invalid_status_key &&
	    strcmp(name, fake->invalid_status_key) == 0) {
		*value = (char *)"unexpected";
		return fake->invalid_status;
	}
	if (strcmp(name, "acc_num") == 0) {
		if (!fake->count)
			return 0;
		*value = (char *)fake->count;
		return 1;
	}
	if (strcmp(name, "http_username") == 0) {
		if (!fake->http_username)
			return 0;
		*value = (char *)fake->http_username;
		return 1;
	}
	if (sscanf(name, "acc_username%d", &index) == 1 &&
	    index >= 0 && index < 4) {
		if (!fake->users[index])
			return 0;
		*value = (char *)fake->users[index];
		return 1;
	}
	if (sscanf(name, "acc_password%d", &index) == 1 &&
	    index >= 0 && index < 4) {
		if (!fake->passwords[index])
			return 0;
		*value = (char *)fake->passwords[index];
		return 1;
	}
	return 0;
}

static int
fake_account_nvram_end(void *context)
{
	struct fake_account_nvram *fake = (struct fake_account_nvram *)context;
	++fake->end_calls;
	if (fake->end_error) {
		errno = fake->end_error;
		return -1;
	}
	return 0;
}

static int
fake_account_name_valid(const char *name)
{
	const unsigned char *p;

	if (!name || !*name)
		return 0;
	for (p = (const unsigned char *)name; *p; ++p) {
		if (!((*p >= 'a' && *p <= 'z') ||
		      (*p >= 'A' && *p <= 'Z') ||
		      (*p >= '0' && *p <= '9') ||
		      *p == '_' || *p == '-'))
			return 0;
	}
	return 1;
}

static int
fake_account_name_reserved(const char *name, const char *http_username)
{
	return strcmp(name, "root") == 0 ||
	       strcmp(name, "nobody") == 0 ||
	       strcmp(name, "sshd") == 0 ||
	       strcmp(name, "anonymous") == 0 ||
	       (http_username && *http_username &&
		strcmp(name, http_username) == 0);
}

static int
fake_account_nvram_set_unused(void *context, const char *name, const char *value)
{
	(void)context;
	(void)name;
	(void)value;
	errno = ENOTSUP;
	return -1;
}

static int
fake_account_nvram_commit_unused(void *context)
{
	(void)context;
	return 0;
}

static const struct disk_share_account_nvram_ops fake_account_nvram_ops = {
	fake_account_nvram_begin,
	fake_account_nvram_get,
	fake_account_nvram_end,
	fake_account_name_valid,
	fake_account_name_reserved,
	fake_account_nvram_set_unused,
	fake_account_nvram_commit_unused
};

static void
test_account_inventory_reads_one_checked_snapshot(void)
{
	struct fake_account_nvram fake = {
		"2", "admin", { "alice", "bob", NULL, NULL },
		NULL, NULL, 0, 0, 0, 0, 0, 0,
		{ NULL, NULL, NULL, NULL }
	};
	char **items = NULL;
	int count = -1;

	assert(disk_share_account_inventory_read(&fake_account_nvram_ops,
		&fake, 4, &count, &items) == 0);
	assert(count == 2);
	assert(strcmp(items[0], "alice") == 0);
	assert(strcmp(items[1], "bob") == 0);
	assert(fake.begin_calls == 1);
	assert(fake.end_calls == 1);
	disk_share_account_inventory_free(count, items);
}

static void
test_account_inventory_missing_count_is_empty(void)
{
	struct fake_account_nvram fake = {
		NULL, NULL, { NULL, NULL, NULL, NULL },
		NULL, NULL, 0, 0, 0, 0, 0, 0,
		{ NULL, NULL, NULL, NULL }
	};
	char **items = (char **)1;
	int count = 99;

	assert(disk_share_account_inventory_read(&fake_account_nvram_ops,
		&fake, 4, &count, &items) == 0);
	assert(count == 0);
	assert(items == NULL);
	assert(fake.begin_calls == 1);
	assert(fake.end_calls == 1);
	assert(fake.get_calls == 1);
}

static void
test_account_inventory_count_read_error_is_not_zero(void)
{
	struct fake_account_nvram fake = {
		"2", "admin", { "alice", "bob", NULL, NULL },
		"acc_num", NULL, 0, 0, 0, 0, 0, 0,
		{ NULL, NULL, NULL, NULL }
	};
	char **items = (char **)1;
	int count = 99;

	errno = 0;
	assert(disk_share_account_inventory_read(&fake_account_nvram_ops,
		&fake, 4, &count, &items) == -1);
	assert(errno == EIO);
	assert(count == 0);
	assert(items == NULL);
	assert(fake.end_calls == 1);
}

static void
test_account_inventory_username_error_frees_partial_list(void)
{
	struct fake_account_nvram fake = {
		"2", "admin", { "alice", "bob", NULL, NULL },
		"acc_username1", NULL, 0, 0, 0, 0, 0, 0,
		{ NULL, NULL, NULL, NULL }
	};
	char **items = (char **)1;
	int count = 99;

	errno = 0;
	assert(disk_share_account_inventory_read(&fake_account_nvram_ops,
		&fake, 4, &count, &items) == -1);
	assert(errno == EIO);
	assert(count == 0);
	assert(items == NULL);
	assert(fake.end_calls == 1);
}

static void
test_account_inventory_transaction_end_error_discards_snapshot(void)
{
	struct fake_account_nvram fake = {
		"2", "admin", { "alice", "bob", NULL, NULL },
		NULL, NULL, 0, 0, EBUSY, 0, 0, 0,
		{ NULL, NULL, NULL, NULL }
	};
	char **items = (char **)1;
	int count = 99;

	errno = 0;
	assert(disk_share_account_inventory_read(&fake_account_nvram_ops,
		&fake, 4, &count, &items) == -1);
	assert(errno == EBUSY);
	assert(count == 0);
	assert(items == NULL);
	assert(fake.end_calls == 1);
}

static void
test_account_inventory_begin_error_does_not_end_transaction(void)
{
	struct fake_account_nvram fake = {
		"1", "admin", { "alice", NULL, NULL, NULL },
		NULL, NULL, 0, EACCES, 0, 0, 0, 0,
		{ NULL, NULL, NULL, NULL }
	};
	char **items = (char **)1;
	int count = 99;

	errno = 0;
	assert(disk_share_account_inventory_read(&fake_account_nvram_ops,
		&fake, 4, &count, &items) == -1);
	assert(errno == EACCES);
	assert(count == 0);
	assert(items == NULL);
	assert(fake.end_calls == 0);
}

static void
test_account_inventory_rejects_invalid_count_text_and_range(void)
{
	struct fake_account_nvram fake = {
		"2x", "admin", { "alice", "bob", NULL, NULL },
		NULL, NULL, 0, 0, 0, 0, 0, 0,
		{ NULL, NULL, NULL, NULL }
	};
	char **items = NULL;
	int count = 0;

	assert(disk_share_account_inventory_read(&fake_account_nvram_ops,
		&fake, 4, &count, &items) == -1);
	assert(errno == EINVAL);
	assert(count == 0 && items == NULL);

	fake.count = "-1";
	assert(disk_share_account_inventory_read(&fake_account_nvram_ops,
		&fake, 4, &count, &items) == -1);
	assert(errno == EINVAL);

	fake.count = "5";
	assert(disk_share_account_inventory_read(&fake_account_nvram_ops,
		&fake, 4, &count, &items) == -1);
	assert(errno == ERANGE);
}

static void
test_account_inventory_rejects_sparse_and_reserved_names(void)
{
	struct fake_account_nvram fake = {
		"2", "admin", { NULL, "alice", NULL, NULL },
		NULL, NULL, 0, 0, 0, 0, 0, 0,
		{ NULL, NULL, NULL, NULL }
	};
	char **items = NULL;
	int count = 0;

	assert(disk_share_account_inventory_read(&fake_account_nvram_ops,
		&fake, 4, &count, &items) == -1);
	assert(errno == EINVAL);
	assert(count == 0 && items == NULL);

	fake.users[0] = "anonymous";
	fake.users[1] = NULL;
	fake.count = "1";
	assert(disk_share_account_inventory_read(&fake_account_nvram_ops,
		&fake, 4, &count, &items) == -1);
	assert(errno == EINVAL);

	fake.users[0] = "admin";
	assert(disk_share_account_inventory_read(&fake_account_nvram_ops,
		&fake, 4, &count, &items) == -1);
	assert(errno == EINVAL);
}

static void
test_account_inventory_rejects_invalid_get_status(void)
{
	struct fake_account_nvram fake = {
		"1", "admin", { "alice", NULL, NULL, NULL },
		NULL, "acc_username0", 2, 0, 0, 0, 0, 0,
		{ NULL, NULL, NULL, NULL }
	};
	char **items = NULL;
	int count = 0;

	assert(disk_share_account_inventory_read(&fake_account_nvram_ops,
		&fake, 4, &count, &items) == -1);
	assert(errno == EIO);
	assert(count == 0 && items == NULL);
}

static void
test_account_inventory_rejects_duplicate_names(void)
{
	struct fake_account_nvram fake = {
		"2", "admin", { "alice", "alice", NULL, NULL },
		NULL, NULL, 0, 0, 0, 0, 0, 0,
		{ NULL, NULL, NULL, NULL }
	};
	char **items = NULL;
	int count = 0;

	assert(disk_share_account_inventory_read(&fake_account_nvram_ops,
		&fake, 4, &count, &items) == -1);
	assert(errno == EINVAL);
	assert(count == 0 && items == NULL);
}

static void
test_account_password_shift_snapshot_copies_values_in_one_transaction(void)
{
	struct fake_account_nvram fake = {
		"3", "admin", { "alice", "bob", "carol", NULL },
		NULL, NULL, 0, 0, 0, 0, 0, 0,
		{ "deleted", "bob-secret", "carol-secret", NULL }
	};
	char passwords[4][DISK_SHARE_ACCOUNT_PASSWORD_VALUE_MAX];

	assert(disk_share_account_password_shift_snapshot(
		&fake_account_nvram_ops, &fake, 3, 0, 4, passwords) == 0);
	assert(strcmp(passwords[0], "bob-secret") == 0);
	assert(strcmp(passwords[1], "carol-secret") == 0);
	assert(passwords[2][0] == '\0');
	assert(fake.begin_calls == 1);
	assert(fake.get_calls == 2);
	assert(fake.end_calls == 1);
}

static void
test_account_password_shift_snapshot_preserves_missing_as_empty(void)
{
	struct fake_account_nvram fake = {
		"3", "admin", { "alice", "bob", "carol", NULL },
		NULL, NULL, 0, 0, 0, 0, 0, 0,
		{ "deleted", NULL, "carol-secret", NULL }
	};
	char passwords[4][DISK_SHARE_ACCOUNT_PASSWORD_VALUE_MAX];

	assert(disk_share_account_password_shift_snapshot(
		&fake_account_nvram_ops, &fake, 3, 0, 4, passwords) == 0);
	assert(passwords[0][0] == '\0');
	assert(strcmp(passwords[1], "carol-secret") == 0);
}

static void
test_account_password_shift_snapshot_read_error_clears_all_values(void)
{
	struct fake_account_nvram fake = {
		"3", "admin", { "alice", "bob", "carol", NULL },
		"acc_password2", NULL, 0, 0, 0, 0, 0, 0,
		{ "deleted", "must-not-be-used", "carol-secret", NULL }
	};
	char passwords[4][DISK_SHARE_ACCOUNT_PASSWORD_VALUE_MAX];
	int i;

	for (i = 0; i < 4; ++i)
		strcpy(passwords[i], "stale");
	errno = 0;
	assert(disk_share_account_password_shift_snapshot(
		&fake_account_nvram_ops, &fake, 3, 0, 4, passwords) == -1);
	assert(errno == EIO);
	for (i = 0; i < 4; ++i)
		assert(passwords[i][0] == '\0');
	assert(fake.begin_calls == 1 && fake.end_calls == 1);
}

static void
test_account_password_shift_snapshot_end_error_discards_values(void)
{
	struct fake_account_nvram fake = {
		"3", "admin", { "alice", "bob", "carol", NULL },
		NULL, NULL, 0, 0, EBUSY, 0, 0, 0,
		{ "deleted", "bob-secret", "carol-secret", NULL }
	};
	char passwords[4][DISK_SHARE_ACCOUNT_PASSWORD_VALUE_MAX];

	assert(disk_share_account_password_shift_snapshot(
		&fake_account_nvram_ops, &fake, 3, 0, 4, passwords) == -1);
	assert(errno == EBUSY);
	assert(passwords[0][0] == '\0' && passwords[1][0] == '\0');
}

static void
test_account_password_shift_snapshot_rejects_oversized_value(void)
{
	struct fake_account_nvram fake = {
		"2", "admin", { "alice", "bob", NULL, NULL },
		NULL, NULL, 0, 0, 0, 0, 0, 0,
		{ "deleted", NULL, NULL, NULL }
	};
	char oversized[DISK_SHARE_ACCOUNT_PASSWORD_VALUE_MAX + 1];
	char passwords[4][DISK_SHARE_ACCOUNT_PASSWORD_VALUE_MAX];

	memset(oversized, 'x', sizeof(oversized) - 1);
	oversized[sizeof(oversized) - 1] = '\0';
	fake.passwords[1] = oversized;
	assert(disk_share_account_password_shift_snapshot(
		&fake_account_nvram_ops, &fake, 2, 0, 4, passwords) == -1);
	assert(errno == ENAMETOOLONG);
	assert(passwords[0][0] == '\0');
}

static void
test_account_commit_policy_defaults_to_commit_for_missing_key(void)
{
	int required = -1;

	assert(disk_share_account_commit_required(0, NULL, &required) == 0);
	assert(required == 1);
}

static void
test_account_commit_policy_preserves_manual_commit_setting(void)
{
	int required = -1;

	assert(disk_share_account_commit_required(1, "1", &required) == 0);
	assert(required == 0);
	assert(disk_share_account_commit_required(1, "0", &required) == 0);
	assert(required == 1);
	assert(disk_share_account_commit_required(1, "2", &required) == 0);
	assert(required == 1);
}

static void
test_account_commit_policy_rejects_read_errors(void)
{
	int required = -1;

	errno = EACCES;
	assert(disk_share_account_commit_required(-1, NULL, &required) == -1);
	assert(errno == EACCES);
	errno = 0;
	assert(disk_share_account_commit_required(-1, NULL, &required) == -1);
	assert(errno == EIO);
	assert(required == 0);
}

static void
test_account_commit_policy_rejects_inconsistent_results(void)
{
	int required = -1;

	assert(disk_share_account_commit_required(1, NULL, &required) == -1);
	assert(errno == EIO);
	assert(disk_share_account_commit_required(2, "1", &required) == -1);
	assert(errno == EIO);
	assert(disk_share_account_commit_required(0, NULL, NULL) == -1);
	assert(errno == EINVAL);
}


static void
test_account_mode_policy_defaults_missing_values(void)
{
	int needs_default = -1;

	assert(disk_share_account_mode_needs_default(0, NULL,
		&needs_default) == 0);
	assert(needs_default == 1);
}

static void
test_account_mode_policy_preserves_supported_modes(void)
{
	int needs_default = -1;

	assert(disk_share_account_mode_needs_default(1, "1",
		&needs_default) == 0);
	assert(needs_default == 0);
	assert(disk_share_account_mode_needs_default(1, "3",
		&needs_default) == 0);
	assert(needs_default == 0);
	assert(disk_share_account_mode_needs_default(1, "0",
		&needs_default) == 0);
	assert(needs_default == 1);
	assert(disk_share_account_mode_needs_default(1, "4",
		&needs_default) == 0);
	assert(needs_default == 1);
}

static void
test_account_mode_policy_rejects_backend_and_malformed_values(void)
{
	int needs_default = -1;

	errno = EBUSY;
	assert(disk_share_account_mode_needs_default(-1, NULL,
		&needs_default) == -1);
	assert(errno == EBUSY);
	assert(disk_share_account_mode_needs_default(2, "1",
		&needs_default) == -1);
	assert(errno == EIO);
	assert(disk_share_account_mode_needs_default(1, NULL,
		&needs_default) == -1);
	assert(errno == EIO);
	assert(disk_share_account_mode_needs_default(1, "1x",
		&needs_default) == -1);
	assert(errno == EINVAL);
	assert(disk_share_account_mode_needs_default(1, "999999999999999999999999",
		&needs_default) == -1);
	assert(errno == EINVAL);
	assert(disk_share_account_mode_needs_default(1, "1",
		NULL) == -1);
	assert(errno == EINVAL);
}


static void
test_account_rename_batch_rolls_back_prior_moves(void)
{
	struct disk_share_account_rename_entry entries[3];
	struct fake_account_rename_state state;
	int rollback_error = -1;

	memset(entries, 0, sizeof(entries));
	strcpy(entries[0].source, "/a/.__old_var.txt");
	strcpy(entries[0].destination, "/a/.__new_var.txt");
	strcpy(entries[1].source, "/b/.__old_var.txt");
	strcpy(entries[1].destination, "/b/.__new_var.txt");
	strcpy(entries[2].source, "/c/.__old_var.txt");
	strcpy(entries[2].destination, "/c/.__new_var.txt");
	entries[0].next = &entries[1];
	entries[1].previous = &entries[0];
	entries[1].next = &entries[2];
	entries[2].previous = &entries[1];

	state.paths[0] = entries[0].source;
	state.paths[1] = entries[1].source;
	state.paths[2] = entries[2].source;
	state.paths[3] = NULL;
	state.calls = 0;
	state.fail_call = 2;
	state.rollback_fail_call = 0;
	fake_account_rename_state = &state;

	errno = 0;
	assert(disk_share_account_rename_entries_apply(
		&entries[0], fake_account_rename, &rollback_error) == -1);
	assert(errno == EACCES);
	assert(rollback_error == 0);
	assert(strcmp(state.paths[0], entries[0].source) == 0);
	assert(strcmp(state.paths[1], entries[1].source) == 0);
	assert(strcmp(state.paths[2], entries[2].source) == 0);
	assert(state.calls == 3);
}

static void
test_account_rename_batch_reports_incomplete_rollback(void)
{
	struct disk_share_account_rename_entry entries[2];
	struct fake_account_rename_state state;
	int rollback_error = 0;

	memset(entries, 0, sizeof(entries));
	strcpy(entries[0].source, "/a/.__old_var.txt");
	strcpy(entries[0].destination, "/a/.__new_var.txt");
	strcpy(entries[1].source, "/b/.__old_var.txt");
	strcpy(entries[1].destination, "/b/.__new_var.txt");
	entries[0].next = &entries[1];
	entries[1].previous = &entries[0];

	state.paths[0] = entries[0].source;
	state.paths[1] = entries[1].source;
	state.paths[2] = NULL;
	state.paths[3] = NULL;
	state.calls = 0;
	state.fail_call = 2;
	state.rollback_fail_call = 3;
	fake_account_rename_state = &state;

	errno = 0;
	assert(disk_share_account_rename_entries_apply(
		&entries[0], fake_account_rename, &rollback_error) == -1);
	assert(errno == EACCES);
	assert(rollback_error == EROFS);
	assert(strcmp(state.paths[0], entries[0].destination) == 0);
	assert(strcmp(state.paths[1], entries[1].source) == 0);
	assert(state.calls == 3);
}


static void
test_account_service_command_status_accepts_exit_zero(void)
{
	assert(disk_share_command_status(0, EACCES) == 0);
}

static void
test_account_service_command_status_rejects_nonzero_exit_and_signal(void)
{
	assert(disk_share_command_status(1 << 8, 0) == EIO);
	assert(disk_share_command_status(SIGTERM, 0) == EIO);
}

static void
test_account_service_command_status_preserves_invocation_errno(void)
{
	assert(disk_share_command_status(-1, EACCES) == EACCES);
	assert(disk_share_command_status(-1, 0) == EIO);
}


static void
test_account_rename_batch_rolls_back_after_successful_publish(void)
{
	struct disk_share_account_rename_entry entries[2];
	struct fake_account_rename_state state;
	int rollback_error = -1;

	memset(entries, 0, sizeof(entries));
	strcpy(entries[0].source, "/a/.__old_var.txt");
	strcpy(entries[0].destination, "/a/.__new_var.txt");
	strcpy(entries[1].source, "/b/.__old_var.txt");
	strcpy(entries[1].destination, "/b/.__new_var.txt");
	entries[0].next = &entries[1];
	entries[1].previous = &entries[0];

	state.paths[0] = entries[0].source;
	state.paths[1] = entries[1].source;
	state.paths[2] = NULL;
	state.paths[3] = NULL;
	state.calls = 0;
	state.fail_call = 0;
	state.rollback_fail_call = 0;
	fake_account_rename_state = &state;

	assert(disk_share_account_rename_entries_apply(
		&entries[0], fake_account_rename, &rollback_error) == 0);
	assert(entries[0].renamed && entries[1].renamed);
	assert(disk_share_account_rename_entries_rollback(
		&entries[0], fake_account_rename, &rollback_error) == 0);
	assert(rollback_error == 0);
	assert(!entries[0].renamed && !entries[1].renamed);
	assert(strcmp(state.paths[0], entries[0].source) == 0);
	assert(strcmp(state.paths[1], entries[1].source) == 0);
	assert(state.calls == 4);
}

static void
test_account_rename_batch_reports_failed_compensation(void)
{
	struct disk_share_account_rename_entry entries[2];
	struct fake_account_rename_state state;
	int rollback_error = 0;

	memset(entries, 0, sizeof(entries));
	strcpy(entries[0].source, "/a/.__old_var.txt");
	strcpy(entries[0].destination, "/a/.__new_var.txt");
	strcpy(entries[1].source, "/b/.__old_var.txt");
	strcpy(entries[1].destination, "/b/.__new_var.txt");
	entries[0].next = &entries[1];
	entries[1].previous = &entries[0];

	state.paths[0] = entries[0].source;
	state.paths[1] = entries[1].source;
	state.paths[2] = NULL;
	state.paths[3] = NULL;
	state.calls = 0;
	state.fail_call = 0;
	state.rollback_fail_call = 3;
	fake_account_rename_state = &state;

	assert(disk_share_account_rename_entries_apply(
		&entries[0], fake_account_rename, &rollback_error) == 0);
	assert(disk_share_account_rename_entries_rollback(
		&entries[0], fake_account_rename, &rollback_error) == -1);
	assert(rollback_error == EROFS);
	assert(!entries[0].renamed);
	assert(entries[1].renamed);
}


struct fake_account_unlink_state {
	const char *paths[4];
	int present[4];
	int calls;
	int fail_call;
};

static struct fake_account_unlink_state *fake_account_unlink_state;

static int
fake_account_unlink(const char *path)
{
	int i;
	struct fake_account_unlink_state *state = fake_account_unlink_state;

	++state->calls;
	if (state->calls == state->fail_call) {
		errno = EACCES;
		return -1;
	}
	for (i = 0; i < 4; ++i) {
		if (state->present[i] && state->paths[i] &&
		    strcmp(state->paths[i], path) == 0) {
			state->present[i] = 0;
			return 0;
		}
	}
	errno = ENOENT;
	return -1;
}

static void
test_account_created_file_rollback_removes_all_created_files(void)
{
	struct disk_share_account_created_file_entry entries[3];
	struct fake_account_unlink_state state;
	int rollback_error = -1;

	memset(entries, 0, sizeof(entries));
	strcpy(entries[0].path, "/a/.router/alice_var.txt");
	strcpy(entries[1].path, "/b/.router/alice_var.txt");
	strcpy(entries[2].path, "/c/.router/alice_var.txt");
	entries[0].created = entries[1].created = entries[2].created = 1;
	entries[0].next = &entries[1];
	entries[1].previous = &entries[0];
	entries[1].next = &entries[2];
	entries[2].previous = &entries[1];

	state.paths[0] = entries[0].path;
	state.paths[1] = entries[1].path;
	state.paths[2] = entries[2].path;
	state.paths[3] = NULL;
	state.present[0] = state.present[1] = state.present[2] = 1;
	state.present[3] = 0;
	state.calls = 0;
	state.fail_call = 0;
	fake_account_unlink_state = &state;

	assert(disk_share_account_created_files_rollback(
		&entries[0], fake_account_unlink, &rollback_error) == 0);
	assert(rollback_error == 0);
	assert(state.calls == 3);
	assert(!state.present[0] && !state.present[1] && !state.present[2]);
	assert(!entries[0].created && !entries[1].created && !entries[2].created);
}

static void
test_account_created_file_rollback_reports_failures_and_continues(void)
{
	struct disk_share_account_created_file_entry entries[3];
	struct fake_account_unlink_state state;
	int rollback_error = 0;

	memset(entries, 0, sizeof(entries));
	strcpy(entries[0].path, "/a/.router/alice_var.txt");
	strcpy(entries[1].path, "/b/.router/alice_var.txt");
	strcpy(entries[2].path, "/c/.router/alice_var.txt");
	entries[0].created = entries[1].created = entries[2].created = 1;
	entries[0].next = &entries[1];
	entries[1].previous = &entries[0];
	entries[1].next = &entries[2];
	entries[2].previous = &entries[1];

	state.paths[0] = entries[0].path;
	state.paths[1] = entries[1].path;
	state.paths[2] = entries[2].path;
	state.paths[3] = NULL;
	state.present[0] = state.present[1] = state.present[2] = 1;
	state.present[3] = 0;
	state.calls = 0;
	state.fail_call = 2;
	fake_account_unlink_state = &state;

	assert(disk_share_account_created_files_rollback(
		&entries[0], fake_account_unlink, &rollback_error) == -1);
	assert(rollback_error == EACCES);
	assert(state.calls == 3);
	assert(!state.present[0] && state.present[1] && !state.present[2]);
	assert(!entries[0].created && entries[1].created && !entries[2].created);
}


struct fake_account_delete_nvram {
	char count[128];
	int count_present;
	char usernames[4][128];
	int username_present[4];
	char passwords[4][128];
	int password_present[4];
	char manual[128];
	int manual_present;
	char samba_mode[128];
	int samba_mode_present;
	char ftp_mode[128];
	int ftp_mode_present;
	int begin_calls;
	int get_calls;
	int set_calls;
	int commit_calls;
	int end_calls;
	int fail_get_call;
	int fail_set_call;
	int rollback_fail_set_call;
	int fail_commit_count;
	int end_error;
};

static int
fake_account_delete_begin(void *context)
{
	struct fake_account_delete_nvram *fake = context;

	++fake->begin_calls;
	return 0;
}

static int
fake_account_delete_get(void *context, const char *name, char **value)
{
	struct fake_account_delete_nvram *fake = context;
	int index;

	++fake->get_calls;
	*value = NULL;
	if (fake->fail_get_call == fake->get_calls) {
		errno = EACCES;
		return -1;
	}
	if (!strcmp(name, "acc_num")) {
		if (!fake->count_present)
			return 0;
		*value = fake->count;
		return 1;
	}
	if (!strcmp(name, "nvram_manual")) {
		if (!fake->manual_present)
			return 0;
		*value = fake->manual;
		return 1;
	}
	if (!strcmp(name, "st_samba_mode")) {
		if (!fake->samba_mode_present)
			return 0;
		*value = fake->samba_mode;
		return 1;
	}
	if (!strcmp(name, "st_ftp_mode")) {
		if (!fake->ftp_mode_present)
			return 0;
		*value = fake->ftp_mode;
		return 1;
	}
	if (sscanf(name, "acc_username%d", &index) == 1 &&
	    index >= 0 && index < 4) {
		if (!fake->username_present[index])
			return 0;
		*value = fake->usernames[index];
		return 1;
	}
	if (sscanf(name, "acc_password%d", &index) == 1 &&
	    index >= 0 && index < 4) {
		if (!fake->password_present[index])
			return 0;
		*value = fake->passwords[index];
		return 1;
	}
	return 0;
}

static int
fake_account_delete_set(void *context, const char *name, const char *value)
{
	struct fake_account_delete_nvram *fake = context;
	char *destination = NULL;
	int *present = NULL;
	int index = -1;

	++fake->set_calls;
	if (!strcmp(name, "acc_num")) {
		destination = fake->count;
		present = &fake->count_present;
	} else if (!strcmp(name, "st_samba_mode")) {
		destination = fake->samba_mode;
		present = &fake->samba_mode_present;
	} else if (!strcmp(name, "st_ftp_mode")) {
		destination = fake->ftp_mode;
		present = &fake->ftp_mode_present;
	} else if (sscanf(name, "acc_username%d", &index) == 1 &&
		   index >= 0 && index < 4) {
		destination = fake->usernames[index];
		present = &fake->username_present[index];
	} else if (sscanf(name, "acc_password%d", &index) == 1 &&
		   index >= 0 && index < 4) {
		destination = fake->passwords[index];
		present = &fake->password_present[index];
	} else {
		errno = EINVAL;
		return -1;
	}

	/* Simulate an ambiguous close error: the key changes but the call fails. */
	if (value) {
		snprintf(destination, 128, "%s", value);
		*present = 1;
	} else {
		destination[0] = '\0';
		*present = 0;
	}
	if (fake->set_calls == fake->fail_set_call) {
		errno = EACCES;
		return -1;
	}
	if (fake->set_calls == fake->rollback_fail_set_call) {
		errno = EROFS;
		return -1;
	}
	return 0;
}

static int
fake_account_delete_commit(void *context)
{
	struct fake_account_delete_nvram *fake = context;

	++fake->commit_calls;
	if (fake->fail_commit_count > 0) {
		--fake->fail_commit_count;
		errno = EIO;
		return -1;
	}
	return 0;
}

static int
fake_account_delete_end(void *context)
{
	struct fake_account_delete_nvram *fake = context;

	++fake->end_calls;
	if (fake->end_error) {
		errno = EIO;
		return -1;
	}
	return 0;
}

static const struct disk_share_account_nvram_ops fake_account_delete_ops = {
	fake_account_delete_begin,
	fake_account_delete_get,
	fake_account_delete_end,
	fake_account_name_valid,
	fake_account_name_reserved,
	fake_account_delete_set,
	fake_account_delete_commit
};

static void
fake_account_delete_init_three(struct fake_account_delete_nvram *fake)
{
	memset(fake, 0, sizeof(*fake));
	strcpy(fake->count, "3");
	fake->count_present = 1;
	strcpy(fake->usernames[0], "alice");
	strcpy(fake->usernames[1], "bob");
	strcpy(fake->usernames[2], "carol");
	fake->username_present[0] = 1;
	fake->username_present[1] = 1;
	fake->username_present[2] = 1;
	strcpy(fake->passwords[0], "alice-secret");
	strcpy(fake->passwords[1], "bob-secret");
	strcpy(fake->passwords[2], "carol-secret");
	fake->password_present[0] = 1;
	fake->password_present[1] = 1;
	fake->password_present[2] = 1;
}

static int
fake_account_inventory_add_run(struct fake_account_delete_nvram *fake,
			       int expected_count, int max_count,
			       char *const *expected_users,
			       const char *account, const char *password)
{
	int result, saved_errno, end_result, end_errno;

	if (fake_account_delete_begin(fake) != 0)
		return -1;
	result = disk_share_account_inventory_add_locked(
		&fake_account_delete_ops, fake, expected_count, max_count,
		expected_users, account, password);
	saved_errno = errno;
	errno = 0;
	end_result = fake_account_delete_end(fake);
	end_errno = errno;
	if (result != 0) {
		errno = saved_errno ? saved_errno : EIO;
		return -1;
	}
	if (end_result != 0) {
		errno = end_errno ? end_errno : EIO;
		return -1;
	}
	return 0;
}

static void
test_account_inventory_add_appends_under_locked_snapshot(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	assert(fake_account_inventory_add_run(
		&fake, 3, 4, users, "dave", "dave-secret") == 0);
	assert(strcmp(fake.count, "4") == 0);
	assert(strcmp(fake.usernames[3], "dave") == 0);
	assert(strcmp(fake.passwords[3], "dave-secret") == 0);
	assert(fake.begin_calls == 1 && fake.end_calls == 1);
	assert(fake.commit_calls == 1);
}

static void
test_account_inventory_add_handles_empty_inventory(void)
{
	struct fake_account_delete_nvram fake;

	memset(&fake, 0, sizeof(fake));
	assert(fake_account_inventory_add_run(
		&fake, 0, 4, NULL, "alice", "alice-secret") == 0);
	assert(strcmp(fake.count, "1") == 0);
	assert(strcmp(fake.usernames[0], "alice") == 0);
	assert(strcmp(fake.passwords[0], "alice-secret") == 0);
	assert(fake.commit_calls == 1);
	assert(fake.begin_calls == 1 && fake.end_calls == 1);
}

static void
test_account_inventory_add_honors_manual_commit(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	strcpy(fake.manual, "1");
	fake.manual_present = 1;
	assert(fake_account_inventory_add_run(
		&fake, 3, 4, users, "dave", "dave-secret") == 0);
	assert(fake.commit_calls == 0);
	assert(strcmp(fake.count, "4") == 0);
}

static void
test_account_inventory_add_rejects_duplicate_and_stale_inventory(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	errno = 0;
	assert(fake_account_inventory_add_run(
		&fake, 3, 4, users, "bob", "new-secret") == -1);
	assert(errno == EEXIST);
	assert(fake.set_calls == 0 && fake.commit_calls == 0);

	fake_account_delete_init_three(&fake);
	strcpy(fake.usernames[1], "mallory");
	errno = 0;
	assert(fake_account_inventory_add_run(
		&fake, 3, 4, users, "dave", "dave-secret") == -1);
	assert(errno == EAGAIN);
	assert(fake.set_calls == 0 && fake.commit_calls == 0);
}

static void
test_account_inventory_add_rejects_full_and_reserved_inventory(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	errno = 0;
	assert(fake_account_inventory_add_run(
		&fake, 4, 4, users, "dave", "dave-secret") == -1);
	assert(errno == ENOSPC);
	assert(fake.set_calls == 0);

	fake_account_delete_init_three(&fake);
	errno = 0;
	assert(fake_account_inventory_add_run(
		&fake, 3, 4, users, "root", "root-secret") == -1);
	assert(errno == EPERM);
	assert(fake.set_calls == 0);
}

static void
test_account_inventory_add_rejects_tail_snapshot_errors(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	fake.fail_get_call = 6;
	errno = 0;
	assert(fake_account_inventory_add_run(
		&fake, 3, 4, users, "dave", "dave-secret") == -1);
	assert(errno == EACCES);
	assert(fake.set_calls == 0 && fake.commit_calls == 0);

	fake_account_delete_init_three(&fake);
	fake.fail_get_call = 8;
	errno = 0;
	assert(fake_account_inventory_add_run(
		&fake, 3, 4, users, "dave", "dave-secret") == -1);
	assert(errno == EACCES);
	assert(fake.set_calls == 0 && fake.commit_calls == 0);
}

static void
test_account_inventory_add_rolls_back_each_failed_setter(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };
	int failure;

	for (failure = 1; failure <= 3; ++failure) {
		fake_account_delete_init_three(&fake);
		fake.fail_set_call = failure;
		errno = 0;
		assert(fake_account_inventory_add_run(
			&fake, 3, 4, users, "dave", "dave-secret") == -1);
		assert(errno == EACCES);
		assert(strcmp(fake.count, "3") == 0);
		assert(!fake.username_present[3]);
		assert(!fake.password_present[3]);
		assert(fake.commit_calls == 1);
		assert(fake.begin_calls == 1 && fake.end_calls == 1);
	}
}

static void
test_account_inventory_add_rolls_back_failed_commit(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	fake.fail_commit_count = 1;
	errno = 0;
	assert(fake_account_inventory_add_run(
		&fake, 3, 4, users, "dave", "dave-secret") == -1);
	assert(errno == EIO);
	assert(strcmp(fake.count, "3") == 0);
	assert(!fake.username_present[3]);
	assert(!fake.password_present[3]);
	assert(fake.commit_calls == 2);
}

static void
test_account_inventory_add_reports_rollback_failure(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	fake.fail_set_call = 2;
	fake.rollback_fail_set_call = 3;
	errno = 0;
	assert(fake_account_inventory_add_run(
		&fake, 3, 4, users, "dave", "dave-secret") == -1);
	assert(errno == EIO);
	assert(strcmp(fake.count, "3") == 0);
	assert(!fake.username_present[3]);
	assert(fake.commit_calls == 1);
}

static int
fake_account_inventory_update_run(struct fake_account_delete_nvram *fake,
				  int expected_count, int max_count,
				  char *const *expected_users, int target,
				  const char *old_account, const char *new_account,
				  const char *new_password)
{
	int result, saved_errno, end_result, end_errno;

	if (fake_account_delete_begin(fake) != 0)
		return -1;
	if (disk_share_account_inventory_update_validate_locked(
		    &fake_account_delete_ops, fake, expected_count, max_count,
		    expected_users, target, old_account,
		    new_account && *new_account &&
		    strcmp(new_account, old_account) ? new_account : NULL) != 0) {
		result = -1;
	} else {
		result = disk_share_account_inventory_update_locked(
			&fake_account_delete_ops, fake, expected_count, max_count,
			expected_users, target, old_account, new_account,
			new_password);
	}
	saved_errno = errno;
	errno = 0;
	end_result = fake_account_delete_end(fake);
	end_errno = errno;
	if (result != 0) {
		errno = saved_errno ? saved_errno : EIO;
		return -1;
	}
	if (end_result != 0) {
		errno = end_errno ? end_errno : EIO;
		return -1;
	}
	return 0;
}

static void
test_account_inventory_update_renames_and_changes_password_atomically(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	assert(fake_account_inventory_update_run(
		&fake, 3, 4, users, 1, "bob", "robert", "robert-secret") == 0);
	assert(strcmp(fake.usernames[1], "robert") == 0);
	assert(strcmp(fake.passwords[1], "robert-secret") == 0);
	assert(strcmp(fake.count, "3") == 0);
	assert(fake.begin_calls == 1 && fake.end_calls == 1);
	assert(fake.commit_calls == 1);
}

static void
test_account_inventory_update_supports_password_only_and_manual_commit(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	strcpy(fake.manual, "1");
	fake.manual_present = 1;
	assert(fake_account_inventory_update_run(
		&fake, 3, 4, users, 0, "alice", NULL, "new-secret") == 0);
	assert(strcmp(fake.usernames[0], "alice") == 0);
	assert(strcmp(fake.passwords[0], "new-secret") == 0);
	assert(fake.commit_calls == 0);
}

static void
test_account_inventory_update_rejects_duplicate_reserved_and_stale(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	errno = 0;
	assert(fake_account_inventory_update_run(
		&fake, 3, 4, users, 0, "alice", "bob", NULL) == -1);
	assert(errno == EEXIST && fake.set_calls == 0);

	fake_account_delete_init_three(&fake);
	errno = 0;
	assert(fake_account_inventory_update_run(
		&fake, 3, 4, users, 0, "alice", "root", NULL) == -1);
	assert(errno == EPERM && fake.set_calls == 0);

	fake_account_delete_init_three(&fake);
	strcpy(fake.usernames[0], "mallory");
	errno = 0;
	assert(fake_account_inventory_update_run(
		&fake, 3, 4, users, 0, "alice", "anna", NULL) == -1);
	assert(errno == EAGAIN && fake.set_calls == 0);

	{
		char *malformed_users[3] = { "alice", NULL, "carol" };

		fake_account_delete_init_three(&fake);
		errno = 0;
		assert(fake_account_inventory_update_run(
			&fake, 3, 4, malformed_users, 1, "bob", "robert", NULL) == -1);
		assert(errno == EINVAL && fake.set_calls == 0);
	}
}

static void
test_account_inventory_update_rolls_back_ambiguous_setters(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };
	int failure;

	for (failure = 1; failure <= 2; ++failure) {
		fake_account_delete_init_three(&fake);
		fake.fail_set_call = failure;
		errno = 0;
		assert(fake_account_inventory_update_run(
			&fake, 3, 4, users, 0, "alice", "anna", "new-secret") == -1);
		assert(errno == EACCES);
		assert(strcmp(fake.usernames[0], "alice") == 0);
		assert(strcmp(fake.passwords[0], "alice-secret") == 0);
		assert(fake.commit_calls == 1);
	}
}

static void
test_account_inventory_update_rolls_back_failed_commit_and_absent_password(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	fake.password_present[0] = 0;
	fake.passwords[0][0] = '\0';
	fake.fail_commit_count = 1;
	errno = 0;
	assert(fake_account_inventory_update_run(
		&fake, 3, 4, users, 0, "alice", "anna", "new-secret") == -1);
	assert(errno == EIO);
	assert(strcmp(fake.usernames[0], "alice") == 0);
	assert(!fake.password_present[0]);
	assert(fake.commit_calls == 2);
}

static void
test_account_inventory_update_rejects_snapshot_and_manual_read_errors(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	fake.fail_get_call = 11;
	errno = 0;
	assert(fake_account_inventory_update_run(
		&fake, 3, 4, users, 0, "alice", NULL, "new-secret") == -1);
	assert(errno == EACCES && fake.set_calls == 0);

	fake_account_delete_init_three(&fake);
	fake.fail_get_call = 12;
	errno = 0;
	assert(fake_account_inventory_update_run(
		&fake, 3, 4, users, 0, "alice", NULL, "new-secret") == -1);
	assert(errno == EACCES && fake.set_calls == 0);
}

static void
test_account_inventory_remove_compacts_under_one_transaction(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	assert(disk_share_account_inventory_remove(
		&fake_account_delete_ops, &fake, 3, 1, 4, users) == 0);
	assert(strcmp(fake.count, "2") == 0);
	assert(strcmp(fake.usernames[0], "alice") == 0);
	assert(strcmp(fake.usernames[1], "carol") == 0);
	assert(strcmp(fake.passwords[1], "carol-secret") == 0);
	assert(fake.begin_calls == 1 && fake.end_calls == 1);
	assert(fake.commit_calls == 1);
}

static void
test_account_inventory_remove_defaults_last_account_modes(void)
{
	struct fake_account_delete_nvram fake;
	char *users[1] = { "alice" };

	memset(&fake, 0, sizeof(fake));
	strcpy(fake.count, "1");
	fake.count_present = 1;
	strcpy(fake.usernames[0], "alice");
	fake.username_present[0] = 1;
	strcpy(fake.passwords[0], "alice-secret");
	fake.password_present[0] = 1;
	strcpy(fake.samba_mode, "0");
	fake.samba_mode_present = 1;
	strcpy(fake.ftp_mode, "3");
	fake.ftp_mode_present = 1;

	assert(disk_share_account_inventory_remove(
		&fake_account_delete_ops, &fake, 1, 0, 4, users) == 0);
	assert(strcmp(fake.count, "0") == 0);
	assert(strcmp(fake.samba_mode, "1") == 0);
	assert(strcmp(fake.ftp_mode, "3") == 0);
	assert(fake.commit_calls == 1);
}

static void
test_account_inventory_remove_rolls_back_last_account_mode_setter(void)
{
	struct fake_account_delete_nvram fake;
	char *users[1] = { "alice" };

	memset(&fake, 0, sizeof(fake));
	strcpy(fake.count, "1");
	fake.count_present = 1;
	strcpy(fake.usernames[0], "alice");
	fake.username_present[0] = 1;
	strcpy(fake.samba_mode, "0");
	fake.samba_mode_present = 1;
	strcpy(fake.ftp_mode, "0");
	fake.ftp_mode_present = 1;
	fake.fail_set_call = 2;

	errno = 0;
	assert(disk_share_account_inventory_remove(
		&fake_account_delete_ops, &fake, 1, 0, 4, users) == -1);
	assert(errno == EACCES);
	assert(strcmp(fake.count, "1") == 0);
	assert(strcmp(fake.samba_mode, "0") == 0);
	assert(strcmp(fake.ftp_mode, "0") == 0);
	assert(fake.commit_calls == 1);
}


static void
test_account_inventory_remove_honors_manual_commit(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	strcpy(fake.manual, "1");
	fake.manual_present = 1;
	assert(disk_share_account_inventory_remove(
		&fake_account_delete_ops, &fake, 3, 1, 4, users) == 0);
	assert(fake.commit_calls == 0);
	assert(strcmp(fake.count, "2") == 0);
}

static void
test_account_inventory_remove_rejects_stale_inventory(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	strcpy(fake.usernames[1], "mallory");
	errno = 0;
	assert(disk_share_account_inventory_remove(
		&fake_account_delete_ops, &fake, 3, 1, 4, users) == -1);
	assert(errno == EAGAIN);
	assert(fake.set_calls == 0 && fake.commit_calls == 0);
	assert(fake.end_calls == 1);
}

static void
test_account_inventory_remove_rolls_back_failed_set(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	fake.fail_set_call = 3;
	errno = 0;
	assert(disk_share_account_inventory_remove(
		&fake_account_delete_ops, &fake, 3, 1, 4, users) == -1);
	assert(errno == EACCES);
	assert(strcmp(fake.count, "3") == 0);
	assert(strcmp(fake.usernames[1], "bob") == 0);
	assert(strcmp(fake.passwords[1], "bob-secret") == 0);
	assert(fake.commit_calls == 1);
	assert(fake.end_calls == 1);
}

static void
test_account_inventory_remove_rolls_back_failed_commit(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	fake.fail_commit_count = 1;
	errno = 0;
	assert(disk_share_account_inventory_remove(
		&fake_account_delete_ops, &fake, 3, 1, 4, users) == -1);
	assert(errno == EIO);
	assert(strcmp(fake.count, "3") == 0);
	assert(strcmp(fake.usernames[1], "bob") == 0);
	assert(strcmp(fake.passwords[1], "bob-secret") == 0);
	assert(fake.commit_calls == 2);
}

static void
test_account_inventory_remove_reports_rollback_failure(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	fake.fail_set_call = 3;
	fake.rollback_fail_set_call = 4;
	errno = 0;
	assert(disk_share_account_inventory_remove(
		&fake_account_delete_ops, &fake, 3, 1, 4, users) == -1);
	assert(errno == EIO);
	assert(strcmp(fake.count, "3") == 0);
	assert(strcmp(fake.usernames[1], "bob") == 0);
	assert(strcmp(fake.passwords[1], "bob-secret") == 0);
}

static void
test_account_inventory_remove_rejects_password_read_error(void)
{
	struct fake_account_delete_nvram fake;
	char *users[3] = { "alice", "bob", "carol" };

	fake_account_delete_init_three(&fake);
	fake.fail_get_call = 5;
	errno = 0;
	assert(disk_share_account_inventory_remove(
		&fake_account_delete_ops, &fake, 3, 1, 4, users) == -1);
	assert(errno == EACCES);
	assert(fake.set_calls == 0 && fake.commit_calls == 0);
}


int
main(void)
{
	assert(router_account_name_is_valid("storage"));
	assert(router_account_name_is_reserved_after_read("admin", 1, "admin"));
	assert(!router_account_name_is_reserved_after_read("storage", 1, "admin"));
	assert(!router_account_name_is_reserved_after_read("storage", 0, NULL));
	assert(router_account_name_is_reserved_after_read("admin", -1, NULL));
	assert(router_account_name_is_reserved_after_read("admin", 1, NULL));

	test_invalid_mount_paths_clear_outputs();
	test_missing_output_parameters_are_safe();
	test_valid_mount_path_starts_with_empty_outputs();
	test_account_var_path_rejects_truncation();
	test_account_rename_status_accepts_success_and_missing_source();
	test_account_rename_status_preserves_real_errors();
	test_account_rename_batch_rolls_back_prior_moves();
	test_account_rename_batch_reports_incomplete_rollback();
	test_account_rename_batch_rolls_back_after_successful_publish();
	test_account_rename_batch_reports_failed_compensation();
	test_account_created_file_rollback_removes_all_created_files();
	test_account_created_file_rollback_reports_failures_and_continues();
	test_account_service_command_status_accepts_exit_zero();
	test_account_service_command_status_rejects_nonzero_exit_and_signal();
	test_account_service_command_status_preserves_invocation_errno();
	test_permission_rewrite_removes_complete_record();
	test_permission_rewrite_renames_key_and_preserves_rights();
	test_permission_rewrite_reports_missing_record_and_clears_outputs();
	test_readdir_entry_and_clean_eof();
	test_readdir_error_is_not_clean_eof();
	test_readdir_rejects_missing_adapters();
	test_snapshot_write_flushes_complete_content();
	test_snapshot_write_reports_io_failure();
	test_snapshot_write_rejects_invalid_data();
	test_account_defaults_write_complete_content();
	test_account_defaults_write_reports_io_failure();
	test_account_defaults_write_rejects_invalid_data();
	test_account_inventory_reads_one_checked_snapshot();
	test_account_inventory_missing_count_is_empty();
	test_account_inventory_count_read_error_is_not_zero();
	test_account_inventory_username_error_frees_partial_list();
	test_account_inventory_transaction_end_error_discards_snapshot();
	test_account_inventory_begin_error_does_not_end_transaction();
	test_account_inventory_rejects_invalid_count_text_and_range();
	test_account_inventory_rejects_sparse_and_reserved_names();
	test_account_inventory_rejects_invalid_get_status();
	test_account_inventory_rejects_duplicate_names();
	test_account_password_shift_snapshot_copies_values_in_one_transaction();
	test_account_password_shift_snapshot_preserves_missing_as_empty();
	test_account_password_shift_snapshot_read_error_clears_all_values();
	test_account_password_shift_snapshot_end_error_discards_values();
	test_account_password_shift_snapshot_rejects_oversized_value();
	test_account_inventory_add_appends_under_locked_snapshot();
	test_account_inventory_add_handles_empty_inventory();
	test_account_inventory_add_honors_manual_commit();
	test_account_inventory_add_rejects_duplicate_and_stale_inventory();
	test_account_inventory_add_rejects_full_and_reserved_inventory();
	test_account_inventory_add_rejects_tail_snapshot_errors();
	test_account_inventory_add_rolls_back_each_failed_setter();
	test_account_inventory_add_rolls_back_failed_commit();
	test_account_inventory_add_reports_rollback_failure();
	test_account_inventory_update_renames_and_changes_password_atomically();
	test_account_inventory_update_supports_password_only_and_manual_commit();
	test_account_inventory_update_rejects_duplicate_reserved_and_stale();
	test_account_inventory_update_rolls_back_ambiguous_setters();
	test_account_inventory_update_rolls_back_failed_commit_and_absent_password();
	test_account_inventory_update_rejects_snapshot_and_manual_read_errors();
	test_account_inventory_remove_compacts_under_one_transaction();
	test_account_inventory_remove_defaults_last_account_modes();
	test_account_inventory_remove_rolls_back_last_account_mode_setter();
	test_account_inventory_remove_honors_manual_commit();
	test_account_inventory_remove_rejects_stale_inventory();
	test_account_inventory_remove_rolls_back_failed_set();
	test_account_inventory_remove_rolls_back_failed_commit();
	test_account_inventory_remove_reports_rollback_failure();
	test_account_inventory_remove_rejects_password_read_error();
	test_account_commit_policy_defaults_to_commit_for_missing_key();
	test_account_commit_policy_preserves_manual_commit_setting();
	test_account_commit_policy_rejects_read_errors();
	test_account_commit_policy_rejects_inconsistent_results();
	test_account_mode_policy_defaults_missing_values();
	test_account_mode_policy_preserves_supported_modes();
	test_account_mode_policy_rejects_backend_and_malformed_values();

	return 0;
}
