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

	assert(dnsmasq_start_policy_should_launch(0));
	assert(!dnsmasq_start_policy_should_launch(1));
	assert(dnsmasq_start_policy_status(EIO, 0) == EIO);
	assert(dnsmasq_start_policy_status(0, 1) == 0);
	assert(dnsmasq_start_policy_status(0, 0) == ESRCH);
	assert(dnsmasq_start_policy_status(0, 7) == 0);
	assert(dnsmasq_stop_policy_should_clear_arp(0));
	assert(!dnsmasq_stop_policy_should_clear_arp(EBUSY));

	puts("DNSMasq restart policy: PASS");
	return 0;
}
