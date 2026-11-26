#include <assert.h>
#include <stdio.h>

#include "iapp_event_log_policy.h"

int
main(void)
{
	const size_t header_size = 4;
	const size_t record_size = 12;
	const size_t maximum_events = 30;
	const size_t maximum_size = header_size + maximum_events * record_size;

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

	puts("Ralink IAPP event-table returned-length policy: PASS");
	return 0;
}
