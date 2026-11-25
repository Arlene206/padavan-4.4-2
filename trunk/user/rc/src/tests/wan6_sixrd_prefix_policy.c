#include <assert.h>
#include <stdio.h>

#include "wan6_sixrd_prefix_policy.h"

static void
test_valid_mapping_boundaries(void)
{
	assert(wan6_sixrd_prefix_mapping_is_valid(0, 96));
	assert(wan6_sixrd_prefix_mapping_is_valid(24, 120));
	assert(wan6_sixrd_prefix_mapping_is_valid(32, 128));
	assert(wan6_sixrd_prefix_mapping_is_valid(32, 0));
}

static void
test_rejects_oversized_derived_prefix(void)
{
	assert(!wan6_sixrd_prefix_mapping_is_valid(0, 97));
	assert(!wan6_sixrd_prefix_mapping_is_valid(31, 128));
	assert(!wan6_sixrd_prefix_mapping_is_valid(16, 113));
}

static void
test_rejects_out_of_range_lengths(void)
{
	assert(!wan6_sixrd_prefix_mapping_is_valid(-1, 96));
	assert(!wan6_sixrd_prefix_mapping_is_valid(33, 96));
	assert(!wan6_sixrd_prefix_mapping_is_valid(24, -1));
	assert(!wan6_sixrd_prefix_mapping_is_valid(24, 129));
}

int
main(void)
{
	test_valid_mapping_boundaries();
	test_rejects_oversized_derived_prefix();
	test_rejects_out_of_range_lengths();
	puts("WAN DHCPv4 6rd prefix bounds regression: PASS");
	return 0;
}
