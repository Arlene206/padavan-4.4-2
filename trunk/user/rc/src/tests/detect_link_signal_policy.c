#include <stdio.h>
#include "detect_link_signal_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_signals_are_recorded_without_dispatch(void)
{
	volatile sig_atomic_t events = 0;

	detect_link_record_signal(&events, SIGALRM);
	detect_link_record_signal(&events, SIGHUP);
	detect_link_record_signal(&events, SIGUSR1);
	CHECK(events == (DL_PENDING_TIMER | DL_PENDING_LED_REFRESH |
	                 DL_PENDING_RESET));
	return 0;
}

static int
test_repeated_standard_signals_coalesce(void)
{
	volatile sig_atomic_t events = 0;
	int i;

	for (i = 0; i < 8; i++) {
		detect_link_record_signal(&events, SIGALRM);
		detect_link_record_signal(&events, SIGHUP);
	}
	CHECK(events == (DL_PENDING_TIMER | DL_PENDING_LED_REFRESH));
	return 0;
}

static int
test_termination_remains_pending_with_other_events(void)
{
	volatile sig_atomic_t events = 0;

	detect_link_record_signal(&events, SIGTERM);
	detect_link_record_signal(&events, SIGALRM);
	detect_link_record_signal(&events, SIGUSR1);
	CHECK(events & DL_PENDING_TERMINATE);
	CHECK(events == (DL_PENDING_TERMINATE | DL_PENDING_TIMER |
	                 DL_PENDING_RESET));
	return 0;
}

static int
test_snapshot_is_cleared_for_next_wait(void)
{
	volatile sig_atomic_t events = 0;
	sig_atomic_t snapshot;

	detect_link_record_signal(&events, SIGHUP);
	detect_link_record_signal(&events, SIGUSR1);
	snapshot = detect_link_take_pending_events(&events);
	CHECK(snapshot == (DL_PENDING_LED_REFRESH | DL_PENDING_RESET));
	CHECK(events == 0);

	detect_link_record_signal(&events, SIGALRM);
	CHECK(detect_link_take_pending_events(&events) == DL_PENDING_TIMER);
	CHECK(events == 0);
	return 0;
}

static int
test_unknown_signals_and_null_state_are_ignored(void)
{
	volatile sig_atomic_t events = 0;

	detect_link_record_signal(&events, SIGCHLD);
	detect_link_record_signal(NULL, SIGTERM);
	CHECK(events == 0);
	CHECK(detect_link_take_pending_events(NULL) == 0);
	return 0;
}

int
main(void)
{
	CHECK(test_signals_are_recorded_without_dispatch() == 0);
	CHECK(test_repeated_standard_signals_coalesce() == 0);
	CHECK(test_termination_remains_pending_with_other_events() == 0);
	CHECK(test_snapshot_is_cleared_for_next_wait() == 0);
	CHECK(test_unknown_signals_and_null_state_are_ignored() == 0);
	puts("detect_link signal deferral policy: PASS");
	return 0;
}
