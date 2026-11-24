#include <stdio.h>

#include "../raeth_qdma_queue_mapping_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_valid_selectors(void)
{
	static const unsigned int selectors[] = {
		0U, 1U, 31U, 62U, 63U,
		0x100U, 0x101U, 0x13fU
	};
	static const unsigned int indices[] = {
		0U, 1U, 31U, 62U, 63U,
		0U, 1U, 63U
	};
	static const unsigned int separate[] = {
		0U, 0U, 0U, 0U, 0U,
		1U, 1U, 1U
	};
	unsigned int index, is_separate;
	unsigned int i;

	for (i = 0; i < sizeof(selectors) / sizeof(selectors[0]); ++i) {
		index = 0xffffffffU;
		is_separate = 0xffffffffU;
		CHECK(raeth_qdma_queue_mapping_decode(selectors[i], &index,
						       &is_separate) == 0);
		CHECK(index == indices[i]);
		CHECK(is_separate == separate[i]);
	}
	return 0;
}

static int
test_reserved_bits_are_rejected_without_output_changes(void)
{
	static const unsigned int selectors[] = {
		0x40U, 0x7fU, 0xffU, 0x140U, 0x1ffU,
		0x200U, 0x1000U, 0xffffffffU
	};
	unsigned int index, separate, i;

	for (i = 0; i < sizeof(selectors) / sizeof(selectors[0]); ++i) {
		index = 17U;
		separate = 23U;
		CHECK(raeth_qdma_queue_mapping_decode(selectors[i], &index,
						       &separate) == -EINVAL);
		CHECK(index == 17U);
		CHECK(separate == 23U);
	}
	return 0;
}

static int
test_queue_ids_fit_the_six_bit_descriptor_field(void)
{
	CHECK(raeth_qdma_queue_id_status(0U) == 0);
	CHECK(raeth_qdma_queue_id_status(1U) == 0);
	CHECK(raeth_qdma_queue_id_status(31U) == 0);
	CHECK(raeth_qdma_queue_id_status(32U) == 0);
	CHECK(raeth_qdma_queue_id_status(63U) == 0);
	CHECK(raeth_qdma_queue_id_status(64U) == -EINVAL);
	CHECK(raeth_qdma_queue_id_status(0xffffffffU) == -EINVAL);
	return 0;
}

static int
test_null_outputs_are_rejected(void)
{
	unsigned int index = 0U, separate = 0U;

	CHECK(raeth_qdma_queue_mapping_decode(0U, NULL, &separate) == -EINVAL);
	CHECK(raeth_qdma_queue_mapping_decode(0U, &index, NULL) == -EINVAL);
	return 0;
}

int
main(void)
{
	CHECK(test_valid_selectors() == 0);
	CHECK(test_reserved_bits_are_rejected_without_output_changes() == 0);
	CHECK(test_queue_ids_fit_the_six_bit_descriptor_field() == 0);
	CHECK(test_null_outputs_are_rejected() == 0);
	puts("RAETH QDMA queue-mapping selector bounds: PASS");
	return 0;
}
