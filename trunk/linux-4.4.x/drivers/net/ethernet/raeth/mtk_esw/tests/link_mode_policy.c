#include <assert.h>
#include <stdio.h>

#include "../link_mode_policy.h"

static void
test_all_defined_speed_and_flow_modes(void)
{
	unsigned int speed, flow;

	for (speed = 0U; speed <= 9U; speed++)
		for (flow = 0U; flow <= 2U; flow++)
			assert(mtk_esw_link_mode_is_valid((flow << 8) | speed));
	for (flow = 0U; flow <= 2U; flow++)
		assert(mtk_esw_link_mode_is_valid((flow << 8) | 15U));
}

static void
test_reserved_speed_flow_and_bits_are_rejected(void)
{
	unsigned int speed;

	for (speed = 10U; speed <= 14U; speed++)
		assert(!mtk_esw_link_mode_is_valid(speed));
	assert(!mtk_esw_link_mode_is_valid(3U << 8));
	assert(!mtk_esw_link_mode_is_valid(0x00000010U));
	assert(!mtk_esw_link_mode_is_valid(0x00000400U));
	assert(!mtk_esw_link_mode_is_valid(0x00010000U));
	assert(!mtk_esw_link_mode_is_valid(0x80000000U));
}

int
main(void)
{
	test_all_defined_speed_and_flow_modes();
	test_reserved_speed_flow_and_bits_are_rejected();
	puts("MTK ESW link-mode validation policy: PASS");
	return 0;
}
