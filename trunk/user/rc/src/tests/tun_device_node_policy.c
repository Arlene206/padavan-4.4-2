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
	struct stat st, directory;
	dev_t expected = makedev(TUN_DEVICE_NODE_MAJOR, TUN_DEVICE_NODE_MINOR);

	memset(&directory, 0, sizeof(directory));
	directory.st_mode = S_IFDIR | 0755;
	directory.st_uid = 0;
	directory.st_gid = 0;
	CHECK(tun_device_directory_is_secure(&directory));

	/* The daemon may run as nobody: execute access is required, listing is not. */
	directory.st_mode = S_IFDIR | 0711;
	CHECK(tun_device_directory_is_secure(&directory));

	directory.st_mode = S_IFDIR | 0700;
	CHECK(!tun_device_directory_is_secure(&directory));
	directory.st_mode = S_IFDIR | 0750;
	CHECK(!tun_device_directory_is_secure(&directory));
	directory.st_mode = S_IFDIR | 0775;
	CHECK(!tun_device_directory_is_secure(&directory));
	directory.st_mode = S_IFDIR | 02755;
	CHECK(!tun_device_directory_is_secure(&directory));
	directory.st_mode = S_IFDIR | 01755;
	CHECK(!tun_device_directory_is_secure(&directory));
	directory.st_mode = S_IFREG | 0755;
	CHECK(!tun_device_directory_is_secure(&directory));
	directory.st_mode = S_IFLNK | 0755;
	CHECK(!tun_device_directory_is_secure(&directory));

	directory.st_mode = S_IFDIR | 0755;
	directory.st_uid = 1000;
	CHECK(!tun_device_directory_is_secure(&directory));
	directory.st_uid = 0;
	directory.st_gid = 1000;
	CHECK(!tun_device_directory_is_secure(&directory));
	CHECK(!tun_device_directory_is_secure(NULL));

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

	puts("OpenVPN TUN directory traversal and device-node identity policy: PASS");
	return 0;
}
