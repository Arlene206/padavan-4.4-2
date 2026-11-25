#include <stdio.h>

#include "../raeth_qdma_register_offset_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_aligned_in_range_offsets(void)
{
	CHECK(raeth_qdma_register_offset_status(0U, 0x3fffU, 0) == 0);
	CHECK(raeth_qdma_register_offset_status(4U, 0x3fffU, 0) == 0);
	CHECK(raeth_qdma_register_offset_status(0x3ffcU, 0x3fffU, 0) == 0);
	return 0;
}

static int
test_unaligned_offsets_fail_closed(void)
{
	static const unsigned int offsets[] = {
		1U, 2U, 3U, 7U, 0x3ffU, 0x3fffU, 0x777U
	};
	unsigned int i;

	for (i = 0; i < sizeof(offsets) / sizeof(offsets[0]); ++i)
		CHECK(raeth_qdma_register_offset_status(offsets[i], 0x3fffU, 0) ==
		      -EINVAL);
	return 0;
}

static int
test_legacy_magic_is_mt7622_only(void)
{
	CHECK(raeth_qdma_register_offset_status(0x777U, 0x3fffU, 1) == 0);
	CHECK(raeth_qdma_register_offset_status(0x777U, 0x3fffU, 0) == -EINVAL);
	CHECK(raeth_qdma_register_offset_status(0x778U, 0x3fffU, 1) == 0);
	CHECK(raeth_qdma_register_offset_status(0x779U, 0x3fffU, 1) == -EINVAL);
	return 0;
}

static int
test_upper_bound_is_checked_before_alignment_exception(void)
{
	CHECK(raeth_qdma_register_offset_status(0x4000U, 0x3fffU, 1) == -EINVAL);
	CHECK(raeth_qdma_register_offset_status(0xffffffffU, 0x3fffU, 1) ==
	      -EINVAL);
	return 0;
}

int
main(void)
{
	CHECK(test_aligned_in_range_offsets() == 0);
	CHECK(test_unaligned_offsets_fail_closed() == 0);
	CHECK(test_legacy_magic_is_mt7622_only() == 0);
	CHECK(test_upper_bound_is_checked_before_alignment_exception() == 0);
	puts("RAETH QDMA register offset alignment policy: PASS");
	return 0;
}
