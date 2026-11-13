#include <limits.h>
#include <stdio.h>

#include "gpio_btn_timing_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_short_press_deadline(void)
{
	CHECK(gpio_btn_elapsed_count(0, 0) == 1);
	CHECK(gpio_btn_short_press_count(gpio_btn_elapsed_count(0, 699999999LL)));
	CHECK(gpio_btn_elapsed_count(0, 700000000LL) == 8);
	CHECK(gpio_btn_short_press_count(gpio_btn_elapsed_count(0, 700000000LL)));
	CHECK(gpio_btn_short_press_count(gpio_btn_elapsed_count(0, 799999999LL)));
	CHECK(gpio_btn_elapsed_count(0, 800000000LL) == 9);
	CHECK(!gpio_btn_short_press_count(gpio_btn_elapsed_count(0, 800000000LL)));
	CHECK(!gpio_btn_short_press_count(0));
	return 0;
}

static int
test_long_press_deadline(void)
{
	CHECK(gpio_btn_elapsed_count(2, 999999999LL) == 30);
	CHECK(!gpio_btn_long_press_count(gpio_btn_elapsed_count(2, 999999999LL)));
	CHECK(gpio_btn_elapsed_count(3, 0) == 31);
	CHECK(gpio_btn_long_press_count(gpio_btn_elapsed_count(3, 0)));
	return 0;
}

static int
test_reset_press_deadline(void)
{
	CHECK(gpio_btn_elapsed_count(4, 999999999LL) == 50);
	CHECK(!gpio_btn_reset_press_count(gpio_btn_elapsed_count(4, 999999999LL)));
	CHECK(gpio_btn_elapsed_count(5, 0) == 51);
	CHECK(gpio_btn_reset_press_count(gpio_btn_elapsed_count(5, 0)));
	return 0;
}

static int
test_clock_delta_normalization_and_saturation(void)
{
	CHECK(gpio_btn_elapsed_count(0, -1) == 1);
	CHECK(gpio_btn_elapsed_count(-1, 999999999LL) == 1);
	CHECK(gpio_btn_elapsed_count(2147483647LL, 0) == INT_MAX);
	return 0;
}

int
main(void)
{
	CHECK(test_short_press_deadline() == 0);
	CHECK(test_long_press_deadline() == 0);
	CHECK(test_reset_press_deadline() == 0);
	CHECK(test_clock_delta_normalization_and_saturation() == 0);
	puts("GPIO button hold timing and threshold policy: PASS");
	return 0;
}
