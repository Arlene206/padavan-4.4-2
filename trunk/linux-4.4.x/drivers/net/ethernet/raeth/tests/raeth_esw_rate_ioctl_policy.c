#include <stdio.h>

#include "../raeth_esw_rate_ioctl_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_valid_rate_requests(void)
{
	CHECK(raeth_esw_rate_ioctl_status(0U, 0U, 0U) == 0);
	CHECK(raeth_esw_rate_ioctl_status(1U, 0U, 0U) == 0);
	CHECK(raeth_esw_rate_ioctl_status(1U, 4U, 100U) == 0);
	CHECK(raeth_esw_rate_ioctl_status(
		      1U, 4U, RAETH_ESW_RATE_MAX_BANDWIDTH_MBPS) == 0);
	CHECK(raeth_esw_rate_ioctl_status(0U, 4U, 0U) == 0);
	return 0;
}

static int
test_invalid_port_and_control_values_fail_closed(void)
{
	CHECK(raeth_esw_rate_ioctl_status(2U, 0U, 0U) == -EINVAL);
	CHECK(raeth_esw_rate_ioctl_status(0xffffffffU, 0U, 0U) == -EINVAL);
	CHECK(raeth_esw_rate_ioctl_status(1U, 5U, 0U) == -EINVAL);
	CHECK(raeth_esw_rate_ioctl_status(1U, 0xffffffffU, 0U) == -EINVAL);
	return 0;
}

static int
test_bandwidth_cannot_overflow_the_eight_bit_rate_field(void)
{
	CHECK(raeth_esw_rate_ioctl_status(
		      1U, 0U, RAETH_ESW_RATE_MAX_BANDWIDTH_MBPS + 1U) == -EINVAL);
	CHECK(raeth_esw_rate_ioctl_status(1U, 4U, 0xffffffffU) == -EINVAL);
	CHECK(raeth_esw_rate_ioctl_status(0U, 0U, 0xffffffffU) == -EINVAL);
	return 0;
}

int
main(void)
{
	CHECK(test_valid_rate_requests() == 0);
	CHECK(test_invalid_port_and_control_values_fail_closed() == 0);
	CHECK(test_bandwidth_cannot_overflow_the_eight_bit_rate_field() == 0);
	puts("RAETH ESW rate ioctl port and magnitude bounds: PASS");
	return 0;
}
