#include <stdio.h>

#include "../raeth_lan_ip_ioctl_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

int
main(void)
{
	static const char normal[] = "192.0.2.1";
	static const char maximum[] = "255.255.255.255";
	static const char empty[] = "";
	static const char unterminated[16] = {
		'2', '5', '5', '.', '2', '5', '5', '.', '2', '5', '5', '.', '2', '5', '5', '.'
	};

	CHECK(raeth_lan_ip_ioctl_status(normal, sizeof(normal)) == 0);
	CHECK(raeth_lan_ip_ioctl_status(maximum, sizeof(maximum)) == 0);
	CHECK(raeth_lan_ip_ioctl_status(empty, sizeof(empty)) == -EINVAL);
	CHECK(raeth_lan_ip_ioctl_status(unterminated, sizeof(unterminated)) == -EINVAL);
	CHECK(raeth_lan_ip_ioctl_status(NULL, sizeof(normal)) == -EINVAL);
	CHECK(raeth_lan_ip_ioctl_status(normal, 0) == -EINVAL);

	puts("RAETH SET_LAN_IP bounded string policy: PASS");
	return 0;
}
