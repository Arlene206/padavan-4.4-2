#ifndef OPENVPN_PROCESS_STOP_POLICY_H
#define OPENVPN_PROCESS_STOP_POLICY_H

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <sys/types.h>
#include <process_inventory.h>

/* OpenVPN's writepid directive stores a numeric PID, not a start_time. */
static inline int
openvpn_process_stop_parse_pid(const char *text, size_t length, pid_t *pid)
{
	unsigned long value = 0;
	size_t i;

	if (!text || !pid || length == 0) {
		errno = EINVAL;
		return -1;
	}
	if (text[length - 1] == '\n')
		length--;
	if (length == 0) {
		errno = EINVAL;
		return -1;
	}
	for (i = 0; i < length; i++) {
		unsigned int digit;
		if (text[i] < '0' || text[i] > '9') {
			errno = EINVAL;
			return -1;
		}
		digit = (unsigned int)(text[i] - '0');
		if (value > ((unsigned long)INT_MAX - digit) / 10UL) {
			errno = ERANGE;
			return -1;
		}
		value = value * 10UL + digit;
	}
	if (value <= 1UL) {
		errno = EINVAL;
		return -1;
	}
	*pid = (pid_t)value;
	return 0;
}

/* Never turn RUNNING or UNKNOWN into a successful teardown postcondition. */
static inline int
openvpn_process_stop_result(enum process_presence presence, int operation_status)
{
	if (operation_status != 0)
		return operation_status;
	return presence == PROCESS_PRESENCE_ABSENT ? 0 : -1;
}

#endif
