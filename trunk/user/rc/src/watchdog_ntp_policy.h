#ifndef WATCHDOG_NTP_POLICY_H
#define WATCHDOG_NTP_POLICY_H

#include <errno.h>

/* Do not overlap one-shot clock synchronization with an ntpd that survived
 * the bounded TERM/KILL lifecycle stop. */
static inline int
watchdog_ntp_refresh_should_launch(int previous_stop_status)
{
	return previous_stop_status == 0;
}

static inline int
watchdog_time_notification_error(int signal_status, int signal_errno,
				 int start_status, int start_errno)
{
	if (signal_status == 0)
		return 0;
	if (signal_errno == ENOENT || signal_errno == ESRCH) {
		if (start_status == 0)
			return 0;
		if (start_errno > 0)
			return start_errno;
		if (start_status > 0)
			return start_status;
		return EIO;
	}
	if (signal_errno > 0)
		return signal_errno;
	if (signal_status > 0)
		return signal_status;
	return EIO;
}

#endif /* WATCHDOG_NTP_POLICY_H */
