#include <assert.h>
#include <stdio.h>

#include "iapp_event_log_policy.h"
#include "iapp_event_log_poll_policy.h"

static void
test_event_log_poll_uses_monotonic_elapsed_time(void)
{
	struct timespec last, now;

	last.tv_sec = 100;
	last.tv_nsec = 900000000L;
	now.tv_sec = 101;
	now.tv_nsec = 899999999L;
	assert(iapp_event_log_poll_due(&now, &last) == 0);

	now.tv_nsec = 900000000L;
	assert(iapp_event_log_poll_due(&now, &last) == 1);

	now.tv_sec = 102;
	now.tv_nsec = 0;
	assert(iapp_event_log_poll_due(&now, &last) == 1);

	/* A backwards clock sample must not trigger a spurious poll. */
	now.tv_sec = 99;
	now.tv_nsec = 999999999L;
	assert(iapp_event_log_poll_due(&now, &last) == 0);

	now.tv_sec = 100;
	now.tv_nsec = 800000000L;
	assert(iapp_event_log_poll_due(&now, &last) == 0);

	assert(iapp_event_log_poll_due(NULL, &last) == -1);
	assert(iapp_event_log_poll_due(&now, NULL) == -1);
	now.tv_sec = -1;
	now.tv_nsec = 0;
	assert(iapp_event_log_poll_due(&now, &last) == -1);
	now.tv_sec = 100;
	last.tv_sec = -1;
	assert(iapp_event_log_poll_due(&now, &last) == -1);
	last.tv_sec = 100;
	now.tv_nsec = -1;
	assert(iapp_event_log_poll_due(&now, &last) == -1);
	now.tv_nsec = 1000000000L;
	assert(iapp_event_log_poll_due(&now, &last) == -1);
}

int
main(void)
{
	const size_t header_size = 4;
	const size_t record_size = 12;
	const size_t maximum_events = 30;
	const size_t maximum_size = header_size + maximum_events * record_size;

	test_event_log_poll_uses_monotonic_elapsed_time();

	assert(iapp_event_log_table_fits(header_size, 0, maximum_events,
					 header_size, record_size, maximum_size));
	assert(iapp_event_log_table_fits(header_size + 2 * record_size, 2,
					 maximum_events, header_size,
					 record_size, maximum_size));
	assert(iapp_event_log_table_fits(maximum_size, maximum_events,
					 maximum_events, header_size,
					 record_size, maximum_size));

	assert(!iapp_event_log_table_fits(header_size - 1, 0, maximum_events,
					  header_size, record_size, maximum_size));
	assert(!iapp_event_log_table_fits(header_size + 2 * record_size - 1,
					 2, maximum_events, header_size,
					 record_size, maximum_size));
	assert(!iapp_event_log_table_fits(maximum_size, maximum_events + 1,
					 maximum_events, header_size,
					 record_size, maximum_size));
	assert(!iapp_event_log_table_fits(maximum_size + 1, 1,
					 maximum_events, header_size,
					 record_size, maximum_size));
	assert(!iapp_event_log_table_fits(header_size, 1, maximum_events,
					 header_size, 0, maximum_size));
	assert(!iapp_event_log_table_fits(header_size, 0, maximum_events,
					 header_size + 1, record_size, maximum_size));

	puts("Ralink IAPP event-log policy: PASS");
	return 0;
}
