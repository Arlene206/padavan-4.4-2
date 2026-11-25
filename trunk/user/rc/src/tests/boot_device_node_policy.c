#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>

#include "../boot_device_node_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

int
main(void)
{
	struct stat st;
	dev_t expected = makedev(220, 0);

	memset(&st, 0, sizeof(st));
	st.st_mode = S_IFCHR | RC_BOOT_DEVICE_NODE_MODE;
	st.st_uid = 0;
	st.st_gid = 0;
	st.st_nlink = 1;
	st.st_rdev = expected;
	CHECK(rc_boot_device_node_is_secure(&st, expected));

	/* A stricter root-only device remains usable by root services. */
	st.st_mode = S_IFCHR | 0600;
	CHECK(rc_boot_device_node_is_secure(&st, expected));

	st.st_mode = S_IFREG | RC_BOOT_DEVICE_NODE_MODE;
	CHECK(!rc_boot_device_node_is_secure(&st, expected));
	st.st_mode = S_IFLNK | RC_BOOT_DEVICE_NODE_MODE;
	CHECK(!rc_boot_device_node_is_secure(&st, expected));

	st.st_mode = S_IFCHR | RC_BOOT_DEVICE_NODE_MODE;
	st.st_rdev = makedev(220, 1);
	CHECK(!rc_boot_device_node_is_secure(&st, expected));
	st.st_rdev = expected;

	st.st_uid = 1000;
	CHECK(!rc_boot_device_node_is_secure(&st, expected));
	st.st_uid = 0;
	st.st_gid = 1000;
	CHECK(!rc_boot_device_node_is_secure(&st, expected));
	st.st_gid = 0;

	st.st_nlink = 2;
	CHECK(!rc_boot_device_node_is_secure(&st, expected));
	st.st_nlink = 1;

	st.st_mode = S_IFCHR | 0400;
	CHECK(!rc_boot_device_node_is_secure(&st, expected));
	st.st_mode = S_IFCHR | 0766;
	CHECK(!rc_boot_device_node_is_secure(&st, expected));
	st.st_mode = S_IFCHR | 04666;
	CHECK(!rc_boot_device_node_is_secure(&st, expected));

	CHECK(!rc_boot_device_node_is_secure(NULL, expected));

	puts("Optional boot device-node identity and permission policy: PASS");
	return 0;
}
