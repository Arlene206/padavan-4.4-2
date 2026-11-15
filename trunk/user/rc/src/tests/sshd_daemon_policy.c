#include <assert.h>
#include <stdio.h>

#include "sshd_daemon_policy.h"

static void test_stop_failure_blocks_start(void)
{
	assert(!sshd_restart_can_start(-1, PROCESS_PRESENCE_ABSENT));
	assert(!sshd_restart_can_start(-1, PROCESS_PRESENCE_RUNNING));
	assert(!sshd_restart_can_start(-1, PROCESS_PRESENCE_UNKNOWN));
}

static void test_running_or_unknown_blocks_start(void)
{
	assert(!sshd_restart_can_start(0, PROCESS_PRESENCE_RUNNING));
	assert(!sshd_restart_can_start(0, PROCESS_PRESENCE_UNKNOWN));
	assert(!sshd_restart_can_start(0, (enum process_presence)9));
}

static void test_confirmed_absence_allows_start(void)
{
	assert(sshd_restart_can_start(0, PROCESS_PRESENCE_ABSENT));
}

static void test_firewall_refresh_policy(void)
{
	assert(!sshd_firewall_refresh_required(PROCESS_PRESENCE_ABSENT,
					       PROCESS_PRESENCE_ABSENT));
	assert(!sshd_firewall_refresh_required(PROCESS_PRESENCE_RUNNING,
					       PROCESS_PRESENCE_RUNNING));
	assert(sshd_firewall_refresh_required(PROCESS_PRESENCE_RUNNING,
					      PROCESS_PRESENCE_ABSENT));
	assert(sshd_firewall_refresh_required(PROCESS_PRESENCE_ABSENT,
					      PROCESS_PRESENCE_RUNNING));
	assert(sshd_firewall_refresh_required(PROCESS_PRESENCE_UNKNOWN,
					      PROCESS_PRESENCE_ABSENT));
	assert(sshd_firewall_refresh_required(PROCESS_PRESENCE_RUNNING,
					      PROCESS_PRESENCE_UNKNOWN));
}

int main(void)
{
	test_stop_failure_blocks_start();
	test_running_or_unknown_blocks_start();
	test_confirmed_absence_allows_start();
	test_firewall_refresh_policy();
	puts("sshd tri-state stop-before-start and firewall reconciliation policy: PASS");
	return 0;
}
