#!/usr/bin/env python3
"""Source and integration contracts for AliyunDrive WebDAV lifecycle safety."""

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
LAUNCHER_PATH = ROOT / "trunk/user/aliyundrive-webdav/aliyundrive-webdav.sh"
TEST_PATH = ROOT / "trunk/user/aliyundrive-webdav/tests/aliyundrive_webdav_lifecycle.sh"
PACKAGE_PATH = ROOT / "trunk/user/aliyundrive-webdav/Makefile"
USER_MAKEFILE_PATH = ROOT / "trunk/user/Makefile"
SERVICES_PATH = ROOT / "trunk/user/rc/src/services.c"
RC_PATH = ROOT / "trunk/user/rc/src/rc.c"
VARIABLES_PATH = ROOT / "trunk/user/httpd/src/variables.c"
WEB_EX_PATH = ROOT / "trunk/user/httpd/src/web_ex.c"
WORKFLOW_PATH = ROOT / ".github/workflows/aliyundrive-webdav-lifecycle.yml"
BUSYBOX_CONFIG_PATH = ROOT / "trunk/configs/boards/busybox.config"
FLOCK_SOURCE_PATH = ROOT / "trunk/user/busybox/busybox-1.24.x/util-linux/flock.c"
DAEMON_BINARY_PATH = ROOT / "trunk/user/aliyundrive-webdav/aliyundrive-webdav"

launcher = LAUNCHER_PATH.read_text()
test = TEST_PATH.read_text()
package = PACKAGE_PATH.read_text()
user_makefile = USER_MAKEFILE_PATH.read_text()
services = SERVICES_PATH.read_text()
rc = RC_PATH.read_text()
variables = VARIABLES_PATH.read_text()
web_ex = WEB_EX_PATH.read_text()
workflow = WORKFLOW_PATH.read_text()
busybox_config = BUSYBOX_CONFIG_PATH.read_text()
flock_source = FLOCK_SOURCE_PATH.read_text()
daemon_binary = DAEMON_BINARY_PATH.read_bytes()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"AliyunDrive WebDAV lifecycle contract: FAIL: {message}")


def function_body(source: str, signature: str) -> str:
    start = source.find(signature)
    require(start >= 0, f"missing function {signature}")
    opening = source.find("{", start)
    require(opening >= 0, f"missing opening brace for {signature}")
    closing = source.find("\n}", opening + 1)
    require(closing >= 0, f"missing closing brace for {signature}")
    return source[opening + 1:closing]


wait_stopped = function_body(launcher, "ald_wait_stopped() {")
wait_started = function_body(launcher, "ald_wait_started() {")
lock = function_body(launcher, "ald_run_locked() {")
stop = function_body(launcher, "ald_kill_unlocked() {")
start = function_body(launcher, "ald_start_unlocked() {")
stop_wrapper = function_body(launcher, "stop_ald() {")

require('while [ "$ald_wait" -lt "$ALD_STOP_WAIT_SECONDS" ]; do' in wait_stopped and
        'sleep 1' in wait_stopped and 'pidof "$NAME"' in wait_stopped and
        'return 1' in wait_stopped,
        "daemon stop polling must be bounded and confirm process absence")

require('if ! pidof "$NAME" >/dev/null 2>&1; then' in stop and
        'killall "$NAME"' in stop and
        'if ald_wait_stopped; then' in stop and
        'killall -9 "$NAME"' in stop and
        stop.index('killall "$NAME"') < stop.index('ald_wait_stopped') <
        stop.index('killall -9 "$NAME"') < stop.rindex('ald_wait_stopped') and
        'return 1' in stop,
        "stop must use name-based TERM, bounded wait, fresh name-based KILL and final absence check")

require('aliyundrive_process=' not in stop and
        'kill -9 "$aliyundrive_process"' not in launcher and
        'killall -9 "$NAME"' in stop,
        "stop must never signal a cached PID that may have been reused")

require('if pidof "$NAME" >/dev/null 2>&1; then' in start and
        start.index('if pidof "$NAME"') < start.index('refresh_token=') and
        '跳过重复启动' in start,
        "duplicate start must exit before reading credentials or launching another daemon")

require('set -- -I' in start and
        '"$ALD_BIN" "$@" --host "$host" --root "$root" -p "$port"' in start and
        'REFRESH_TOKEN="$refresh_token"' in start and
        'WEBDAV_AUTH_USER="$auth_user"' in start and
        'WEBDAV_AUTH_PASSWORD="$auth_password"' in start and
        '-r "$refresh_token"' not in start and
        '-U "$auth_user"' not in start and
        '-W "$auth_password"' not in start and
        '-S "$read_buf_size" --cache-size "$cache_size"' in start and
        '--cache-ttl "$cache_ttl" --workdir "/tmp/$NAME"' in start,
        "refresh token and WebDAV credentials must use supported environment variables, never argv")

require(all(name in daemon_binary for name in (
            b"REFRESH_TOKEN", b"WEBDAV_AUTH_USER", b"WEBDAV_AUTH_PASSWORD",
        )),
        "the exact bundled daemon binary must contain the three compiled Clap environment-variable names")

require('set -- "$@" --domain-id "$domain_id"' in start and
        'set -- "$@" --no-trash' in start and
        'set -- "$@" --read-only' in start,
        "domain and optional read-only/trash flags must retain the original option policy")


