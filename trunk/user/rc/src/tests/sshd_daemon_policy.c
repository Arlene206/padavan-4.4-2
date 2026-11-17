#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "sshd_daemon_policy.h"

static void test_process_name_selection_matches_inventory_backend(void)
{
	assert(strcmp(sshd_process_name_for_binaries(1, 1), "dropbear") == 0);
	assert(strcmp(sshd_process_name_for_binaries(1, 0), "dropbear") == 0);
	assert(strcmp(sshd_process_name_for_binaries(0, 1), "sshd") == 0);
	assert(sshd_process_name_for_binaries(0, 0) == NULL);
}

static void test_direct_start_requires_confirmed_absence(void)
{
	assert(sshd_start_allowed(PROCESS_PRESENCE_ABSENT));
	assert(!sshd_start_allowed(PROCESS_PRESENCE_RUNNING));
	assert(!sshd_start_allowed(PROCESS_PRESENCE_UNKNOWN));
	assert(!sshd_start_allowed((enum process_presence)9));
}

static void test_restart_initial_inventory_gates_name_based_stop(void)
{
	assert(sshd_restart_before_state_allows_stop(PROCESS_PRESENCE_ABSENT));
	assert(sshd_restart_before_state_allows_stop(PROCESS_PRESENCE_RUNNING));
	assert(!sshd_restart_before_state_allows_stop(PROCESS_PRESENCE_UNKNOWN));
	assert(!sshd_restart_before_state_allows_stop((enum process_presence)9));

	assert(!sshd_restart_requires_stop(PROCESS_PRESENCE_ABSENT));
	assert(sshd_restart_requires_stop(PROCESS_PRESENCE_RUNNING));
	assert(!sshd_restart_requires_stop(PROCESS_PRESENCE_UNKNOWN));
	assert(!sshd_restart_requires_stop((enum process_presence)9));
}

static void test_detached_start_requires_running_child(void)
{
	assert(sshd_start_postcondition(0, PROCESS_PRESENCE_RUNNING) == 0);
	assert(sshd_start_postcondition(0, PROCESS_PRESENCE_ABSENT) == -1);
	assert(sshd_start_postcondition(0, PROCESS_PRESENCE_UNKNOWN) == -1);
	assert(sshd_start_postcondition(0, (enum process_presence)9) == -1);
	assert(sshd_start_postcondition(EACCES, PROCESS_PRESENCE_RUNNING) == EACCES);
}

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

static void test_firewall_failure_requires_listener_shutdown(void)
{
	assert(!sshd_firewall_failure_requires_stop(0, PROCESS_PRESENCE_RUNNING));
	assert(!sshd_firewall_failure_requires_stop(0, PROCESS_PRESENCE_UNKNOWN));
	assert(!sshd_firewall_failure_requires_stop(1, PROCESS_PRESENCE_ABSENT));
	assert(sshd_firewall_failure_requires_stop(1, PROCESS_PRESENCE_RUNNING));
	assert(sshd_firewall_failure_requires_stop(-1, PROCESS_PRESENCE_UNKNOWN));
	assert(sshd_firewall_failure_requires_stop(1, (enum process_presence)9));
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
	test_process_name_selection_matches_inventory_backend();
	test_direct_start_requires_confirmed_absence();
	test_restart_initial_inventory_gates_name_based_stop();
	test_detached_start_requires_running_child();
	test_stop_failure_blocks_start();
	test_running_or_unknown_blocks_start();
	test_confirmed_absence_allows_start();
	test_firewall_refresh_policy();
	test_firewall_failure_requires_listener_shutdown();
	puts("sshd tri-state stop-before-start and firewall reconciliation policy: PASS");
	return 0;
}
