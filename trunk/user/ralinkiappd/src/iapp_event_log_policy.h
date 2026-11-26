#ifndef IAPP_EVENT_LOG_POLICY_H
#define IAPP_EVENT_LOG_POLICY_H

#include <stddef.h>

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
