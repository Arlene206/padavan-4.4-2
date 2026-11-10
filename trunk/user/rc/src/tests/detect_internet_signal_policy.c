#include <stdio.h>
#include "detect_internet_signal_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_timer_and_activity_are_deferred(void)
{
	volatile sig_atomic_t events = 0;
	volatile sig_atomic_t control = DI_CONTROL_NONE;
	volatile sig_atomic_t paused = 0;

	detect_internet_record_signal(&events, &control, &paused, SIGALRM);
	detect_internet_record_signal(&events, &control, &paused, SIGHUP);
	CHECK(events == (DI_PENDING_TIMER | DI_PENDING_ACTIVITY));
	CHECK(control == DI_CONTROL_NONE);
	CHECK(paused == 0);
	return 0;
}

static int
test_standard_signal_coalescing_is_bounded(void)
{
	volatile sig_atomic_t events = 0;
	volatile sig_atomic_t control = DI_CONTROL_NONE;
	volatile sig_atomic_t paused = 0;

	detect_internet_record_signal(&events, &control, &paused, SIGALRM);
	detect_internet_record_signal(&events, &control, &paused, SIGALRM);
	detect_internet_record_signal(&events, &control, &paused, SIGALRM);
	CHECK(events == DI_PENDING_TIMER);
	return 0;
}

static int
test_latest_control_request_wins(void)
{
	volatile sig_atomic_t events = 0;
	volatile sig_atomic_t control = DI_CONTROL_NONE;
	volatile sig_atomic_t paused = 0;

	detect_internet_record_signal(&events, &control, &paused, SIGUSR2);
	CHECK(control == DI_CONTROL_PAUSE);
	CHECK(paused == 1);
	detect_internet_record_signal(&events, &control, &paused, SIGUSR1);
	CHECK(control == DI_CONTROL_RESUME);
	/* Resume is applied in normal context, not from the signal handler. */
	CHECK(paused == 1);
	return 0;
}

static int
test_termination_is_fail_safe_and_sticky(void)
{
	volatile sig_atomic_t events = 0;
	volatile sig_atomic_t control = DI_CONTROL_NONE;
	volatile sig_atomic_t paused = 0;

	detect_internet_record_signal(&events, &control, &paused, SIGTERM);
	detect_internet_record_signal(&events, &control, &paused, SIGUSR1);
	CHECK(events == DI_PENDING_TERMINATE);
	CHECK(control == DI_CONTROL_RESUME);
	CHECK(paused == 1);
	return 0;
}

static int
test_unknown_signals_and_invalid_arguments_are_ignored(void)
{
	volatile sig_atomic_t events = 0;
	volatile sig_atomic_t control = DI_CONTROL_NONE;
	volatile sig_atomic_t paused = 0;

	detect_internet_record_signal(&events, &control, &paused, SIGCHLD);
	detect_internet_record_signal(NULL, &control, &paused, SIGALRM);
	detect_internet_record_signal(&events, NULL, &paused, SIGUSR1);
	detect_internet_record_signal(&events, &control, NULL, SIGUSR2);
	CHECK(events == 0);
	CHECK(control == DI_CONTROL_NONE);
	CHECK(paused == 0);
	return 0;
}

int
main(void)
{
	CHECK(test_timer_and_activity_are_deferred() == 0);
	CHECK(test_standard_signal_coalescing_is_bounded() == 0);
	CHECK(test_latest_control_request_wins() == 0);
	CHECK(test_termination_is_fail_safe_and_sticky() == 0);
	CHECK(test_unknown_signals_and_invalid_arguments_are_ignored() == 0);
	puts("detect_internet signal deferral policy: PASS");
	return 0;
}
