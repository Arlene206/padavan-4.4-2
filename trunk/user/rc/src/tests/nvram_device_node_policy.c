#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>

#include "../nvram_device_node_policy.h"

static int
expect_policy(const struct stat *st, dev_t device, int expected,
	      const char *label)
{
	int result = nvram_device_node_is_secure(st, device);

	if (result != expected) {
		fprintf(stderr, "FAIL: %s: got %d expected %d\n",
			label, result, expected);
		return 0;
	}
	return 1;
}

int
main(void)
{
	struct stat st;
	dev_t device = makedev(NVRAM_DEVICE_NODE_MAJOR,
			       NVRAM_DEVICE_NODE_MINOR);
	int passed = 1;

	memset(&st, 0, sizeof(st));
	st.st_mode = S_IFCHR | NVRAM_DEVICE_NODE_MODE;
	st.st_uid = 0;
	st.st_gid = 0;
	st.st_nlink = 1;
	st.st_rdev = device;
	passed &= expect_policy(&st, device, 1, "root:root character device 0600");

	st.st_mode = S_IFCHR | 0666;
	passed &= expect_policy(&st, device, 0, "world-writable device");
	st.st_mode = S_IFCHR | 0620;
	passed &= expect_policy(&st, device, 0, "group-writable device");
	st.st_mode = S_IFCHR | 0600;
	st.st_uid = 1000;
	passed &= expect_policy(&st, device, 0, "non-root owner");
	st.st_uid = 0;
	st.st_gid = 1000;
	passed &= expect_policy(&st, device, 0, "non-root group");
	st.st_gid = 0;
	st.st_rdev = makedev(NVRAM_DEVICE_NODE_MAJOR + 1,
			     NVRAM_DEVICE_NODE_MINOR);
	passed &= expect_policy(&st, device, 0, "wrong device number");
	st.st_rdev = device;
	st.st_mode = S_IFREG | 0600;
	passed &= expect_policy(&st, device, 0, "regular-file substitution");
	st.st_mode = S_IFLNK | 0777;
	passed &= expect_policy(&st, device, 0, "symlink substitution");
	st.st_mode = S_IFCHR | 0600;
	st.st_nlink = 2;
	passed &= expect_policy(&st, device, 0, "unexpected hard link");

	if (!passed)
		return 1;

	printf("NVRAM device node root ownership, mode and identity policy regression: PASS\n");
	return 0;
}
