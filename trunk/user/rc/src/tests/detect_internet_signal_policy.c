#include <stdio.h>
#include <errno.h>
#include "detect_internet_signal_policy.h"
#include "detect_internet_notification_policy.h"

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
test_notification_queue_failure_preserves_errno_or_falls_back(void)
{
	CHECK(detect_internet_notification_error(0, 0) == 0);
	CHECK(detect_internet_notification_error(-1, ENOSPC) == ENOSPC);
	CHECK(detect_internet_notification_error(-1, 0) == EIO);
	CHECK(detect_internet_notification_error(EACCES, 0) == EACCES);
	CHECK(detect_internet_notification_error(EIO, ENOSPC) == ENOSPC);
	return 0;
}

static int
test_launcher_and_pidfile_errors_are_normalized(void)
{
	CHECK(detect_internet_operation_error(0, EPERM) == 0);
	CHECK(detect_internet_operation_error(-1, ESRCH) == ESRCH);
	CHECK(detect_internet_operation_error(-1, 0) == EIO);
	CHECK(detect_internet_operation_error(23, 0) == 23);
	CHECK(detect_internet_operation_error(-1, ENOSPC) == ENOSPC);
	return 0;
}

static int
test_pause_treats_absence_as_noop_and_reports_identity_failure(void)
{
	CHECK(detect_internet_pause_error(0, EPERM) == 0);
	CHECK(detect_internet_pause_error(-1, ENOENT) == 0);
	CHECK(detect_internet_pause_error(-1, ESRCH) == 0);
	CHECK(detect_internet_pause_error(-1, EPERM) == EPERM);
	CHECK(detect_internet_pause_error(-1, 0) == EIO);
	CHECK(detect_internet_pause_error(37, 0) == 37);
	return 0;
}

static int
test_resume_delay_store_failure_is_not_ignored(void)
{
	CHECK(detect_internet_operation_error(0, EACCES) == 0);
	CHECK(detect_internet_operation_error(-1, EACCES) == EACCES);
	CHECK(detect_internet_operation_error(-1, 0) == EIO);
	CHECK(detect_internet_operation_error(17, 0) == 17);
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
	CHECK(test_notification_queue_failure_preserves_errno_or_falls_back() == 0);
	CHECK(test_launcher_and_pidfile_errors_are_normalized() == 0);
	CHECK(test_pause_treats_absence_as_noop_and_reports_identity_failure() == 0);
	CHECK(test_resume_delay_store_failure_is_not_ignored() == 0);
	CHECK(test_unknown_signals_and_invalid_arguments_are_ignored() == 0);
	puts("detect_internet signal deferral policy: PASS");
	return 0;
}
