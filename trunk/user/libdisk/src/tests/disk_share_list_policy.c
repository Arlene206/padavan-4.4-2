#include <assert.h>
#include <stddef.h>

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

int
main(void)
{
	test_invalid_mount_paths_clear_outputs();
	test_missing_output_parameters_are_safe();
	test_valid_mount_path_starts_with_empty_outputs();
	return 0;
}
