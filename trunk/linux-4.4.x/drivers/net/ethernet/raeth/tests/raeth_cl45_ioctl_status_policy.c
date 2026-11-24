#include <stdio.h>

#include "../raeth_cl45_ioctl_status_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

int
main(void)
{
	CHECK(raeth_cl45_ioctl_status(1U) == 0);
	CHECK(raeth_cl45_ioctl_status(0U) == -EIO);
	CHECK(raeth_cl45_ioctl_status(2U) == -EIO);

	puts("RAETH Clause 45 ioctl error propagation policy: PASS");
	return 0;
}
