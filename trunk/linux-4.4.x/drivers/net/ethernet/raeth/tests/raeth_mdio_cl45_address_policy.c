#include <stdio.h>

#include "../raeth_mdio_cl45_address_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_valid_clause45_addresses(void)
{
	CHECK(raeth_mdio_cl45_address_status(0U, 0U, 0U) == 0);
	CHECK(raeth_mdio_cl45_address_status(31U, 31U, 0xffffU) == 0);
	CHECK(raeth_mdio_cl45_address_status(1U, 30U, 0x8000U) == 0);
	return 0;
}

static int
test_port_and_device_widths_are_enforced(void)
{
	CHECK(raeth_mdio_cl45_address_status(32U, 0U, 0U) == -EINVAL);
	CHECK(raeth_mdio_cl45_address_status(0xffffffffU, 0U, 0U) == -EINVAL);
	CHECK(raeth_mdio_cl45_address_status(0U, 32U, 0U) == -EINVAL);
	CHECK(raeth_mdio_cl45_address_status(0U, 0xffffffffU, 0U) == -EINVAL);
	return 0;
}

static int
test_register_address_is_limited_to_sixteen_bits(void)
{
	CHECK(raeth_mdio_cl45_address_status(0U, 0U, 0x10000U) == -EINVAL);
	CHECK(raeth_mdio_cl45_address_status(31U, 31U, 0xffffffffU) == -EINVAL);
	return 0;
}

int
main(void)
{
	CHECK(test_valid_clause45_addresses() == 0);
	CHECK(test_port_and_device_widths_are_enforced() == 0);
	CHECK(test_register_address_is_limited_to_sixteen_bits() == 0);
	puts("RAETH Clause 45 ioctl address bounds: PASS");
	return 0;
}
