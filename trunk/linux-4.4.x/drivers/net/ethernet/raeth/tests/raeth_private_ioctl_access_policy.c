#include <stdio.h>

#include "../raeth_private_ioctl_access_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

int
main(void)
{
	CHECK(raeth_private_ioctl_access_status(1) == 0);
	CHECK(raeth_private_ioctl_access_status(0) == -EPERM);
	CHECK(raeth_private_ioctl_access_status(2) == -EPERM);
	CHECK(raeth_private_ioctl_access_status(-1) == -EPERM);

	puts("RAETH private ioctl CAP_NET_ADMIN authorization policy: PASS");
	return 0;
}
