/* SPDX-License-Identifier: GPL-2.0 */
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "dhcp6c_timer_policy.h"

struct test_timer {
	int id;
	unsigned int check_generation;
	struct test_timer *next;
};

static struct test_timer *
new_timer(int id)
{
	struct test_timer *timer = calloc(1, sizeof(*timer));

	assert(timer != NULL);
	timer->id = id;
	return timer;
}

static void
insert_head(struct test_timer **head, struct test_timer *timer,
	    unsigned int check_generation, unsigned long long *list_generation)
{
	timer->check_generation = check_generation;
	timer->next = *head;
	*head = timer;
	++*list_generation;
}

static void
remove_timer(struct test_timer **head, struct test_timer *timer,
	    unsigned long long *list_generation)
{
	struct test_timer **cursor = head;

	while (*cursor != NULL && *cursor != timer)
		cursor = &(*cursor)->next;
	assert(*cursor == timer);
	*cursor = timer->next;
	++*list_generation;
	free(timer);
}

static void
test_generation_wrap_and_callback_invalidation(void)
{
	assert(dhcp6c_timer_next_check_generation(0U) == 1U);
	assert(dhcp6c_timer_next_check_generation(UINT_MAX) == 1U);
	assert(dhcp6c_timer_seen_in_check(7U, 7U));
	assert(!dhcp6c_timer_seen_in_check(6U, 7U));

	assert(dhcp6c_timer_callback_requires_rescan(1, 11ULL, 11ULL));
	assert(dhcp6c_timer_callback_requires_rescan(0, 11ULL, 12ULL));
	assert(!dhcp6c_timer_callback_requires_rescan(0, 11ULL, 11ULL));
}

/*
 * Reproduce the critical callback pattern: while timer 1 is running, free
 * the saved-next timer 2 and insert a fresh timer at the list head. Traversal
 * must restart, skip timer 1 and the new timer for this pass, and still visit
 * timer 3 exactly once without dereferencing the freed timer 2.
 */
static void
test_callback_removal_restarts_without_revisiting_or_running_new_timer(void)
{
	struct test_timer *first = new_timer(1);
	struct test_timer *second = new_timer(2);
	struct test_timer *third = new_timer(3);
	struct test_timer *inserted = new_timer(4);
	struct test_timer *head = NULL;
	struct test_timer *timer;
	unsigned int generation = 1U;
	unsigned long long list_generation = 0ULL;
	int visited_first = 0, visited_second = 0;
	int visited_third = 0, visited_inserted = 0;
	int callback_done = 0;

	insert_head(&head, third, 0U, &list_generation);
	insert_head(&head, second, 0U, &list_generation);
	insert_head(&head, first, 0U, &list_generation);

	timer = head;
	while (timer != NULL) {
		unsigned long long generation_before;

		if (dhcp6c_timer_seen_in_check(timer->check_generation, generation)) {
			timer = timer->next;
			continue;
		}
		timer->check_generation = generation;
		switch (timer->id) {
		case 1: ++visited_first; break;
		case 2: ++visited_second; break;
		case 3: ++visited_third; break;
		case 4: ++visited_inserted; break;
		}

		if (timer->id == 1 && !callback_done) {
			generation_before = list_generation;
			remove_timer(&head, second, &list_generation);
			second = NULL;
			insert_head(&head, inserted, generation, &list_generation);
			callback_done = 1;
			if (dhcp6c_timer_callback_requires_rescan(0, generation_before,
								 list_generation)) {
				timer = head;
				continue;
			}
		}

		timer = timer->next;
	}

	assert(visited_first == 1);
	assert(visited_second == 0);
	assert(visited_third == 1);
	assert(visited_inserted == 0);
	assert(second == NULL);

	while (head != NULL) {
		timer = head;
		head = timer->next;
		free(timer);
	}
	puts("DHCPv6 timer callback mutation traversal regression: PASS");
}

int
main(void)
{
	test_generation_wrap_and_callback_invalidation();
	test_callback_removal_restarts_without_revisiting_or_running_new_timer();
	puts("DHCPv6 timer generation and callback invalidation policy: PASS");
	return 0;
}
