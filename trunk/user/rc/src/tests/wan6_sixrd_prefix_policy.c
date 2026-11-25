#include <assert.h>
#include <stdio.h>
#include <string.h>

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

static void
test_prefix_host_bits_are_ignored(void)
{
	unsigned char prefix[16], saved[16];
	unsigned int i;

	memset(prefix, 0xff, sizeof(prefix));
	assert(wan6_sixrd_prefix_clear_host_bits(prefix, 33));
	for (i = 0; i < 4; ++i)
		assert(prefix[i] == 0xff);
	assert(prefix[4] == 0x80);
	for (i = 5; i < sizeof(prefix); ++i)
		assert(prefix[i] == 0);

	memset(prefix, 0xff, sizeof(prefix));
	assert(wan6_sixrd_prefix_clear_host_bits(prefix, 0));
	for (i = 0; i < sizeof(prefix); ++i)
		assert(prefix[i] == 0);

	memset(prefix, 0xa5, sizeof(prefix));
	assert(wan6_sixrd_prefix_clear_host_bits(prefix, 128));
	for (i = 0; i < sizeof(prefix); ++i)
		assert(prefix[i] == 0xa5);

	memset(prefix, 0x5a, sizeof(prefix));
	memcpy(saved, prefix, sizeof(prefix));
	assert(!wan6_sixrd_prefix_clear_host_bits(prefix, -1));
	assert(!wan6_sixrd_prefix_clear_host_bits(prefix, 129));
	assert(!wan6_sixrd_prefix_clear_host_bits(NULL, 64));
	assert(memcmp(prefix, saved, sizeof(prefix)) == 0);
}

int
main(void)
{
	test_valid_mapping_boundaries();
	test_rejects_oversized_derived_prefix();
	test_rejects_out_of_range_lengths();
	test_prefix_host_bits_are_ignored();
	puts("WAN DHCPv4 6rd prefix bounds/host-bit regression: PASS");
	return 0;
}
