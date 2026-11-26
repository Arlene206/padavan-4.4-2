#ifndef IAPP_EVENT_LOG_POLICY_H
#define IAPP_EVENT_LOG_POLICY_H

#include <stddef.h>

/*
 * Validate a driver-returned event table without overflowing a size
 * calculation. returned_size includes the table header (Num), and maximum
 * size is the capacity originally supplied to the driver.
 */
static inline int
iapp_event_log_table_fits(size_t returned_size, size_t event_count,
			  size_t max_events, size_t header_size,
			  size_t record_size, size_t maximum_size)
{
	if (header_size == 0 || record_size == 0 ||
	    returned_size < header_size || returned_size > maximum_size ||
	    event_count > max_events)
		return 0;

	return event_count <= (returned_size - header_size) / record_size;
}

/* Check an append without multiplying event_count by record_size. */
static inline int
iapp_event_log_append_fits(size_t current_size, size_t event_count,
			   size_t record_size, size_t max_size)
{
	if (record_size == 0 || current_size > max_size)
		return 0;

	return event_count <= (max_size - current_size) / record_size;
}

#endif /* IAPP_EVENT_LOG_POLICY_H */
