#ifndef PADAVAN_VPN_SERVER_CALLBACK_POLICY_H
#define PADAVAN_VPN_SERVER_CALLBACK_POLICY_H

#include <errno.h>
#include <limits.h>
#include <sys/types.h>

/* Parse a pppd PID without accepting signs, whitespace, partial numbers or
 * values that cannot be represented by pid_t. */
static inline int
vpn_server_callback_pid_parse(const char *text, pid_t *pid_out)
{
	const unsigned char *p;
	unsigned long value = 0;
	pid_t pid;

	if (!text || !*text || !pid_out) {
		errno = EINVAL;
		return -1;
	}
	for (p = (const unsigned char *)text; *p; ++p) {
		unsigned int digit;

		if (*p < '0' || *p > '9') {
			errno = EINVAL;
			return -1;
		}
		digit = (unsigned int)(*p - '0');
		if (value > (ULONG_MAX - digit) / 10UL) {
			errno = ERANGE;
			return -1;
		}
		value = value * 10UL + digit;
	}
	pid = (pid_t)value;
	if (pid <= (pid_t)1 || (unsigned long)pid != value) {
		errno = ERANGE;
		return -1;
	}
	*pid_out = pid;
	return 0;
}

static inline int
vpn_server_callback_pids_match(pid_t callback_pid, pid_t parent_pid,
			       pid_t pidfile_pid)
{
	return callback_pid > (pid_t)1 && parent_pid > (pid_t)1 &&
	       pidfile_pid > (pid_t)1 && callback_pid == parent_pid &&
	       parent_pid == pidfile_pid;
}

#endif /* PADAVAN_VPN_SERVER_CALLBACK_POLICY_H */
