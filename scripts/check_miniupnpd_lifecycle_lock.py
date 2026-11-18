#!/usr/bin/env python3
"""Verify MiniUPnP lifecycle serialization and fail-closed startup ordering."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
SERVICES = (RC / "services_ex.c").read_text()
LOCK = (RC / "miniupnpd_lifecycle_lock.h").read_text()
RC_H = (RC / "rc.h").read_text()


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
wait_header = (RC / "service_wait_policy.h").read_text()
wait_body = body(wait_header, "service_wait_seconds_with(unsigned int seconds")
assert "while (seconds > 0)" in wait_body
assert "if (remaining > seconds)" in wait_body
assert "seconds = remaining;" in wait_body
assert '#include "service_wait_policy.h"' in LOCK
lock_acquire = body(LOCK, "miniupnpd_lifecycle_lock_acquire_path(const char *path")
assert "if (errno == EINTR)" in lock_acquire
assert lock_acquire.index("if (errno == EINTR)") < lock_acquire.index("attempt++")
assert "service_wait_seconds(1)" in lock_acquire
start_wait = body(SERVICES, "service_start_wait_one_second(const char *service_name")
assert "service_wait_seconds(1)" in start_wait
for signature, label in (
    ("dnsmasq_confirm_started(int launch_status)", '"DNSMasq"'),
    ("miniupnpd_confirm_started(int launch_status)", '"miniupnpd"'),
    ("inadyn_confirm_started(int launch_status)", '"Inadyn"'),
):
    confirm = body(SERVICES, signature)
    assert label in confirm
    assert "sleep(1)" not in confirm
test_source = (RC / "tests/miniupnpd_lifecycle_lock.c").read_text()
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
assert "miniupnpd_confirm_started(ret)" in start

stop = body(SERVICES, "stop_upnp_locked(void)")
assert stop.index("kill_services(svcs, 3, 1)") < stop.index(
    "miniupnpd_process_presence(&scan_error)"
)
check = body(SERVICES, "check_upnp_wanif_changed_locked(char *wan_ifname)")
assert "return stop_upnp_locked();" in check
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
