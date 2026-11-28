#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include "switch_device_node_policy.h"

static void
test_accepts_expected_root_private_character_device(void)
{
	struct stat st;
	memset(&st, 0, sizeof(st));
	st.st_mode = S_IFCHR | 0600;
	st.st_uid = 0;
	st.st_gid = 0;
	st.st_nlink = 1;
	st.st_rdev = makedev(234, 7);
	assert(rc_switch_device_node_is_secure(&st, S_IFCHR | 0600,
	                                       makedev(234, 7)));
}

static void
test_rejects_wrong_type_and_device_number(void)
{
	struct stat st;
	memset(&st, 0, sizeof(st));
	st.st_mode = S_IFREG | 0600;
	st.st_uid = 0;
	st.st_gid = 0;
	st.st_nlink = 1;
	st.st_rdev = makedev(234, 7);
	assert(!rc_switch_device_node_is_secure(&st, S_IFCHR | 0600,
	                                        makedev(234, 7)));
	st.st_mode = S_IFCHR | 0600;
	assert(!rc_switch_device_node_is_secure(&st, S_IFCHR | 0600,
	                                        makedev(234, 8)));
}

static void
test_rejects_symlink_permissions_and_ownership(void)
{
	struct stat st;
	memset(&st, 0, sizeof(st));
	st.st_mode = S_IFLNK | 0600;
	st.st_uid = 0;
	st.st_gid = 0;
	st.st_nlink = 1;
	st.st_rdev = makedev(234, 7);
	assert(!rc_switch_device_node_is_secure(&st, S_IFCHR | 0600,
	                                        makedev(234, 7)));
	st.st_mode = S_IFCHR | 0666;
	assert(!rc_switch_device_node_is_secure(&st, S_IFCHR | 0600,
	                                        makedev(234, 7)));
	st.st_mode = S_IFCHR | 0600;
	st.st_uid = 1000;
	assert(!rc_switch_device_node_is_secure(&st, S_IFCHR | 0600,
	                                        makedev(234, 7)));
	st.st_uid = 0;
	st.st_gid = 1000;
	assert(!rc_switch_device_node_is_secure(&st, S_IFCHR | 0600,
	                                        makedev(234, 7)));
	st.st_gid = 0;
	st.st_nlink = 2;
	assert(!rc_switch_device_node_is_secure(&st, S_IFCHR | 0600,
	                                        makedev(234, 7)));
}

int
main(void)
{
	test_accepts_expected_root_private_character_device();
	test_rejects_wrong_type_and_device_number();
	test_rejects_symlink_permissions_and_ownership();
	return 0;
}
