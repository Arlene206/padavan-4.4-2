#include <stdio.h>

#include "../ioctl_access_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

int
main(void)
{
	CHECK(mtk_esw_ioctl_access_status(1) == 0);
	CHECK(mtk_esw_ioctl_access_status(0) == -EPERM);
	/* Only the exact positive result from capable() grants access. */
	CHECK(mtk_esw_ioctl_access_status(2) == -EPERM);
	CHECK(mtk_esw_ioctl_access_status(-1) == -EPERM);

	puts("MTK ESW ioctl CAP_NET_ADMIN authorization policy: PASS");
	return 0;
}
