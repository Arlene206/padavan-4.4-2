#include <stdio.h>

#include "../mii_mgr_phy_address_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_five_bit_clause22_register_boundaries(void)
{
	CHECK(mii_mgr_phy_register_status(0U) == 0);
	CHECK(mii_mgr_phy_register_status(1U) == 0);
	CHECK(mii_mgr_phy_register_status(31U) == 0);
	CHECK(mii_mgr_phy_register_status(32U) == -EINVAL);
	CHECK(mii_mgr_phy_register_status(0xffffffffU) == -EINVAL);
	return 0;
}

static int
test_sixteen_bit_clause22_write_data_boundaries(void)
{
	CHECK(mii_mgr_phy_write_data_status(0U) == 0);
	CHECK(mii_mgr_phy_write_data_status(1U) == 0);
	CHECK(mii_mgr_phy_write_data_status(0xffffU) == 0);
	CHECK(mii_mgr_phy_write_data_status(0x10000U) == -EINVAL);
	CHECK(mii_mgr_phy_write_data_status(0xffffffffU) == -EINVAL);
	return 0;
}

static int
test_five_bit_phy_address_boundaries(void)
{
	CHECK(mii_mgr_phy_address_status(0U) == 0);
	CHECK(mii_mgr_phy_address_status(1U) == 0);
	CHECK(mii_mgr_phy_address_status(31U) == 0);
	CHECK(mii_mgr_phy_address_status(32U) == -EINVAL);
	CHECK(mii_mgr_phy_address_status(0xffffffffU) == -EINVAL);
	return 0;
}

int
main(void)
{
	CHECK(test_five_bit_clause22_register_boundaries() == 0);
	CHECK(test_sixteen_bit_clause22_write_data_boundaries() == 0);
	CHECK(test_five_bit_phy_address_boundaries() == 0);
	puts("RAETH Clause 22 PHY address, register and write-data bounds: PASS");
	return 0;
}
