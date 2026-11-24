#include <errno.h>
#include <limits.h>
#include <stdio.h>

#include "wan_src_phy_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_external_switch_ports_are_accepted(void)
{
	int port;
	int configured;
	for (configured = 0; configured <= WAN_SRC_PHY_MAX_EXTERNAL; ++configured) {
		port = -1;
		CHECK(wan_src_phy_resolve(configured, &port) == 0);
		CHECK(port == configured);
	}
	return 0;
}

static int
test_negative_legacy_values_keep_wan_default(void)
{
	int port = -1;
	CHECK(wan_src_phy_resolve(-1, &port) == 0);
	CHECK(port == WAN_SRC_PHY_DEFAULT);
	port = -1;
	CHECK(wan_src_phy_resolve(INT_MIN, &port) == 0);
	CHECK(port == WAN_SRC_PHY_DEFAULT);
	return 0;
}

static int
test_unknown_and_cpu_ports_fail_without_publishing(void)
{
	static const int invalid[] = { 6, 13, 14, 15, INT_MAX };
	int port;
	unsigned int i;
	for (i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
		port = 77;
		CHECK(wan_src_phy_resolve(invalid[i], &port) == ERANGE);
		CHECK(port == 77);
	}
	CHECK(wan_src_phy_resolve(1, NULL) == EINVAL);
	return 0;
}

int
main(void)
{
	CHECK(test_external_switch_ports_are_accepted() == 0);
	CHECK(test_negative_legacy_values_keep_wan_default() == 0);
	CHECK(test_unknown_and_cpu_ports_fail_without_publishing() == 0);
	puts("WAN source PHY selector range and fail-closed policy: PASS");
	return 0;
}
