#ifndef WIRELESS_GUEST_ISOLATION_POLICY_H
#define WIRELESS_GUEST_ISOLATION_POLICY_H

#include <errno.h>

typedef struct {
	int first_status;
	unsigned int failure_count;
	const char *first_operation;
} wireless_guest_isolation_status_t;

static inline void
wireless_guest_isolation_status_reset(wireless_guest_isolation_status_t *status)
{
	if (!status)
		return;
	status->first_status = 0;
	status->failure_count = 0;
	status->first_operation = 0;
}

static inline void
wireless_guest_isolation_status_record(wireless_guest_isolation_status_t *status,
				       const char *operation, int operation_status)
{
	if (!status || operation_status == 0)
		return;
	if (status->first_status == 0) {
		status->first_status = operation_status;
		status->first_operation = operation;
	}
	status->failure_count++;
}

static inline int
wireless_guest_isolation_status_result(const wireless_guest_isolation_status_t *status)
{
	return (status && status->first_status == 0) ? 0 : -EIO;
}

/* Guest-control APIs retain 0/1 as unchanged/changed on success. */
static inline int
wireless_guest_isolation_control_result(int changed,
					const wireless_guest_isolation_status_t *status)
{
	return wireless_guest_isolation_status_result(status) != 0 ? -EIO : changed;
}

#endif /* WIRELESS_GUEST_ISOLATION_POLICY_H */
