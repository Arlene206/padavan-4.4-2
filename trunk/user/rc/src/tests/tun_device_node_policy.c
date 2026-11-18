#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>

#include "../tun_device_node_policy.h"

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
	dev_t expected = makedev(TUN_DEVICE_NODE_MAJOR, TUN_DEVICE_NODE_MINOR);

	memset(&st, 0, sizeof(st));
	st.st_mode = S_IFCHR | TUN_DEVICE_NODE_MODE;
	st.st_uid = 0;
	st.st_gid = 0;
	st.st_nlink = 1;
	st.st_rdev = expected;
	CHECK(tun_device_node_is_secure(&st, expected));

	/* A root-only device is stricter than the default 0666 and remains valid. */
	st.st_mode = S_IFCHR | 0600;
	CHECK(tun_device_node_is_secure(&st, expected));

	st.st_mode = S_IFREG | TUN_DEVICE_NODE_MODE;
	CHECK(!tun_device_node_is_secure(&st, expected));
	st.st_mode = S_IFLNK | TUN_DEVICE_NODE_MODE;
	CHECK(!tun_device_node_is_secure(&st, expected));

	st.st_mode = S_IFCHR | TUN_DEVICE_NODE_MODE;
	st.st_rdev = makedev(TUN_DEVICE_NODE_MAJOR, TUN_DEVICE_NODE_MINOR + 1);
	CHECK(!tun_device_node_is_secure(&st, expected));
	st.st_rdev = expected;

	st.st_uid = 1000;
	CHECK(!tun_device_node_is_secure(&st, expected));
	st.st_uid = 0;
	st.st_gid = 1000;
	CHECK(!tun_device_node_is_secure(&st, expected));
	st.st_gid = 0;

	st.st_nlink = 2;
	CHECK(!tun_device_node_is_secure(&st, expected));
	st.st_nlink = 1;

	st.st_mode = S_IFCHR | 0400;
	CHECK(!tun_device_node_is_secure(&st, expected));
	st.st_mode = S_IFCHR | 0766;
	CHECK(!tun_device_node_is_secure(&st, expected));
	st.st_mode = S_IFCHR | 04666;
	CHECK(!tun_device_node_is_secure(&st, expected));

	CHECK(!tun_device_node_is_secure(NULL, expected));

	puts("OpenVPN TUN device-node type, identity, ownership and mode policy: PASS");
	return 0;
}