require('while [ "$ald_wait" -lt "$ALD_START_WAIT_SECONDS" ]; do' in wait_started and
        'pidof "$NAME"' in wait_started and 'sleep 1' in wait_started and
        'return 1' in wait_started and
        start.index('ald_wait_started') > start.index('"$ALD_BIN"'),
        "start must wait a bounded interval and fail unless the daemon becomes observable")

require('flock -n 9' in lock and 'flock -u 9' in lock and
        'exec 9>>"$ALD_LOCK_FILE"' in lock and 'exec 9>&-' in lock and
        'mkdir -p "$ald_lock_dir"' in lock and
        'ALD_LOCK_FILE' in launcher and
        'ald_run_locked ald_kill_unlocked' in launcher and
        'ald_run_locked ald_start_unlocked' in launcher and
        'ald_kill_unlocked' in start and
        'rm -f "$ALD_LOCK_FILE"' not in lock,
        "start/stop must share a nonblocking stable-inode lock without recursive locking")

require('CONFIG_FLOCK=y' in busybox_config and
        'nonblock' in flock_source and 'unlock' in flock_source and
        'OPT_n' in flock_source and 'OPT_u' in flock_source,
        "target BusyBox must include flock with nonblocking and unlock support")

require('>/dev/null 2>&1 9>&- &' in start,
        "the background daemon must not inherit and retain the lifecycle lock descriptor")

require('kill_ald' in stop_wrapper,
        "the public stop action must propagate the checked kill_ald result")

require('$(ROMFSINST) -p +x $(THISDIR)/aliyundrive-webdav.sh /usr/bin/aliyundrive-webdav.sh' in package and
        '$(ROMFSINST) -p +x $(THISDIR)/aliyundrive-webdav /usr/bin/aliyundrive-webdav' in package and
        'dir_$(CONFIG_FIRMWARE_INCLUDE_ALDRIVER)' in user_makefile,
        "ROMFS package installation and firmware configuration gate must remain intact")

require('eval("/usr/bin/aliyundrive-webdav.sh","stop")' in services and
        'eval("/usr/bin/aliyundrive-webdav.sh","start")' in services and
        'void restart_aldriver(void)' in services and
        'stop_aldriver();' in services and 'start_aldriver();' in services,
        "RC service wrapper must preserve stop/start/restart launcher wiring")

require('RCN_RESTART_ALDRIVER' in rc and 'restart_aldriver();' in rc and
        '#if defined(APP_ALDRIVER)' in rc,
        "RC notification dispatch must still reach the AliyunDrive restart wrapper")

ald_fields = (
    "aliyundrive_enable", "ald_refresh_token", "ald_auth_user",
    "ald_auth_password", "ald_read_buffer_size", "ald_cache_size",
    "ald_cache_ttl", "ald_host", "ald_port", "ald_root",
    "ald_domain_id", "ald_no_trash", "ald_read_only",
)
require(all(field in variables for field in ald_fields) and
        variables.count("EVM_RESTART_ALDRIVER") >= len(ald_fields) and
        "notify_rc(events_desc[i].notify_cmd)" in web_ex,
        "HTTPD settings must continue to notify RC when AliyunDrive configuration changes")

for marker in (
    "FAIL: graceful stop escalated or used a cached numeric PID",
    "FAIL: bounded TERM/KILL/absence sequence is incorrect",
    "FAIL: permanently running daemon was reported stopped",
    "FAIL: duplicate start launched or stopped an existing daemon",
    "FAIL: non-secret NVRAM values were split, globbed or reordered in daemon argv",
    "FAIL: secret environment values were missing, split or modified",
    "FAIL: cloud token or WebDAV password leaked into daemon argv",
    "AliyunDrive WebDAV refresh-token and auth secrets stay out of daemon argv: PASS",
    "FAIL: lock contention allowed a lifecycle transition",
    "FAIL: startup timeout was reported successful",
    "AliyunDrive WebDAV serialized lifecycle lock and startup readiness regression: PASS",
    "AliyunDrive WebDAV bounded stop and process-name signaling regression: PASS",
    "AliyunDrive WebDAV duplicate-start guard and argv integrity regression: PASS",
):
    require(marker in test, f"host regression missing case or result marker: {marker}")

paths = [
    "SESSION_HANDOFF.md",
    "REFACTOR_ROADMAP.md",
    "dev_log.md",
    ".github/workflows/aliyundrive-webdav-lifecycle.yml",
    "scripts/check_aliyundrive_webdav_lifecycle.py",
    "trunk/user/aliyundrive-webdav/aliyundrive-webdav",
    "trunk/user/aliyundrive-webdav/aliyundrive-webdav.sh",
    "trunk/user/aliyundrive-webdav/tests/aliyundrive_webdav_lifecycle.sh",
    "trunk/user/aliyundrive-webdav/Makefile",
    "trunk/user/Makefile",
    "trunk/user/rc/src/services.c",
    "trunk/user/rc/src/rc.h",
    "trunk/user/rc/src/rc.c",
    "trunk/user/httpd/src/variables.c",
    "trunk/user/httpd/src/web_ex.c",
    "trunk/configs/boards/busybox.config",
    "trunk/user/busybox/busybox-1.24.x/util-linux/flock.c",
]
push = workflow.split("  push:", 1)[1].split("  pull_request:", 1)[0]
pull = workflow.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for path in paths:
    token = f'      - "{path}"'
    require(push.count(token) == 1 and pull.count(token) == 1,
            f"{path} must appear exactly once in push and pull_request path filters")

print("AliyunDrive WebDAV bounded lifecycle, argv integrity and RC integration contracts: PASS")
