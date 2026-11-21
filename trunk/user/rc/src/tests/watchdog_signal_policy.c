#include <stdio.h>

#include "watchdog_signal_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_signal_events_are_recorded(void)
{
	volatile sig_atomic_t events = 0;

	watchdog_record_signal(&events, SIGALRM);
	watchdog_record_signal(&events, SIGHUP);
	watchdog_record_signal(&events, SIGUSR1);
	watchdog_record_signal(&events, SIGTERM);
	CHECK(events == (WD_PENDING_TIMER | WD_PENDING_TIME_REFRESH |
			 WD_PENDING_WIFI_STATE | WD_PENDING_TERMINATE));
	return 0;
}

static int
test_repeated_standard_signals_coalesce(void)
{
	volatile sig_atomic_t events = 0;
	int i;

	for (i = 0; i < 8; i++) {
		watchdog_record_signal(&events, SIGALRM);
		watchdog_record_signal(&events, SIGHUP);
		watchdog_record_signal(&events, SIGUSR1);
	}
	CHECK(events == (WD_PENDING_TIMER | WD_PENDING_TIME_REFRESH |
			 WD_PENDING_WIFI_STATE));
	return 0;
}

static int
test_termination_is_sticky_and_coexists_with_work(void)
{
	volatile sig_atomic_t events = 0;

	watchdog_record_signal(&events, SIGTERM);
	watchdog_record_signal(&events, SIGALRM);
	watchdog_record_signal(&events, SIGUSR1);
	watchdog_record_signal(&events, SIGHUP);
	CHECK(events & WD_PENDING_TERMINATE);
	CHECK(events == (WD_PENDING_TERMINATE | WD_PENDING_TIMER |
			 WD_PENDING_TIME_REFRESH | WD_PENDING_WIFI_STATE));
	return 0;
}

static int
test_snapshot_clears_only_the_consumed_batch(void)
{
	volatile sig_atomic_t events = 0;
	sig_atomic_t snapshot;

	watchdog_record_signal(&events, SIGHUP);
	watchdog_record_signal(&events, SIGUSR1);
	snapshot = watchdog_take_pending_events(&events);
	CHECK(snapshot == (WD_PENDING_TIME_REFRESH | WD_PENDING_WIFI_STATE));
	CHECK(events == 0);

	watchdog_record_signal(&events, SIGALRM);
	CHECK(watchdog_take_pending_events(&events) == WD_PENDING_TIMER);
	CHECK(events == 0);
	return 0;
}

static int
test_unmanaged_signals_and_null_state_are_ignored(void)
{
	volatile sig_atomic_t events = 0;

	watchdog_record_signal(&events, SIGCHLD);
	watchdog_record_signal(NULL, SIGTERM);
	CHECK(events == 0);
	CHECK(watchdog_take_pending_events(NULL) == 0);
	return 0;
}

int
main(void)
{
	CHECK(test_signal_events_are_recorded() == 0);
	CHECK(test_repeated_standard_signals_coalesce() == 0);
	CHECK(test_termination_is_sticky_and_coexists_with_work() == 0);
	CHECK(test_snapshot_clears_only_the_consumed_batch() == 0);
	CHECK(test_unmanaged_signals_and_null_state_are_ignored() == 0);
	puts("watchdog signal deferral policy: PASS");
	return 0;
}
