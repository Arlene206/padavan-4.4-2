/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DHCP6C_TIMER_POLICY_H
#define DHCP6C_TIMER_POLICY_H

/* Reserve zero for timers not yet visited in a timer-check pass. */
static inline unsigned int
dhcp6c_timer_next_check_generation(unsigned int generation)
{
	generation++;
	return generation == 0 ? 1U : generation;
}

static inline int
dhcp6c_timer_seen_in_check(unsigned int timer_generation,
			   unsigned int check_generation)
{
	return timer_generation == check_generation;
}

/*
 * A callback may free its own timer or mutate any other timer in the list.
 * Either case invalidates a previously captured LIST_NEXT() pointer.
 */
static inline int
dhcp6c_timer_callback_requires_rescan(int timer_freed,
				     unsigned long long generation_before,
				     unsigned long long generation_after)
{
	return timer_freed || generation_before != generation_after;
}

#endif /* DHCP6C_TIMER_POLICY_H */
