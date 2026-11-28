#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "transmission_link_policy.h"

static int
test_root_owned_nonwritable_parent_is_required(void)
{
	assert(transmission_link_parent_status(0, 0, 1, 0, 0, 0755) == 0);
	assert(transmission_link_parent_status(0, 0, 1, 0, 0, 0700) == 0);
	assert(transmission_link_parent_status(0, 0, 1, 0, 1000, 0755) == EPERM);
	return 0;
}

static int
test_symlink_and_writable_parents_are_rejected(void)
{
	assert(transmission_link_parent_status(0, 0, 1, 1, 0, 0755) == EPERM);
	assert(transmission_link_parent_status(0, 0, 0, 0, 0, 0755) == EPERM);
	assert(transmission_link_parent_status(0, 0, 1, 0, 0, 0775) == EPERM);
	assert(transmission_link_parent_status(0, 0, 1, 0, 0, 0757) == EPERM);
	return 0;
}

static int
test_only_existing_symlink_may_be_replaced(void)
{
	assert(transmission_link_destination_status(0, 0, 1) == 0);
	assert(transmission_link_destination_status(0, 0, 0) == EEXIST);
	assert(transmission_link_destination_status(-1, ENOENT, 0) == 0);
	return 0;
}

static int
test_lookup_errors_are_preserved(void)
{
	assert(transmission_link_parent_status(-1, EACCES, 0, 0, 0, 0755) == EACCES);
	assert(transmission_link_parent_status(-1, 0, 0, 0, 0, 0755) == EIO);
	assert(transmission_link_destination_status(-1, EACCES, 0) == EACCES);
	assert(transmission_link_destination_status(-1, 0, 0) == EIO);
	return 0;
}

int
main(void)
{
	assert(test_root_owned_nonwritable_parent_is_required() == 0);
	assert(test_symlink_and_writable_parents_are_rejected() == 0);
	assert(test_only_existing_symlink_may_be_replaced() == 0);
	assert(test_lookup_errors_are_preserved() == 0);
	puts("Transmission storage-link publication policy: PASS");
	return 0;
}
