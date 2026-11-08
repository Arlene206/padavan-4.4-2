#include <assert.h>
#include <stdio.h>

#define main mii_mgr_cl45_program_main
#include "../mii_mgr_cl45.c"
#undef main

static void test_argument_order(void)
{
	char *device_first[] = {
		"mii_mgr_cl45", "-g", "-p", "3", "-d", "0x5", "-r", "0x4", NULL
	};
	char *register_first[] = {
		"mii_mgr_cl45", "-g", "-p", "3", "-r", "0x4", "-d", "0x5", NULL
	};
	struct ra_mii_ioctl_data first;
	struct ra_mii_ioctl_data second;
	int first_method;
	int second_method;

	assert(parse_arguments(8, device_first, &first, &first_method) == 0);
	assert(parse_arguments(8, register_first, &second, &second_method) == 0);
	assert(first_method == RAETH_MII_READ_CL45);
	assert(second_method == RAETH_MII_READ_CL45);
	assert(first.port_num == 3 && second.port_num == 3);
	assert(first.dev_addr == 5 && second.dev_addr == 5);
	assert(first.reg_addr == 4 && second.reg_addr == 4);
}

static void test_write_and_bounds(void)
{
	char *write_ok[] = {
		"mii_mgr_cl45", "-s", "-p", "4", "-d", "0x6", "-r", "0x1",
		"-v", "0xff11", NULL
	};
	char *port_bad[] = {
		"mii_mgr_cl45", "-g", "-p", "0x20", "-d", "5", "-r", "4", NULL
	};
	char *device_bad[] = {
		"mii_mgr_cl45", "-g", "-p", "3", "-d", "0x20", "-r", "4", NULL
	};
	char *register_bad[] = {
		"mii_mgr_cl45", "-g", "-p", "3", "-d", "5", "-r", "0x10000", NULL
	};
	char *value_bad[] = {
		"mii_mgr_cl45", "-s", "-p", "3", "-d", "5", "-r", "4",
		"-v", "0x10000", NULL
	};
	char *negative[] = {
		"mii_mgr_cl45", "-g", "-p", "-1", "-d", "5", "-r", "4", NULL
	};
	char *trailing_junk[] = {
		"mii_mgr_cl45", "-g", "-p", "3x", "-d", "5", "-r", "4", NULL
	};
	struct ra_mii_ioctl_data mii;
	int method;

	assert(parse_arguments(10, write_ok, &mii, &method) == 0);
	assert(method == RAETH_MII_WRITE_CL45);
	assert(mii.port_num == 4 && mii.dev_addr == 6);
	assert(mii.reg_addr == 1 && mii.val_in == 0xff11);
	assert(parse_arguments(8, port_bad, &mii, &method) < 0);
	assert(parse_arguments(8, device_bad, &mii, &method) < 0);
	assert(parse_arguments(8, register_bad, &mii, &method) < 0);
	assert(parse_arguments(10, value_bad, &mii, &method) < 0);
	assert(parse_arguments(8, negative, &mii, &method) < 0);
	assert(parse_arguments(8, trailing_junk, &mii, &method) < 0);
}

static void test_required_and_conflicting_options(void)
{
	char *missing_register[] = {
		"mii_mgr_cl45", "-g", "-p", "3", "-d", "5", NULL
	};
	char *missing_write_value[] = {
		"mii_mgr_cl45", "-s", "-p", "3", "-d", "5", "-r", "4", NULL
	};
	char *conflicting_method[] = {
		"mii_mgr_cl45", "-g", "-s", "-p", "3", "-d", "5", "-r", "4",
		"-v", "1", NULL
	};
	struct ra_mii_ioctl_data mii;
	int method;

	assert(parse_arguments(6, missing_register, &mii, &method) < 0);
	assert(parse_arguments(8, missing_write_value, &mii, &method) < 0);
	assert(parse_arguments(11, conflicting_method, &mii, &method) < 0);
}

int main(void)
{
	test_argument_order();
	test_write_and_bounds();
	test_required_and_conflicting_options();
	puts("mii_mgr_cl45 argument parsing, ordering and range checks: PASS");
	return 0;
}
