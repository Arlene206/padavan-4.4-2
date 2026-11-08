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
	puts("DNSMasq restart policy: PASS");
	return 0;
}
