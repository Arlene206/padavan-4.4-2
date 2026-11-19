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

/*
 * is_module_loaded_status() uses 1/0/-errno for loaded/absent/unknown.
 * A module requested by an enabled isolation policy must be confirmed loaded.
 */
static inline int
wireless_guest_isolation_module_presence_error(int module_status)
{
	if (module_status > 0)
		return 0;
	if (module_status == 0)
		return ENODEV;
	return -module_status;
}

enum {
	WIRELESS_GUEST_EBTABLES_DISABLE_NONE = 0,
	WIRELESS_GUEST_EBTABLES_DISABLE_CLEANUP_ONLY = 1,
	WIRELESS_GUEST_EBTABLES_DISABLE_CLEANUP_AND_UNLOAD = 2
};

/*
 * Unknown module state still permits a best-effort rules cleanup, but never
 * permits unloading modules whose presence and ownership are unverified.
 */
static inline int
wireless_guest_isolation_ebtables_disable_action(int module_status)
{
	if (module_status > 0)
		return WIRELESS_GUEST_EBTABLES_DISABLE_CLEANUP_AND_UNLOAD;
	if (module_status == 0)
		return WIRELESS_GUEST_EBTABLES_DISABLE_NONE;
	return WIRELESS_GUEST_EBTABLES_DISABLE_CLEANUP_ONLY;
}

/*
 * get_interface_flags() returns -1 when the kernel cannot report link flags.
 * Do not silently interpret that state as "interface down".
 */
static inline int
wireless_guest_isolation_link_state(int interface_flags, int interface_error,
				    int absent_error, int up_mask, int *is_up)
{
	if (!is_up)
		return -EINVAL;
	if (interface_flags < 0) {
		if (interface_error != absent_error)
			return -EIO;
		*is_up = 0;
		return 0;
	}
	*is_up = (interface_flags & up_mask) != 0;
	return 0;
}

/* Guest-control APIs retain 0/1 as unchanged/changed on success. */
static inline int
wireless_guest_isolation_control_result(int changed,
					const wireless_guest_isolation_status_t *status)
{
	return wireless_guest_isolation_status_result(status) != 0 ? -EIO : changed;
}

 
/* Do not tear down a guest AP that was already active before startup. */
static inline int
wireless_guest_startup_rollback_candidate(int was_up_before, int startup_attempted)
{
	return !was_up_before && startup_attempted;
}

/*
 * Callback-driven rollback lets host tests verify that every safety action is
 * attempted even when an earlier cleanup operation fails.
 */
typedef struct {
	int (*bring_down)(void *context);
	int (*detach_bridge)(void *context);
	int (*verify_down)(void *context);
} wireless_guest_isolation_rollback_ops_t;

static inline int
wireless_guest_isolation_status_rollback(
	wireless_guest_isolation_status_t *status,
	const wireless_guest_isolation_rollback_ops_t *ops, void *context)
{
	if (!status)
		return -EIO;
	if (!ops || !ops->bring_down || !ops->detach_bridge || !ops->verify_down) {
		wireless_guest_isolation_status_record(status,
			"validate guest AP rollback operations", -EINVAL);
		return -EIO;
	}

	wireless_guest_isolation_status_record(status,
		"request guest AP down during rollback", ops->bring_down(context));
	wireless_guest_isolation_status_record(status,
		"detach guest AP bridge membership during rollback",
		ops->detach_bridge(context));
	wireless_guest_isolation_status_record(status,
		"verify guest AP is down after rollback", ops->verify_down(context));

	return wireless_guest_isolation_status_result(status);
}

#endif /* WIRELESS_GUEST_ISOLATION_POLICY_H */
