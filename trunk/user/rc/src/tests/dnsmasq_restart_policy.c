#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "dnsmasq_restart_policy.h"

int
main(void)
{
	assert(dnsmasq_restart_policy_action(1, EIO, 1) == DNSMASQ_RESTART_KEEP_RUNNING);
	assert(dnsmasq_restart_policy_action(1, ENOSPC, 0) == DNSMASQ_RESTART_KEEP_RUNNING);
	assert(dnsmasq_restart_policy_action(0, EIO, 1) == DNSMASQ_RESTART_ABORT);
	assert(dnsmasq_restart_policy_action(0, ENOENT, 0) == DNSMASQ_RESTART_ABORT);
	assert(dnsmasq_restart_policy_action(0, 0, 0) == DNSMASQ_RESTART_STOP_ONLY);
	assert(dnsmasq_restart_policy_action(1, 0, 0) == DNSMASQ_RESTART_STOP_ONLY);
	assert(dnsmasq_restart_policy_action(0, 0, 1) == DNSMASQ_RESTART_STOP_AND_START);
	assert(dnsmasq_restart_policy_action(1, 0, 1) == DNSMASQ_RESTART_STOP_AND_START);

	assert(dnsmasq_start_policy_preflight(PROCESS_PRESENCE_ABSENT) == 0);
	assert(dnsmasq_start_policy_preflight(PROCESS_PRESENCE_RUNNING) == EBUSY);
	assert(dnsmasq_start_policy_preflight(PROCESS_PRESENCE_UNKNOWN) == EIO);
	assert(dnsmasq_start_policy_preflight((enum process_presence)7) == EIO);
	assert(dnsmasq_restart_policy_stop_status(0, PROCESS_PRESENCE_ABSENT) == 0);
	assert(dnsmasq_restart_policy_stop_status(0, PROCESS_PRESENCE_RUNNING) == EBUSY);
	assert(dnsmasq_restart_policy_stop_status(0, PROCESS_PRESENCE_UNKNOWN) == EIO);
	assert(dnsmasq_restart_policy_stop_status(0, (enum process_presence)7) == EIO);
	assert(dnsmasq_restart_policy_stop_status(EIO, PROCESS_PRESENCE_ABSENT) == EIO);
	assert(dnsmasq_start_policy_status(EIO, 0) == EIO);
	assert(dnsmasq_start_policy_status(0, PROCESS_PRESENCE_RUNNING) == 0);
	assert(dnsmasq_start_policy_status(0, PROCESS_PRESENCE_ABSENT) == ESRCH);
	assert(dnsmasq_start_policy_status(0, PROCESS_PRESENCE_UNKNOWN) == EIO);
	assert(dnsmasq_start_policy_status(0, (enum process_presence)7) == EIO);
	assert(dnsmasq_start_policy_status(EIO, PROCESS_PRESENCE_RUNNING) == EIO);
	assert(!dnsmasq_failed_start_may_leave_daemon(PROCESS_PRESENCE_ABSENT));
	assert(dnsmasq_failed_start_may_leave_daemon(PROCESS_PRESENCE_RUNNING));
	assert(dnsmasq_failed_start_may_leave_daemon(PROCESS_PRESENCE_UNKNOWN));
	assert(dnsmasq_failed_start_may_leave_daemon((enum process_presence)7));
	assert(dnsmasq_stop_policy_should_clear_arp(0));
	assert(!dnsmasq_stop_policy_should_clear_arp(EBUSY));
	assert(dnsmasq_stop_policy_cleanup_status(0, 0) == 0);
	assert(dnsmasq_stop_policy_cleanup_status(0, EIO) == EIO);
	assert(dnsmasq_stop_policy_cleanup_status(EBUSY, 0) == EBUSY);
	assert(dnsmasq_stop_policy_cleanup_status(EBUSY, EIO) == EBUSY);

	assert(dnsmasq_ddns_cache_purge_action_for_presence(PROCESS_PRESENCE_RUNNING) ==
	       DNSMASQ_DDNS_CACHE_PURGE_RESTART);
	assert(dnsmasq_ddns_cache_purge_action_for_presence(PROCESS_PRESENCE_ABSENT) ==
	       DNSMASQ_DDNS_CACHE_PURGE_SKIP);
	assert(dnsmasq_ddns_cache_purge_action_for_presence(PROCESS_PRESENCE_UNKNOWN) ==
	       DNSMASQ_DDNS_CACHE_PURGE_UNKNOWN);
	assert(dnsmasq_ddns_cache_purge_action_for_presence((enum process_presence)7) ==
	       DNSMASQ_DDNS_CACHE_PURGE_UNKNOWN);

	puts("DNSMasq restart policy: PASS");
	return 0;
}
