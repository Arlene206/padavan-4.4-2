#ifndef RC_SERVICE_START_STATUS_H
#define RC_SERVICE_START_STATUS_H

/* Keep the first nonzero boot-service status without skipping later starts. */
static inline int
rc_first_service_start_status(int first_status, int next_status)
{
	return first_status != 0 ? first_status : next_status;
}

/* A queued storage event is acknowledged only after required starts pass. */
static inline int
rc_service_start_event_is_complete(int start_status)
{
	return start_status == 0;
}

#endif
