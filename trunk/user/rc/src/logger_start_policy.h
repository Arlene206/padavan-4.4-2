#ifndef RC_LOGGER_START_POLICY_H
#define RC_LOGGER_START_POLICY_H

/* Preserve the first logger startup failure while attempting both daemons. */
static inline int
rc_logger_start_status(int syslog_status, int klog_status)
{
	return syslog_status != 0 ? syslog_status : klog_status;
}

#endif
