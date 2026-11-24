#include <stdio.h>

#include "../raeth_esw_reg_offset_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_aligned_offsets_within_each_chip_bound(void)
{
	CHECK(raeth_esw_reg_offset_status(0U, 0xfcU) == 0);
	CHECK(raeth_esw_reg_offset_status(0xfcU, 0xfcU) == 0);
	CHECK(raeth_esw_reg_offset_status(0U, 0x16cU) == 0);
	CHECK(raeth_esw_reg_offset_status(0x16cU, 0x16cU) == 0);
	CHECK(raeth_esw_reg_offset_status(0x100U, 0x16cU) == 0);
	return 0;
}

static int
test_unaligned_offsets_are_rejected(void)
{
	static const unsigned int offsets[] = {
		1U, 2U, 3U, 5U, 0xfDU, 0x101U, 0x16fU
	};
	unsigned int i;

	for (i = 0; i < sizeof(offsets) / sizeof(offsets[0]); ++i) {
		CHECK(raeth_esw_reg_offset_status(offsets[i], 0x16cU) ==
		      -EINVAL);
	}
	return 0;
}

static int
test_offsets_above_hardware_window_are_rejected(void)
{
	CHECK(raeth_esw_reg_offset_status(0x100U, 0xfcU) == -EINVAL);
	CHECK(raeth_esw_reg_offset_status(0x170U, 0x16cU) == -EINVAL);
	CHECK(raeth_esw_reg_offset_status(0xffffffffU, 0x16cU) == -EINVAL);
	return 0;
}

int
main(void)
{
	CHECK(test_aligned_offsets_within_each_chip_bound() == 0);
	CHECK(test_unaligned_offsets_are_rejected() == 0);
	CHECK(test_offsets_above_hardware_window_are_rejected() == 0);
	puts("RAETH ESW raw MMIO offset alignment policy: PASS");
	return 0;
}
