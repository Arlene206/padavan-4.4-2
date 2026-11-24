#include <assert.h>
#include <limits.h>
#include <stdio.h>

#include "../mtk_wpa_psk_policy.h"

static void
test_wpa_psk_length_boundaries(void)
{
	assert(mtk_wpa_psk_length_valid(0U, 64U));
	assert(mtk_wpa_psk_length_valid(1U, 64U));
	assert(mtk_wpa_psk_length_valid(63U, 64U));
	assert(mtk_wpa_psk_length_valid(64U, 64U));
	assert(!mtk_wpa_psk_length_valid(65U, 64U));
	assert(!mtk_wpa_psk_length_valid(UINT_MAX, 64U));
	assert(!mtk_wpa_psk_length_valid(0U, 0U));
}

int
main(void)
{
	test_wpa_psk_length_boundaries();
	puts("MTK WPA PSK input boundary regression: PASS");
	return 0;
}
