#ifndef PADAVAN_XUPNPD_DAEMON_POLICY_H
#define PADAVAN_XUPNPD_DAEMON_POLICY_H

#include "detached_daemon_start_policy.h"

typedef detached_daemon_presence_scan_fn xupnpd_presence_scan_fn;
typedef detached_daemon_presence_wait_fn xupnpd_presence_wait_fn;

/* Preserve the XUPnPD-specific interface while sharing detached-daemon policy. */
static inline int
xupnpd_daemon_start_postcondition_with_ops(
	int launcher_status, xupnpd_presence_scan_fn scan,
	xupnpd_presence_wait_fn wait, void *context,
	unsigned int initial_wait_seconds,
	unsigned int retry_wait_seconds, unsigned int attempts)
{
	return detached_daemon_start_postcondition_with_ops(
		launcher_status, scan, wait, context, initial_wait_seconds,
		retry_wait_seconds, attempts);
}

#endif /* PADAVAN_XUPNPD_DAEMON_POLICY_H */
