#ifndef GPIO_BTN_TIMING_POLICY_H
#define GPIO_BTN_TIMING_POLICY_H

#include <limits.h>

/* The daemon updates held-button state every 100 ms. The initial press is
 * count 1, so count 8 represents the complete [700 ms, 800 ms) bucket. */
#define GPIO_BTN_HOLD_INTERVAL_US 100000
#define GPIO_BTN_HOLD_INTERVAL_NS 100000000LL
#define GPIO_BTN_HOLD_TICKS_PER_SECOND (1000000000LL / GPIO_BTN_HOLD_INTERVAL_NS)
#define GPIO_BTN_SHORT_MAX_COUNT 8
#define GPIO_BTN_LONG_AFTER_COUNT 30
#define GPIO_BTN_RESET_AFTER_COUNT 50

/* Convert a CLOCK_MONOTONIC delta into the same count used by the button
 * state machine: one tick at press time, then one per elapsed 100 ms. */
static inline int
gpio_btn_elapsed_count(long long sec, long long nsec)
{
	unsigned long long ticks;

	if (nsec < 0) {
		sec--;
		nsec += 1000000000LL;
	}
	if (sec < 0) {
		sec = 0;
		nsec = 0;
	}

	if ((unsigned long long)sec >
	    (unsigned long long)INT_MAX / GPIO_BTN_HOLD_TICKS_PER_SECOND)
		return INT_MAX;

	ticks = (unsigned long long)sec * GPIO_BTN_HOLD_TICKS_PER_SECOND +
		(unsigned long long)nsec / GPIO_BTN_HOLD_INTERVAL_NS + 1ULL;
	return ticks > INT_MAX ? INT_MAX : (int)ticks;
}

static inline int
gpio_btn_short_press_count(int count)
{
	return count > 0 && count <= GPIO_BTN_SHORT_MAX_COUNT;
}

static inline int
gpio_btn_long_press_count(int count)
{
	return count > GPIO_BTN_LONG_AFTER_COUNT;
}

static inline int
gpio_btn_reset_press_count(int count)
{
	return count > GPIO_BTN_RESET_AFTER_COUNT;
}

#endif /* GPIO_BTN_TIMING_POLICY_H */
