#ifndef RC_LOGGER_START_POLICY_H
#define RC_LOGGER_START_POLICY_H

/* Preserve the first failure while attempting both logger daemons. */
static inline int
rc_logger_start_status(int syslog_status, int klog_status)
{
	return syslog_status != 0 ? syslog_status : klog_status;
}

/*
 * The optional logger-settling interval occurs between syslogd and klogd.
 * Keep that error in the same first-failure order without skipping klogd.
 */
static inline int
rc_logger_start_status_with_wait(int syslog_status, int wait_status,
				 int klog_status)
{
	if (syslog_status != 0)
		return syslog_status;
	if (wait_status != 0)
		return wait_status;
	return klog_status;
}

#endif
