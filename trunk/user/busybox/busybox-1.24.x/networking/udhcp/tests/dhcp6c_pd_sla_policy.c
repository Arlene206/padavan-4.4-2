#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "dhcp6c_pd_sla_policy.h"

static void
test_partial_sla_field_is_network_bit_aligned(void)
{
	unsigned char address[16];

	memset(address, 0, sizeof(address));
	assert(dhcp6c_pd_sla_insert(address, 56, 4, 1U) == 0);
	assert(address[7] == 0x10);

	memset(address, 0, sizeof(address));
	assert(dhcp6c_pd_sla_insert(address, 56, 4, 15U) == 0);
	assert(address[7] == 0xf0);

	memset(address, 0, sizeof(address));
	assert(dhcp6c_pd_sla_insert(address, 48, 12, 0xabcU) == 0);
	assert(address[6] == 0xab);
	assert(address[7] == 0xc0);

	/* A non-byte-aligned delegated prefix keeps the adjacent bits untouched. */
	memset(address, 0, sizeof(address));
	address[6] = 0xa8;
	address[7] = 0x0b;
	assert(dhcp6c_pd_sla_insert(address, 53, 7, 0x55U) == 0);
	assert(address[6] == 0xad);
	assert(address[7] == 0x5b);
}

static void
test_byte_aligned_and_full_width_sla_fields(void)
{
	unsigned char address[16];

	memset(address, 0, sizeof(address));
	assert(dhcp6c_pd_sla_insert(address, 56, 8, 0xa5U) == 0);
	assert(address[7] == 0xa5);

	memset(address, 0, sizeof(address));
	assert(dhcp6c_pd_sla_insert(address, 8, 32, 0x89abcdefU) == 0);
	assert(address[1] == 0x89);
	assert(address[2] == 0xab);
	assert(address[3] == 0xcd);
	assert(address[4] == 0xef);
}

static void
test_delegated_prefix_clears_host_bits_at_byte_boundaries(void)
{
	unsigned char address[16];
	int i;

	memset(address, 0xff, sizeof(address));
	assert(dhcp6c_pd_prefix_clear_host_bits(address, 53) == 0);
	for (i = 0; i < 6; ++i)
		assert(address[i] == 0xff);
	assert(address[6] == 0xf8);
	for (i = 7; i < 16; ++i)
		assert(address[i] == 0);

	memset(address, 0xff, sizeof(address));
	assert(dhcp6c_pd_prefix_clear_host_bits(address, 56) == 0);
	for (i = 0; i < 7; ++i)
		assert(address[i] == 0xff);
	for (i = 7; i < 16; ++i)
		assert(address[i] == 0);

	memset(address, 0xff, sizeof(address));
	assert(dhcp6c_pd_prefix_clear_host_bits(address, 128) == 0);
	for (i = 0; i < 16; ++i)
		assert(address[i] == 0xff);
}

static void
test_sla_insertion_preserves_only_canonical_prefix_bits(void)
{
	unsigned char address[16];

	memset(address, 0xff, sizeof(address));
	assert(dhcp6c_pd_prefix_clear_host_bits(address, 53) == 0);
	assert(dhcp6c_pd_sla_insert(address, 53, 7, 0x55U) == 0);
	assert(address[6] == 0xfd);
	assert(address[7] == 0x50);
	assert(address[8] == 0);
	assert(address[15] == 0);
}

static void
test_prefix_canonicalization_rejects_invalid_input_atomically(void)
{
	unsigned char address[16];
	unsigned char original[16];

	memset(address, 0x5a, sizeof(address));
	memcpy(original, address, sizeof(address));
	errno = 0;
	assert(dhcp6c_pd_prefix_clear_host_bits(address, -1) == -1);
	assert(errno == EINVAL);
	assert(memcmp(address, original, sizeof(address)) == 0);

	errno = 0;
	assert(dhcp6c_pd_prefix_clear_host_bits(address, 129) == -1);
	assert(errno == EINVAL);
	assert(memcmp(address, original, sizeof(address)) == 0);
}

static void
test_invalid_sla_configurations_are_rejected_atomically(void)
{
	unsigned char address[16];
	unsigned char original[16];

	memset(address, 0x5a, sizeof(address));
	memcpy(original, address, sizeof(address));
	errno = 0;
	assert(dhcp6c_pd_sla_insert(address, 56, 4, 16U) == -1);
	assert(errno == EINVAL);
	assert(memcmp(address, original, sizeof(address)) == 0);

	errno = 0;
	assert(dhcp6c_pd_sla_insert(address, 56, 0, 1U) == -1);
	assert(errno == EINVAL);
	assert(memcmp(address, original, sizeof(address)) == 0);

	errno = 0;
	assert(dhcp6c_pd_sla_insert(address, 8, 33, 1U) == -1);
	assert(errno == EINVAL);
	assert(memcmp(address, original, sizeof(address)) == 0);

	errno = 0;
	assert(dhcp6c_pd_sla_insert(address, 128, 1, 1U) == -1);
	assert(errno == EINVAL);
	assert(memcmp(address, original, sizeof(address)) == 0);

	assert(dhcp6c_pd_sla_id_valid(0, 0U) == 0);
	assert(dhcp6c_pd_sla_id_valid(32, 0xffffffffU) == 0);
	assert(dhcp6c_pd_sla_id_valid(4, 0x0fU) == 0);
	assert(dhcp6c_pd_sla_id_valid(4, 0x10U) == -1);
	assert(dhcp6c_pd_sla_id_valid(33, 0U) == -1);
}

int
main(void)
{
	test_partial_sla_field_is_network_bit_aligned();
	test_delegated_prefix_clears_host_bits_at_byte_boundaries();
	test_sla_insertion_preserves_only_canonical_prefix_bits();
	test_prefix_canonicalization_rejects_invalid_input_atomically();
	test_byte_aligned_and_full_width_sla_fields();
	test_invalid_sla_configurations_are_rejected_atomically();
	puts("DHCPv6 delegated-prefix SLA bit placement policy: PASS");
	return 0;
}
