#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/iapp_ioctl_ifname_policy.h"

static void
test_valid_two_interface_slots(void)
{
	char names[2][16] = { "rai0", "ra0" };
	const char *selected;

	selected = iapp_ioctl_ifname_select((const char *)names, 2, sizeof(names[0]),
						 2, 0, 12, "ra0");
	assert(strcmp(selected, "rai0") == 0);
	selected = iapp_ioctl_ifname_select((const char *)names, 2, sizeof(names[0]),
						 2, 1, 12, "fallback0");
	assert(strcmp(selected, "ra0") == 0);
}

static void
test_out_of_range_and_unconfigured_indices_fall_back(void)
{
	char names[2][16] = { "rai0", "ra0" };
	const char *fallback = "fallback0";

	assert(strcmp(iapp_ioctl_ifname_select((const char *)names, 2, 16, 2, 2, 12, fallback), fallback) == 0);
	assert(strcmp(iapp_ioctl_ifname_select((const char *)names, 2, 16, 2, 255, 12, fallback), fallback) == 0);
	assert(strcmp(iapp_ioctl_ifname_select((const char *)names, 2, 16, 1, 1, 12, fallback), fallback) == 0);
	assert(strcmp(iapp_ioctl_ifname_select((const char *)names, 2, 16, 0, 0, 12, fallback), fallback) == 0);
	assert(strcmp(iapp_ioctl_ifname_select((const char *)names, 2, 16, -1, 0, 12, fallback), fallback) == 0);
	assert(strcmp(iapp_ioctl_ifname_select((const char *)names, 2, 16, 2, -1, 12, fallback), fallback) == 0);
}

static void
test_malformed_slots_and_names_fall_back(void)
{
	char unterminated[2][16];
	char empty[2][16] = { "", "ra0" };
	char too_long[2][32] = { "interface-name-too-long", "ra0" };
	const char *fallback = "fallback0";

	memset(unterminated, 'x', sizeof(unterminated));
	assert(strcmp(iapp_ioctl_ifname_select((const char *)unterminated, 2, 16, 2, 0, 12, fallback), fallback) == 0);
	assert(strcmp(iapp_ioctl_ifname_select((const char *)empty, 2, 16, 2, 0, 12, fallback), fallback) == 0);
	assert(strcmp(iapp_ioctl_ifname_select((const char *)too_long, 2, sizeof(too_long[0]), 2, 0, 12, fallback), fallback) == 0);
	assert(strcmp(iapp_ioctl_ifname_select(NULL, 2, 16, 2, 0, 12, fallback), fallback) == 0);
	assert(strcmp(iapp_ioctl_ifname_select((const char *)empty, 2, 0, 2, 0, 12, fallback), fallback) == 0);
}

static void
test_parser_rejects_count_overflow_and_invalid_names(void)
{
	assert(iapp_ioctl_ifname_arg_valid(0, 2, 4, 16));
	assert(iapp_ifname_arg_valid(15, 16));
	assert(!iapp_ifname_arg_valid(0, 16));
	assert(!iapp_ifname_arg_valid(16, 16));
	assert(iapp_ioctl_ifname_arg_valid(1, 2, 3, 16));
	assert(!iapp_ioctl_ifname_arg_valid(2, 2, 4, 16));
	assert(!iapp_ioctl_ifname_arg_valid(-1, 2, 4, 16));
	assert(!iapp_ioctl_ifname_arg_valid(0, 2, 0, 16));
	assert(!iapp_ioctl_ifname_arg_valid(0, 2, 16, 16));
}

int
main(void)
{
	test_valid_two_interface_slots();
	test_out_of_range_and_unconfigured_indices_fall_back();
	test_malformed_slots_and_names_fall_back();
	test_parser_rejects_count_overflow_and_invalid_names();
	puts("IAPP ioctl interface index and parser bounds policy: PASS");
	return 0;
}
