#ifndef UDHCPC_STOP_POLICY_H
#define UDHCPC_STOP_POLICY_H

#include <errno.h>

/* Allow the DHCP RELEASE/deconfig callback to complete before termination. */
#define UDHCPC_STOP_DECONFIG_GRACE_US 300000U

static inline int
udhcpc_stop_release_wait_required(int signal_status)
{
	return signal_status == 0;
}

static inline int
udhcpc_stop_release_status(int signal_status, int signal_errno,
                           int wait_status)
{
	if (signal_status == 0)
		return wait_status;
	if (signal_errno == ENOENT || signal_errno == ESRCH)
		return 0;
	return -1;
}

#endif
