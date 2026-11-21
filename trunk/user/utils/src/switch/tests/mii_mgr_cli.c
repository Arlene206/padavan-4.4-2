#include <assert.h>
#include <stdio.h>
#include <string.h>

#define main mii_mgr_program_main
#include "../mii_mgr.c"
#undef main

static void test_phy_and_register_order(void)
{
	char *phy_first[] = {
		"mii_mgr", "-g", "-p", "3", "-r", "4", NULL
	};
	char *reg_first[] = {
		"mii_mgr", "-g", "-r", "4", "-p", "3", NULL
	};
#if defined(CONFIG_RALINK_MT7621) || defined(CONFIG_MT7530_GSW)
	char *switch_reg_phy_first[] = {
		"mii_mgr", "-g", "-p", "31", "-r", "0x1234", NULL
	};
	char *switch_reg_first[] = {
		"mii_mgr", "-g", "-r", "0x1234", "-p", "31", NULL
	};
#else
	char *switch_reg_phy_first[] = {
		"mii_mgr", "-g", "-p", "31", "-r", "4", NULL
	};
	char *switch_reg_first[] = {
		"mii_mgr", "-g", "-r", "4", "-p", "31", NULL
	};
#endif
	struct mii_mgr_options a, b;

	assert(parse_arguments(6, phy_first, &a) == 0);
	assert(parse_arguments(6, reg_first, &b) == 0);
	assert(a.method == RAETH_MII_READ && b.method == RAETH_MII_READ);
	assert(a.mii.phy_id == 3 && b.mii.phy_id == 3);
	assert(a.mii.reg_num == 4 && b.mii.reg_num == 4);

	assert(parse_arguments(6, switch_reg_phy_first, &a) == 0);
	assert(parse_arguments(6, switch_reg_first, &b) == 0);
	assert(a.mii.phy_id == 31 && b.mii.phy_id == 31);
#if defined(CONFIG_RALINK_MT7621) || defined(CONFIG_MT7530_GSW)
	assert(a.mii.reg_num == 0x1234 && b.mii.reg_num == 0x1234);
#else
	assert(a.mii.reg_num == 4 && b.mii.reg_num == 4);
#endif
}

static void test_write_and_ranges(void)
{
	char *write_ok[] = {
		"mii_mgr", "-s", "-p", "4", "-r", "1", "-v", "0xff11", NULL
	};
	char *phy_bad[] = {
		"mii_mgr", "-g", "-p", "32", "-r", "1", NULL
	};
	char *reg_bad[] = {
		"mii_mgr", "-g", "-p", "3", "-r", "32", NULL
	};
	char *switch_reg_bad[] = {
		"mii_mgr", "-g", "-p", "31", "-r", "0x10000", NULL
	};
	char *value_bad[] = {
		"mii_mgr", "-s", "-p", "3", "-r", "1", "-v", "0x10000", NULL
	};
	char *negative[] = {
		"mii_mgr", "-g", "-p", "-1", "-r", "1", NULL
	};
	char *trailing[] = {
		"mii_mgr", "-g", "-p", "3x", "-r", "1", NULL
	};
	char *wrong_base[] = {
		"mii_mgr", "-g", "-p", "3", "-r", "0x4", NULL
	};
	char *overflow[] = {
		"mii_mgr", "-g", "-p", "999999999999999999999999999", "-r", "1", NULL
	};
	struct mii_mgr_options parsed;

	assert(parse_arguments(8, write_ok, &parsed) == 0);
	assert(parsed.method == RAETH_MII_WRITE);
	assert(parsed.mii.phy_id == 4 && parsed.mii.reg_num == 1);
	assert(parsed.mii.val_in == 0xff11);
	assert(parse_arguments(6, phy_bad, &parsed) < 0);
	assert(parse_arguments(6, reg_bad, &parsed) < 0);
	assert(parse_arguments(6, switch_reg_bad, &parsed) < 0);
	assert(parse_arguments(8, value_bad, &parsed) < 0);
	assert(parse_arguments(6, negative, &parsed) < 0);
	assert(parse_arguments(6, trailing, &parsed) < 0);
	assert(parse_arguments(6, wrong_base, &parsed) < 0);
	assert(parse_arguments(6, overflow, &parsed) < 0);
}

#if defined(CONFIG_RALINK_MT7628)
static void test_page_select(void)
{
	char *page_only[] = {
		"mii_mgr", "-L", "0x12", "-p", "3", NULL
	};
	char *page_and_read[] = {
		"mii_mgr", "-G", "0x2", "-p", "3", "-g", "-r", "4", NULL
	};
	struct mii_mgr_options parsed;

	assert(parse_arguments(5, page_only, &parsed) == 0);
	assert(parsed.page_select == 1 && parsed.page_mii.phy_id == 3);
	assert(parsed.page_mii.reg_num == 31);
	assert(parsed.page_mii.val_in == ((0x12U << 12) | 0x8000U));
	assert(parse_arguments(8, page_and_read, &parsed) == 0);
	assert(parsed.page_select == 1 && parsed.method == RAETH_MII_READ);
	assert(parsed.mii.reg_num == 4 && parsed.page_mii.val_in == (0x2U << 12));
}
#endif

static void test_modes_and_required_options(void)
{
	char *missing_reg[] = { "mii_mgr", "-g", "-p", "3", NULL };
	char *missing_phy[] = { "mii_mgr", "-g", "-r", "1", NULL };
	char *missing_value[] = {
		"mii_mgr", "-s", "-p", "3", "-r", "1", NULL
	};
	char *read_value[] = {
		"mii_mgr", "-g", "-p", "3", "-r", "1", "-v", "2", NULL
	};
	char *both_modes[] = {
		"mii_mgr", "-g", "-s", "-p", "3", "-r", "1", "-v", "2", NULL
	};
	char *extra[] = { "mii_mgr", "-g", "-p", "3", "-r", "1", "extra", NULL };
	char *duplicate[] = {
		"mii_mgr", "-g", "-p", "3", "-p", "4", "-r", "1", NULL
	};
	char *legacy_t[] = { "mii_mgr", "-g", "-t", "-p", "3", "-r", "1", NULL };
	struct mii_mgr_options parsed;

	assert(parse_arguments(4, missing_reg, &parsed) < 0);
	assert(parse_arguments(4, missing_phy, &parsed) < 0);
	assert(parse_arguments(6, missing_value, &parsed) < 0);
	assert(parse_arguments(8, read_value, &parsed) < 0);
	assert(parse_arguments(9, both_modes, &parsed) < 0);
	assert(parse_arguments(7, extra, &parsed) < 0);
	assert(parse_arguments(8, duplicate, &parsed) < 0);
	assert(parse_arguments(7, legacy_t, &parsed) == 0);
}

int main(void)
{
	test_phy_and_register_order();
	test_write_and_ranges();
#if defined(CONFIG_RALINK_MT7628)
	test_page_select();
#endif
	test_modes_and_required_options();
	puts("mii_mgr argument ordering, validation and mode checks: PASS");
	return 0;
}
