#ifndef PADAVAN_DNSMASQ_RESTART_POLICY_H
#define PADAVAN_DNSMASQ_RESTART_POLICY_H

#include <errno.h>
#include "process_inventory.h"

/* Separate preparation from stop/launch so failed refreshes cannot take down
 * an already healthy daemon, and disabled service configurations stop cleanly. */
/* Only a complete ABSENT snapshot authorizes configuration and launch. */
static int
dnsmasq_start_policy_preflight(enum process_presence daemon_presence)
{
	if (daemon_presence == PROCESS_PRESENCE_ABSENT)
		return 0;
	if (daemon_presence == PROCESS_PRESENCE_RUNNING)
		return EBUSY;
	return EIO;
}

/* After stop, only a complete ABSENT snapshot permits a replacement launch. */
static int
dnsmasq_restart_policy_stop_status(int stop_status,
				   enum process_presence daemon_presence)
{
	if (stop_status != 0)
		return stop_status;
	if (daemon_presence == PROCESS_PRESENCE_ABSENT)
		return 0;
	if (daemon_presence == PROCESS_PRESENCE_RUNNING)
		return EBUSY;
	return EIO;
}

/* Static ARP bindings are removed only after the daemon stop is confirmed. */
static int
dnsmasq_stop_policy_should_clear_arp(int stop_status)
{
	return stop_status == 0;
}

/* A successful daemon stop is not fully successful if required ARP cleanup
 * failed. Preserve the original stop error when both operations fail. */
static int
dnsmasq_stop_policy_cleanup_status(int stop_status, int arp_cleanup_status)
{
	return stop_status != 0 ? stop_status : arp_cleanup_status;
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
enum dnsmasq_ddns_cache_purge_action {
	DNSMASQ_DDNS_CACHE_PURGE_SKIP = 0,
	DNSMASQ_DDNS_CACHE_PURGE_RESTART,
	DNSMASQ_DDNS_CACHE_PURGE_UNKNOWN
};

/* An opportunistic DDNS cache purge must not interpret UNKNOWN as ABSENT or
 * authorize a DNSMasq operation on an incomplete process snapshot. */
static enum dnsmasq_ddns_cache_purge_action
dnsmasq_ddns_cache_purge_action_for_presence(enum process_presence presence)
{
	if (presence == PROCESS_PRESENCE_RUNNING)
		return DNSMASQ_DDNS_CACHE_PURGE_RESTART;
	if (presence == PROCESS_PRESENCE_ABSENT)
		return DNSMASQ_DDNS_CACHE_PURGE_SKIP;
	return DNSMASQ_DDNS_CACHE_PURGE_UNKNOWN;
}

static int
dnsmasq_start_policy_status(int launch_status,
			    enum process_presence daemon_presence)
{
	if (launch_status != 0)
		return launch_status;
	if (daemon_presence == PROCESS_PRESENCE_RUNNING)
		return 0;
	if (daemon_presence == PROCESS_PRESENCE_ABSENT)
		return ESRCH;
	return EIO;
}

/* A failed detached launch must treat UNKNOWN as possibly still active. */
static int
dnsmasq_failed_start_may_leave_daemon(enum process_presence state)
{
	return state != PROCESS_PRESENCE_ABSENT;
}

#endif /* PADAVAN_DNSMASQ_RESTART_POLICY_H */
