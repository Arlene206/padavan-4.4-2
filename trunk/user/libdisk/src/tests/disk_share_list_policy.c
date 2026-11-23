#include <assert.h>
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
	return strcmp(name, "anonymous") == 0 ||
	       (http_username && *http_username &&
		strcmp(name, http_username) == 0);
}

static const struct disk_share_account_nvram_ops fake_account_nvram_ops = {
	fake_account_nvram_begin,
	fake_account_nvram_get,
	fake_account_nvram_end,
	fake_account_name_valid,
	fake_account_name_reserved
};

static void
test_account_inventory_reads_one_checked_snapshot(void)
{
	struct fake_account_nvram fake = {
		"2", "admin", { "alice", "bob", NULL, NULL },
		NULL, NULL, 0, 0, 0, 0, 0, 0
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
		NULL, NULL, 0, 0, 0, 0, 0, 0
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
		"acc_num", NULL, 0, 0, 0, 0, 0, 0
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
		"acc_username1", NULL, 0, 0, 0, 0, 0, 0
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
		NULL, NULL, 0, 0, EBUSY, 0, 0, 0
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
		NULL, NULL, 0, EACCES, 0, 0, 0, 0
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
		NULL, NULL, 0, 0, 0, 0, 0, 0
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
		NULL, NULL, 0, 0, 0, 0, 0, 0
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
		NULL, "acc_username0", 2, 0, 0, 0, 0, 0
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
		NULL, NULL, 0, 0, 0, 0, 0, 0
	};
	char **items = NULL;
	int count = 0;

	assert(disk_share_account_inventory_read(&fake_account_nvram_ops,
		&fake, 4, &count, &items) == -1);
	assert(errno == EINVAL);
	assert(count == 0 && items == NULL);
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

	return 0;
}
