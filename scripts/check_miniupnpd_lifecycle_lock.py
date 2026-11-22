#!/usr/bin/env python3
"""Verify MiniUPnP lifecycle serialization and fail-closed startup ordering."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
SERVICES = (RC / "services_ex.c").read_text()
LOCK = (RC / "miniupnpd_lifecycle_lock.h").read_text()
RETRY = (RC / "lifecycle_lock_retry_policy.h").read_text()
INADYN_LOCK = (RC / "inadyn_lifecycle_lock.h").read_text()
DNSMASQ_LOCK = (RC / "dnsmasq_lifecycle_lock.h").read_text()
VPN_SERVER_LOCK = (RC / "vpn_server_lifecycle_lock.h").read_text()
RC_H = (RC / "rc.h").read_text()
DAEMON_POLICY = (RC / "miniupnpd_daemon_policy.h").read_text()
DAEMON_TEST = (RC / "tests/miniupnpd_daemon_policy_test.c").read_text()
WANIF_POLICY = (RC / "miniupnpd_wanif_change_policy.h").read_text()
WANIF_TEST = (RC / "tests/miniupnpd_wanif_change_policy.c").read_text()


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:position]
    raise AssertionError(f"unterminated function: {signature}")


def compact(source: str) -> str:
    return " ".join(source.split())


assert '#include "miniupnpd_lifecycle_lock.h"' in SERVICES
assert "MINIUPNPD_LIFECYCLE_LOCK_FILE" in LOCK
assert "O_NOFOLLOW | O_CLOEXEC" in LOCK
assert "st.st_nlink != 1" in LOCK
assert "(st.st_mode & 077) != 0" in LOCK
assert "F_SETLK" in LOCK and "errno = EBUSY" in LOCK
assert "RC_LIFECYCLE_LOCK_MAX_INTERRUPTS 8U" in RETRY
retry_policy = body(RETRY, "rc_lifecycle_lock_retry_interrupted(unsigned int *interruptions)")
assert "if (!interruptions)" in retry_policy
assert "*interruptions >= RC_LIFECYCLE_LOCK_MAX_INTERRUPTS" in retry_policy
assert "++*interruptions;" in retry_policy
wait_header = (RC / "service_wait_policy.h").read_text()
wait_body = body(wait_header, "service_wait_seconds_with(unsigned int seconds")
assert "while (seconds > 0)" in wait_body
assert "if (remaining > seconds)" in wait_body
assert "seconds = remaining;" in wait_body
assert '#include "service_wait_policy.h"' in LOCK
lock_acquire = body(LOCK, "miniupnpd_lifecycle_lock_acquire_path(const char *path")
assert "unsigned int interruptions = 0;" in lock_acquire
assert "rc_lifecycle_lock_retry_interrupted(&interruptions)" in lock_acquire
assert lock_acquire.index("if (errno == EINTR)") < lock_acquire.index(
    "rc_lifecycle_lock_retry_interrupted(&interruptions)"
) < lock_acquire.index("attempt++")
assert "interruptions = 0;" in lock_acquire
assert "service_wait_seconds(1)" in lock_acquire
lock_release = body(LOCK, "miniupnpd_lifecycle_lock_release(int fd)")
assert "rc_lifecycle_lock_retry_interrupted(&interruptions)" in lock_release
for source, signature in (
    (INADYN_LOCK, "inadyn_lifecycle_lock_release(int fd)"),
    (DNSMASQ_LOCK, "dnsmasq_lifecycle_lock_release(int fd)"),
    (VPN_SERVER_LOCK, "vpn_server_lifecycle_lock_release(int fd)"),
):
    assert '#include "lifecycle_lock_retry_policy.h"' in source
    release_body = body(source, signature)
    assert "unsigned int interruptions = 0;" in release_body
    assert "rc_lifecycle_lock_retry_interrupted(&interruptions)" in release_body

# EINTR must preserve the configured contention wait; never burn retries instantly.
inadyn_acquire = body(INADYN_LOCK, "inadyn_lifecycle_lock_acquire_path(const char *path")
assert "service_wait_seconds(1)" in inadyn_acquire
assert "if (!interrupted && service_wait_seconds(1) != 0)" not in inadyn_acquire
dnsmasq_acquire = body(DNSMASQ_LOCK, "dnsmasq_lifecycle_lock_acquire_path(const char *path")
assert "service_wait_seconds(1)" in dnsmasq_acquire
assert "sleep(1)" not in dnsmasq_acquire
vpn_acquire = body(VPN_SERVER_LOCK, "vpn_server_lifecycle_lock_acquire_path(const char *path")
assert "service_wait_seconds(1)" in vpn_acquire
assert "sleep(1)" not in vpn_acquire

start_wait = body(SERVICES, "service_start_wait_one_second(const char *service_name")
assert "service_wait_seconds(1)" in start_wait
mini_confirm = body(SERVICES, "miniupnpd_confirm_started(int launch_status)")
assert "int wait_status;" in mini_confirm
assert "wait_status = service_start_wait_one_second(\"miniupnpd\")" in mini_confirm
for signature, label in (
    ("dnsmasq_confirm_started(int launch_status)", '"DNSMasq"'),
    ("miniupnpd_confirm_started(int launch_status)", '"miniupnpd"'),
    ("inadyn_confirm_started(int launch_status)", '"Inadyn"'),
):
    confirm = body(SERVICES, signature)
    assert label in confirm
    assert "sleep(1)" not in confirm
test_source = (RC / "tests/miniupnpd_lifecycle_lock.c").read_text()
assert "test_interrupted_lock_retries_are_bounded" in test_source
assert "RC_LIFECYCLE_LOCK_MAX_INTERRUPTS" in test_source
assert "fixture.remaining[0] = 3" in test_source
assert "fixture.requested[2] == 1" in test_source
assert "test_interrupted_service_wait_retries_remaining_time" in test_source
assert "unlink(" not in LOCK, "the stable lock inode must never be unlinked"

finish = body(SERVICES, "miniupnpd_lifecycle_lock_finish(int lock_fd")
assert "miniupnpd_lifecycle_lock_release(lock_fd)" in finish
assert "operation_status != 0 ? operation_status : lock_errno" in finish

for public, internal, operation in (
    ("start_upnp(void)", "start_upnp_locked()", "start"),
    ("stop_upnp(void)", "stop_upnp_locked()", "stop"),
    ("check_upnp_wanif_changed(char *wan_ifname)", "check_upnp_wanif_changed_locked(wan_ifname)", "WAN check"),
    ("update_upnp(void)", "update_upnp_locked()", "update"),
):
    entry = body(SERVICES, "int\n" + public)
    assert entry.index("miniupnpd_lifecycle_lock_acquire()") < entry.index(internal)
    assert entry.index(internal) < entry.index("miniupnpd_lifecycle_lock_finish(")
    assert f'"{operation}"' in entry

restart = body(SERVICES, "int\nrestart_upnp(void)")
assert restart.index("miniupnpd_lifecycle_lock_acquire()") < restart.index(
    "rc_upnp_restart_run(stop_upnp_locked, restart_firewall)"
)
assert restart.index("rc_upnp_restart_run(stop_upnp_locked, restart_firewall)") < restart.index(
    'miniupnpd_lifecycle_lock_finish(lock_fd, ret, "restart")'
)

start = compact(body(SERVICES, "start_upnp_locked(void)"))
assert "miniupnpd_process_presence(&scan_error)" in start
assert "miniupnpd_config_publish(" in start
assert "miniupnpd launch aborted: daemon appeared during configuration" in start
assert start.index("miniupnpd_config_publish(") < start.index(
    "miniupnpd launch aborted: daemon appeared during configuration"
) < start.index('eval("/usr/bin/miniupnpd")')
assert start.count("miniupnpd_process_presence(&scan_error)") >= 2
assert "miniupnpd_confirm_started(launch_status)" in start
assert "if (start_status != 0)" in start
assert "start_errno = errno > 0 ? errno :" in start
assert "errno = 0;" in start
assert start.rindex("errno = 0;") < start.index('eval("/usr/bin/miniupnpd")')
assert "rollback_status = stop_upnp_locked()" in start
assert "rollback_presence = miniupnpd_process_presence(&rollback_scan_error)" in start
assert "miniupnpd_failed_start_may_leave_daemon(rollback_presence)" in start
assert "errno = start_errno;" in start
assert "ret = start_status;" in start
assert start.index("miniupnpd_confirm_started(launch_status)") < start.index("stop_upnp_locked()")
assert start.index("stop_upnp_locked()") < start.index("miniupnpd_process_presence(&rollback_scan_error)")
assert start.index("miniupnpd_process_presence(&rollback_scan_error)") < start.index("miniupnpd_failed_start_may_leave_daemon")
assert start.index("miniupnpd_failed_start_may_leave_daemon") < start.index("errno = start_errno;")
assert "miniupnpd_failed_start_may_leave_daemon(enum process_presence state)" in DAEMON_POLICY
assert "return state != PROCESS_PRESENCE_ABSENT;" in DAEMON_POLICY
assert "miniupnpd_test_failed_start_rollback_policy" in DAEMON_TEST

stop = body(SERVICES, "static int\nstop_upnp_locked(void)")
assert stop.index("kill_services(svcs, 3, 1)") < stop.index(
    "miniupnpd_process_presence(&scan_error)"
)
check = body(SERVICES, "check_upnp_wanif_changed_locked(char *wan_ifname)")
assert "get_wan_unit_value_copy_status(" in check
assert '"ifname_t"' in check
assert "wan_ifname_copy(" in check
assert "miniupnpd_wanif_change_decide(" in check
assert "if (should_stop)" in check
assert "keeping daemon active" in check
assert 'get_wan_unit_value(0, "ifname_t")' not in check
assert check.index("get_wan_unit_value_copy_status(") < check.index("if (should_stop)")
assert "if (read_status != 0)" in WANIF_POLICY
assert "*should_stop = 0;" in WANIF_POLICY
assert "requested[0] == '\\0'" in WANIF_POLICY
assert "active[0] == '\\0'" in WANIF_POLICY
for token in ("test_backend_error_never_requests_stop",
              "test_missing_or_invalid_identity_never_requests_stop",
              "test_only_verified_difference_requests_stop",
              "test_missing_output_is_rejected"):
    assert token in WANIF_TEST

update = body(SERVICES, "update_upnp_locked(void)")
assert "return start_upnp_locked();" in update
assert "return stop_upnp_locked();" in update
for public_call in ("start_upnp()", "stop_upnp()", "check_upnp_wanif_changed(", "update_upnp()"):
    assert public_call not in update, f"nested public lifecycle lock call: {public_call}"

for declaration in (
    "int start_upnp(void);", "int stop_upnp(void);",
    "int check_upnp_wanif_changed(char *wan_ifname);",
    "int update_upnp(void);", "int restart_upnp(void);",
):
    assert declaration in RC_H

print("MiniUPnP cross-process lifecycle lock and startup/stop ordering contract: PASS")
