#ifndef RC_LOGGER_START_POLICY_H
#define RC_LOGGER_START_POLICY_H

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

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

/* Checked NVRAM reads use 1=present, 0=missing, -1=backend/close error. */
static inline int
rc_logger_nvram_read_status(int api_result, int saved_errno)
{
	if (api_result == 0 || api_result == 1)
		return 0;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	return EIO;
}

/* Copy an optional address before the next transaction read invalidates cache. */
static inline int
rc_logger_nvram_address_status(int api_result, int saved_errno,
			       const char *value, char *destination,
			       size_t destination_size)
{
	size_t length;
	int status;

	if (!destination || destination_size == 0)
		return EINVAL;
	destination[0] = '\0';
	status = rc_logger_nvram_read_status(api_result, saved_errno);
	if (status != 0)
		return status;
	if (api_result == 0)
		return value ? EIO : 0;
	if (!value)
		return EIO;

	length = strlen(value);
	/* Oversized/invalid legacy values simply disable remote logging. */
	if (length >= destination_size)
		return 0;
	memcpy(destination, value, length + 1);
	return 0;
}

/*
 * Match nvram_safe_get_int("log_port", 514, 1, 65535) for readable values,
 * but keep a backend fault distinct so remote logging is reported degraded.
 * atoi() accepted a decimal prefix; strtol() preserves that compatibility
 * while safely mapping overflow and out-of-range values to the 514 default.
 */
static inline int
rc_logger_nvram_port_status(int api_result, int saved_errno,
			    const char *value, int default_port,
			    int minimum, int maximum, int *port)
{
	char *end;
	long parsed;
	int status;

	if (!port || minimum > maximum || default_port < minimum ||
	    default_port > maximum)
		return EINVAL;
	*port = default_port;
	status = rc_logger_nvram_read_status(api_result, saved_errno);
	if (status != 0)
		return status;
	if (api_result == 0)
		return value ? EIO : 0;
	if (!value)
		return EIO;

	errno = 0;
	parsed = strtol(value, &end, 10);
	if (end == value || errno == ERANGE || parsed < minimum || parsed > maximum)
		return 0;
	if (errno != 0)
		return errno;

	*port = (int)parsed;
	return 0;
}

#endif
