#include <stdio.h>
#include "inicd_signal_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_signal_mapping(void)
{
	CHECK(inicd_event_for_signal(SIGALRM) == INICD_EVENT_POLL);
	CHECK(inicd_event_for_signal(SIGTERM) == INICD_EVENT_STOP);
	CHECK(inicd_event_for_signal(SIGINT) == 0);
	return 0;
}

static int
test_signal_coalescing_and_stop_precedence(void)
{
	volatile sig_atomic_t pending = 0;
	sig_atomic_t events;

	inicd_record_signal(&pending, SIGALRM);
	inicd_record_signal(&pending, SIGALRM);
	CHECK(pending == INICD_EVENT_POLL);
	CHECK(!inicd_event_should_stop(pending));
	inicd_record_signal(&pending, SIGTERM);
	CHECK(pending == (INICD_EVENT_POLL | INICD_EVENT_STOP));
	CHECK(inicd_event_should_stop(pending));
	events = inicd_take_pending_events(&pending);
	CHECK(events == (INICD_EVENT_POLL | INICD_EVENT_STOP));
	CHECK(inicd_event_should_stop(events));
	CHECK(pending == 0);
	CHECK(inicd_take_pending_events(&pending) == 0);
	return 0;
}

static int
test_null_event_storage(void)
{
	inicd_record_signal(NULL, SIGALRM);
	CHECK(inicd_take_pending_events(NULL) == 0);
	CHECK(!inicd_event_should_stop(INICD_EVENT_POLL));
	return 0;
}

int
main(void)
{
	CHECK(test_signal_mapping() == 0);
	CHECK(test_signal_coalescing_and_stop_precedence() == 0);
	CHECK(test_null_event_storage() == 0);
	puts("iNIC signal deferral policy: PASS");
	return 0;
}
