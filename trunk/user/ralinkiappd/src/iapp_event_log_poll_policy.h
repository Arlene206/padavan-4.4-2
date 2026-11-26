#ifndef IAPP_EVENT_LOG_POLL_POLICY_H
#define IAPP_EVENT_LOG_POLL_POLICY_H

/* Event snapshots are expected at least once per elapsed wall-clock second. */
static inline int
iapp_event_log_poll_due(long elapsed_seconds)
{
	return elapsed_seconds >= 1;
}

#endif /* IAPP_EVENT_LOG_POLL_POLICY_H */
