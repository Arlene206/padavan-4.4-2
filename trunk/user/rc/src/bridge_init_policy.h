#ifndef RC_BRIDGE_INIT_POLICY_H
#define RC_BRIDGE_INIT_POLICY_H

#include <errno.h>

typedef struct {
	int first_status;
	unsigned int failure_count;
	const char *first_operation;
} rc_bridge_init_status_t;

static inline void
rc_bridge_init_status_reset(rc_bridge_init_status_t *status)
{
	if (!status)
		return;
	status->first_status = 0;
	status->failure_count = 0;
	status->first_operation = NULL;
}

static inline void
rc_bridge_init_record(rc_bridge_init_status_t *status, const char *operation,
		      int operation_status)
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
rc_bridge_init_result(const rc_bridge_init_status_t *status)
{
	return status ? status->first_status : EINVAL;
}

/* An existing bridge is success only when a kernel bridge-port query confirms it. */
static inline int
rc_bridge_init_add_bridge_result(int add_status, int add_errno,
				 int bridge_probe_status)
{
	if (add_status == 0)
		return 0;
	if (add_status < 0 && add_errno == EEXIST && bridge_probe_status >= 0)
		return 0;
	return add_status;
}

#endif /* RC_BRIDGE_INIT_POLICY_H */
