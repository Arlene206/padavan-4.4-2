#include <assert.h>
#include <stddef.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "disk_share_list_policy.h"

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

int
main(void)
{
	test_invalid_mount_paths_clear_outputs();
	test_missing_output_parameters_are_safe();
	test_valid_mount_path_starts_with_empty_outputs();
	test_account_var_path_rejects_truncation();
	test_readdir_entry_and_clean_eof();
	test_readdir_error_is_not_clean_eof();
	test_readdir_rejects_missing_adapters();
	test_snapshot_write_flushes_complete_content();
	test_snapshot_write_reports_io_failure();
	test_snapshot_write_rejects_invalid_data();
	test_account_defaults_write_complete_content();
	test_account_defaults_write_reports_io_failure();
	test_account_defaults_write_rejects_invalid_data();
	return 0;
}
