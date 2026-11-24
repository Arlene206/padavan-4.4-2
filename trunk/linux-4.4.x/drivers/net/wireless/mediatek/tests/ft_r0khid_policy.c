#include <assert.h>
#include <limits.h>
#include <stdio.h>

#include "../mtk_ft_r0khid_policy.h"

static void
test_r0khid_length_boundaries(void)
{
	assert(!mtk_ft_r0khid_length_valid(0U, 48U));
	assert(mtk_ft_r0khid_length_valid(1U, 48U));
	assert(mtk_ft_r0khid_length_valid(48U, 48U));
	assert(!mtk_ft_r0khid_length_valid(49U, 48U));
	assert(!mtk_ft_r0khid_length_valid(UINT_MAX, 48U));
	assert(!mtk_ft_r0khid_length_valid(1U, 0U));
}

int
main(void)
{
	test_r0khid_length_boundaries();
	puts("MTK bounded 802.11r R0KHID length regression: PASS");
	return 0;
}
