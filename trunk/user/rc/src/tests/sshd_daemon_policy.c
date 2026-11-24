#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "firewall_nvram_match_policy.h"
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

static void
test_sshd_firewall_config_read_preserves_match_and_backend_errors(void)
{
	int matches = 77;

	assert(firewall_nvram_match_status(1, 0, "1", "1", 0,
					   &matches) == 0);
	assert(matches == 1);
	assert(firewall_nvram_match_status(1, 0, "01", "1", 0,
					   &matches) == 0);
	assert(matches == 0);
	assert(firewall_nvram_match_status(1, 0, " 1", "1", 0,
					   &matches) == 0);
	assert(matches == 0);
	assert(firewall_nvram_match_status(0, 0, NULL, "1", 0,
					   &matches) == 0);
	assert(matches == 0);
	matches = 7;
	assert(firewall_nvram_match_status(-1, EACCES, NULL, "1", 0,
					   &matches) == EACCES);
	assert(matches == 7);
	assert(firewall_nvram_match_status(0, 0, "1", "1", 0,
					   &matches) == EIO);
	assert(matches == 7);
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

static void
test_failed_start_rollback_reports_unconfirmed_absence(void)
{
	assert(!sshd_failed_start_preserve_running_state(PROCESS_PRESENCE_ABSENT));
	assert(sshd_failed_start_preserve_running_state(PROCESS_PRESENCE_RUNNING));
	assert(sshd_failed_start_preserve_running_state(PROCESS_PRESENCE_UNKNOWN));
	assert(sshd_failed_start_preserve_running_state((enum process_presence)9));
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


static void test_checked_sshd_enable_mode_read(void)
{
    int mode = 77;

    assert(sshd_nvram_mode_status(1, 0, "1", &mode) == 0);
    assert(mode == 1);
    assert(sshd_nvram_mode_status(1, 0, "2", &mode) == 0);
    assert(mode == 2);
    assert(sshd_nvram_mode_status(1, 0, "0", &mode) == 0);
    assert(mode == 0);
    assert(sshd_nvram_mode_status(0, 0, NULL, &mode) == 0);
    assert(mode == 0);
    assert(sshd_nvram_mode_status(1, 0, "", &mode) == 0);
    assert(mode == 0);
    assert(sshd_nvram_mode_status(1, 0, " 2 \t", &mode) == 0);
    assert(mode == 2);

    mode = 77;
    assert(sshd_nvram_mode_status(-1, EACCES, NULL, &mode) == EACCES);
    assert(mode == 77);
    assert(sshd_nvram_mode_status(-1, 0, NULL, &mode) == EIO);
    assert(mode == 77);
    assert(sshd_nvram_mode_status(0, 0, "1", &mode) == EIO);
    assert(mode == 77);
    assert(sshd_nvram_mode_status(1, 0, NULL, &mode) == EIO);
    assert(mode == 77);
    assert(sshd_nvram_mode_status(2, 0, "1", &mode) == EIO);
    assert(mode == 77);
    assert(sshd_nvram_mode_status(1, 0, "2junk", &mode) == EINVAL);
    assert(mode == 77);
    assert(sshd_nvram_mode_status(1, 0, "9999999999999999999",
                                  &mode) == ERANGE);
    assert(mode == 77);
    assert(sshd_nvram_mode_status(1, 0, "1", NULL) == EINVAL);
}

static void
test_sshd_enable_mode_rejects_unsupported_values(void)
{
	int mode = 77;

	assert(sshd_nvram_mode_status(1, 0, "-1", &mode) == EINVAL);
	assert(mode == 77);
	assert(sshd_nvram_mode_status(1, 0, "3", &mode) == EINVAL);
	assert(mode == 77);
	assert(sshd_nvram_mode_status(1, 0, "-2147483648", &mode) == EINVAL);
	assert(mode == 77);
	assert(sshd_nvram_mode_status(1, 0, "2147483647", &mode) == EINVAL);
	assert(mode == 77);
}

int main(void)
{
	test_checked_sshd_enable_mode_read();
	test_sshd_enable_mode_rejects_unsupported_values();
	test_process_name_selection_matches_inventory_backend();
	test_direct_start_requires_confirmed_absence();
	test_restart_initial_inventory_gates_name_based_stop();
	test_detached_start_requires_running_child();
	test_stop_failure_blocks_start();
	test_running_or_unknown_blocks_start();
	test_confirmed_absence_allows_start();
	test_failed_start_rollback_reports_unconfirmed_absence();
	test_firewall_refresh_policy();
	test_sshd_firewall_config_read_preserves_match_and_backend_errors();
	test_firewall_failure_requires_listener_shutdown();
	puts("sshd tri-state stop-before-start and firewall reconciliation policy: PASS");
	return 0;
}
