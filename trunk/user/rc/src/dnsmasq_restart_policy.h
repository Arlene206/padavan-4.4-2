#ifndef PADAVAN_DNSMASQ_RESTART_POLICY_H
#define PADAVAN_DNSMASQ_RESTART_POLICY_H

#include <errno.h>

/* Separate preparation from stop/launch so failed refreshes cannot take down
 * an already healthy daemon, and disabled service configurations stop cleanly. */
/* Start only when no previous daemon is present. */
static int
dnsmasq_start_policy_should_launch(int daemon_running)
{
	return !daemon_running;
}

/* Static ARP bindings are removed only after the daemon stop is confirmed. */
static int
dnsmasq_stop_policy_should_clear_arp(int stop_status)
{
	return stop_status == 0;
}

enum dnsmasq_restart_action {
	DNSMASQ_RESTART_ABORT = 0,
	DNSMASQ_RESTART_KEEP_RUNNING,
	DNSMASQ_RESTART_STOP_ONLY,
	DNSMASQ_RESTART_STOP_AND_START
};

static enum dnsmasq_restart_action
dnsmasq_restart_policy_action(int was_running, int prepare_status, int service_enabled)
{
	if (prepare_status != 0)
		return was_running ? DNSMASQ_RESTART_KEEP_RUNNING : DNSMASQ_RESTART_ABORT;

	return service_enabled ? DNSMASQ_RESTART_STOP_AND_START : DNSMASQ_RESTART_STOP_ONLY;
}

/* A successful daemonization parent exit is not proof that the child remains
 * available. Require a process-presence postcondition after bounded polling. */
static int
dnsmasq_start_policy_status(int launch_status, int daemon_running)
{
	if (launch_status != 0)
		return launch_status;
	return daemon_running ? 0 : ESRCH;
}

#endif /* PADAVAN_DNSMASQ_RESTART_POLICY_H */
