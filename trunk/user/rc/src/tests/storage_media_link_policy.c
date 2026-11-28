#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "storage_media_link_policy.h"

static int
test_known_media_link_pairs_are_enforced(void)
{
	assert(storage_media_link_pair_status("transmission", "/mnt/transmission") == 0);
	assert(storage_media_link_pair_status("aria", "/mnt/aria") == 0);
	assert(storage_media_link_pair_status(".dms", "/mnt/minidlna") == 0);
	assert(storage_media_link_pair_status(".itunes", "/mnt/firefly") == 0);
	assert(storage_media_link_pair_status("transmission", "/mnt/aria") == EINVAL);
	assert(storage_media_link_pair_status("aria", "/mnt/transmission") == EINVAL);
	assert(storage_media_link_pair_status("other", "/mnt/other") == EINVAL);
	assert(storage_media_link_pair_status(NULL, "/mnt/aria") == EINVAL);
	assert(storage_media_link_pair_status("aria", NULL) == EINVAL);
	return 0;
}

static int
test_root_owned_nonwritable_parent_is_required(void)
{
	assert(storage_media_link_parent_status(0, 0, 1, 0, 0, 0755) == 0);
	assert(storage_media_link_parent_status(0, 0, 1, 0, 0, 0700) == 0);
	assert(storage_media_link_parent_status(0, 0, 1, 0, 1000, 0755) == EPERM);
	return 0;
}

static int
test_symlink_and_writable_parents_are_rejected(void)
{
	assert(storage_media_link_parent_status(0, 0, 1, 1, 0, 0755) == EPERM);
	assert(storage_media_link_parent_status(0, 0, 0, 0, 0, 0755) == EPERM);
	assert(storage_media_link_parent_status(0, 0, 1, 0, 0, 0775) == EPERM);
	assert(storage_media_link_parent_status(0, 0, 1, 0, 0, 0757) == EPERM);
	return 0;
}

static int
test_only_existing_symlink_may_be_replaced(void)
{
	assert(storage_media_link_destination_status(0, 0, 1) == 0);
	assert(storage_media_link_destination_status(0, 0, 0) == EEXIST);
	assert(storage_media_link_destination_status(-1, ENOENT, 0) == 0);
	return 0;
}

static int
test_lookup_errors_are_preserved(void)
{
	assert(storage_media_link_parent_status(-1, EACCES, 0, 0, 0, 0755) == EACCES);
	assert(storage_media_link_parent_status(-1, 0, 0, 0, 0, 0755) == EIO);
	assert(storage_media_link_destination_status(-1, EACCES, 0) == EACCES);
	assert(storage_media_link_destination_status(-1, 0, 0) == EIO);
	return 0;
}

static int
test_media_target_must_be_a_real_directory(void)
{
	assert(storage_media_link_target_status(0, 0, 1, 0) == 0);
	assert(storage_media_link_target_status(0, 0, 1, 1) == EPERM);
	assert(storage_media_link_target_status(0, 0, 0, 0) == ENOTDIR);
	assert(storage_media_link_target_status(-1, ENOENT, 0, 0) == ENOENT);
	assert(storage_media_link_target_status(-1, EACCES, 0, 0) == EACCES);
	assert(storage_media_link_target_status(-1, 0, 0, 0) == EIO);
	return 0;
}

int
main(void)
{
	assert(test_known_media_link_pairs_are_enforced() == 0);
	assert(test_root_owned_nonwritable_parent_is_required() == 0);
	assert(test_symlink_and_writable_parents_are_rejected() == 0);
	assert(test_only_existing_symlink_may_be_replaced() == 0);
	assert(test_lookup_errors_are_preserved() == 0);
	assert(test_media_target_must_be_a_real_directory() == 0);
	puts("Storage media-link publication policy: PASS");
	return 0;
}
