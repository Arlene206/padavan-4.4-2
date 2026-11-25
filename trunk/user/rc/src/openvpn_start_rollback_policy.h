#ifndef OPENVPN_START_ROLLBACK_POLICY_H
#define OPENVPN_START_ROLLBACK_POLICY_H

#include <errno.h>
#include <process_inventory.h>

#define OPENVPN_START_ROLLBACK_CONFIRM_SCANS 2
#define OPENVPN_START_ROLLBACK_CONFIRM_WAIT_SECONDS 1

typedef enum process_presence (*openvpn_start_rollback_scan_fn)(
	void *context, int *scan_error);
typedef int (*openvpn_start_rollback_wait_fn)(void *context,
					      unsigned int seconds);

/* Two clean full-inventory snapshots, separated in time, are required before
 * rollback may infer that no detached OpenVPN child remains. */
static inline enum process_presence
openvpn_start_rollback_confirm_absent_with_ops(
	openvpn_start_rollback_scan_fn scan,
	openvpn_start_rollback_wait_fn wait,
	void *context, int *scan_error_out)
{
	unsigned int attempt;

	if (scan_error_out)
		*scan_error_out = 0;
	if (!scan || !wait) {
		errno = EINVAL;
		if (scan_error_out)
			*scan_error_out = EINVAL;
		return PROCESS_PRESENCE_UNKNOWN;
	}

	for (attempt = 0; attempt < OPENVPN_START_ROLLBACK_CONFIRM_SCANS;
	     ++attempt) {
		int scan_error = 0;
		enum process_presence state = scan(context, &scan_error);

		if (state == PROCESS_PRESENCE_ABSENT) {
			if (attempt + 1 < OPENVPN_START_ROLLBACK_CONFIRM_SCANS) {
				errno = 0;
				if (wait(context,
					 OPENVPN_START_ROLLBACK_CONFIRM_WAIT_SECONDS) != 0) {
					int saved_errno = errno ? errno : EINTR;
					errno = saved_errno;
					if (scan_error_out)
						*scan_error_out = saved_errno;
					return PROCESS_PRESENCE_UNKNOWN;
				}
			}
			continue;
		}
		if (state == PROCESS_PRESENCE_UNKNOWN) {
			int saved_errno = scan_error > 0 ? scan_error : EIO;
			errno = saved_errno;
			if (scan_error_out)
				*scan_error_out = saved_errno;
			return PROCESS_PRESENCE_UNKNOWN;
		}
		if (state == PROCESS_PRESENCE_RUNNING)
			return PROCESS_PRESENCE_RUNNING;

		errno = EIO;
		if (scan_error_out)
			*scan_error_out = EIO;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	return PROCESS_PRESENCE_ABSENT;
}

/* Both conditions are mandatory: never remove pre-existing resources, and
 * never remove newly-created resources while a daemon might still be alive. */
static inline int
openvpn_start_rollback_cleanup_owned(enum process_presence process_state,
				     int resource_created)
{
	return resource_created == 1 &&
		process_state == PROCESS_PRESENCE_ABSENT;
}

#endif /* OPENVPN_START_ROLLBACK_POLICY_H */
