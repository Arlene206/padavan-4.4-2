#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "../mtk_ft_r0khid_policy.h"

static void
test_r0khid_input_length_boundaries(void)
{
	assert(!mtk_ft_r0khid_input_length_valid(0U, 48U));
	assert(mtk_ft_r0khid_input_length_valid(1U, 48U));
	assert(mtk_ft_r0khid_input_length_valid(48U, 48U));
	assert(mtk_ft_r0khid_input_length_valid(49U, 48U));
	assert(!mtk_ft_r0khid_input_length_valid(50U, 48U));
	assert(!mtk_ft_r0khid_input_length_valid(UINT_MAX, 48U));
	assert(!mtk_ft_r0khid_input_length_valid(1U, 0U));
}

static void
test_r0khid_payload_normalization(void)
{
	unsigned char value[49];
	unsigned int length;

	memset(value, 'x', sizeof(value));
	assert(mtk_ft_r0khid_normalize_length(value, 1U, 48U, &length) == 0);
	assert(length == 1U);
	assert(mtk_ft_r0khid_normalize_length(value, 48U, 48U, &length) == 0);
	assert(length == 48U);

	value[48] = '\0';
	assert(mtk_ft_r0khid_normalize_length(value, 49U, 48U, &length) == 0);
	assert(length == 48U);

	value[48] = 'x';
	assert(mtk_ft_r0khid_normalize_length(value, 49U, 48U, &length) != 0);
	value[0] = '\0';
	assert(mtk_ft_r0khid_normalize_length(value, 1U, 48U, &length) != 0);
	assert(mtk_ft_r0khid_normalize_length(value, 0U, 48U, &length) != 0);
	assert(mtk_ft_r0khid_normalize_length(NULL, 1U, 48U, &length) != 0);
	assert(mtk_ft_r0khid_normalize_length(value, 1U, 48U, NULL) != 0);
	assert(mtk_ft_r0khid_normalize_length(value, 1U, 0U, &length) != 0);
}

int
main(void)
{
	test_r0khid_input_length_boundaries();
	test_r0khid_payload_normalization();
	puts("MTK bounded 802.11r R0KHID length regression: PASS");
	return 0;
}
