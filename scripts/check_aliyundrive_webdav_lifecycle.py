#!/usr/bin/env python3
"""Source and integration contracts for AliyunDrive WebDAV lifecycle safety."""

from pathlib import Path
import hashlib
import json
import re
import struct
import sys
import tempfile

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
ARCH_CONFIG_PATH = ROOT / "trunk/config.arch"
UCLIBC_CONFIG_PATH = ROOT / "trunk/configs/boards/uclibc-mipsel.config"
BOARD_TEMPLATES_PATH = ROOT / "trunk/configs/templates"
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
binary_test = (ROOT / "scripts/test_aliyundrive_webdav_binary.py").read_text()
busybox_config = BUSYBOX_CONFIG_PATH.read_text()
flock_source = FLOCK_SOURCE_PATH.read_text()
arch_config = ARCH_CONFIG_PATH.read_text()
uclibc_config = UCLIBC_CONFIG_PATH.read_text()
daemon_binary = DAEMON_BINARY_PATH.read_bytes()
daemon_sha256 = hashlib.sha256(daemon_binary).hexdigest()
env_names = (b"REFRESH_TOKEN", b"WEBDAV_AUTH_USER", b"WEBDAV_AUTH_PASSWORD")
capability_report_path = Path(tempfile.gettempdir()) / "padavan-aliyundrive-webdav-capability.json"
try:
    capability_report = json.loads(capability_report_path.read_text(encoding="utf-8"))
except (OSError, json.JSONDecodeError):
    capability_report = {}
required_env_names = [name.decode("ascii") for name in env_names]
reported_env_names = capability_report.get("credential_env", [])
missing_env_names = [name for name in required_env_names if name not in reported_env_names]
daemon_has_env_credentials = (
    capability_report.get("binary_sha256") == daemon_sha256
    and reported_env_names == required_env_names
)
daemon_elf = "unknown"
daemon_elf_class = None
daemon_elf_data = None
daemon_elf_type = None
daemon_elf_machine = None
daemon_elf_flags = None
daemon_elf_arch = "unknown"
daemon_elf_abi = "unknown"
daemon_elf_interpreter = "not found"
if daemon_binary[:4] == bytes((0x7F, 0x45, 0x4C, 0x46)) and len(daemon_binary) >= 52:
    daemon_elf_class = daemon_binary[4]
    daemon_elf_data = daemon_binary[5]
    byte_order = "<" if daemon_elf_data == 1 else ">"
    elf_class_name = {1: "ELF32", 2: "ELF64"}.get(daemon_elf_class, "ELF?")
    elf_data_name = {1: "little-endian", 2: "big-endian"}.get(daemon_elf_data, "unknown-endian")
    if daemon_elf_class == 1 and daemon_elf_data in (1, 2):
        daemon_elf_type, daemon_elf_machine = struct.unpack_from(f"{byte_order}HH", daemon_binary, 16)
        daemon_elf_flags = struct.unpack_from(f"{byte_order}I", daemon_binary, 36)[0]
        arch_code = daemon_elf_flags & 0xF0000000
        daemon_elf_arch = {
            0x00000000: "MIPS-I", 0x10000000: "MIPS-II",
            0x20000000: "MIPS-III", 0x30000000: "MIPS-IV",
            0x40000000: "MIPS-V", 0x50000000: "MIPS32",
            0x60000000: "MIPS64", 0x70000000: "MIPS32R2",
            0x80000000: "MIPS64R2", 0x90000000: "MIPS32R6",
            0xA0000000: "MIPS64R6",
        }.get(arch_code, f"MIPS-arch-flags-0x{arch_code:08x}")
        abi_code = daemon_elf_flags & 0x0000F000
        daemon_elf_abi = {
            0x00000000: "unspecified", 0x00001000: "O32",
            0x00002000: "O64", 0x00003000: "EABI32",
            0x00004000: "EABI64",
        }.get(abi_code, f"ABI-flags-0x{abi_code:04x}")
        if daemon_elf_flags & 0x20:
            daemon_elf_abi += "+ABI2"
        phoff = struct.unpack_from(f"{byte_order}I", daemon_binary, 28)[0]
        phentsize, phnum = struct.unpack_from(f"{byte_order}HH", daemon_binary, 42)
        if phentsize >= 32 and phoff + phentsize * phnum <= len(daemon_binary):
            for index in range(phnum):
                phdr = phoff + index * phentsize
                p_type = struct.unpack_from(f"{byte_order}I", daemon_binary, phdr)[0]
                if p_type == 3:  # PT_INTERP
                    p_offset = struct.unpack_from(f"{byte_order}I", daemon_binary, phdr + 4)[0]
                    p_filesz = struct.unpack_from(f"{byte_order}I", daemon_binary, phdr + 16)[0]
                    if p_offset + p_filesz <= len(daemon_binary):
                        daemon_elf_interpreter = daemon_binary[p_offset:p_offset + p_filesz].split(bytes((0,)), 1)[0].decode("ascii", "replace")
                    break
    daemon_elf = (
        f"{elf_class_name} {elf_data_name} e_machine={daemon_elf_machine} "
        f"e_type={daemon_elf_type} e_flags={f'0x{daemon_elf_flags:08x}' if daemon_elf_flags is not None else 'unknown'} "
        f"arch={daemon_elf_arch} abi={daemon_elf_abi} interpreter={daemon_elf_interpreter}"
    )

daemon_ascii_strings = [item.decode("ascii", "replace") for item in re.findall(rb"[\x20-\x7e]{4,}", daemon_binary)]
daemon_relevant_strings = [
    item for item in daemon_ascii_strings
    if re.search(r"aliyundrive-webdav|refresh-token|auth-user|auth-password|domain-id|workdir|read-only|no-trash|version|WEBDAV_AUTH|REFRESH_TOKEN", item, re.IGNORECASE)
][:60]

board_templates = sorted(BOARD_TEMPLATES_PATH.glob("*.config"))
board_template_settings = {}
for config_path in board_templates:
    config_text = config_path.read_text()
    match = re.search(r"(?m)^CONFIG_FIRMWARE_INCLUDE_ALDRIVER=(y|n)$", config_text)
    board_template_settings[config_path.stem] = match.group(1) if match else "unset"
board_aldriver_enabled = sorted(name for name, value in board_template_settings.items() if value == "y")
board_aldriver_disabled = sorted(name for name, value in board_template_settings.items() if value == "n")
board_aldriver_unset = sorted(name for name, value in board_template_settings.items() if value == "unset")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"AliyunDrive WebDAV lifecycle contract: FAIL: {message}")


require(daemon_has_env_credentials,
        "the QEMU --help capability report must match this exact binary SHA and advertise all three credential environment names")


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
workdir = function_body(launcher, "ald_secure_workdir() {")

require('-L "$ALD_WORK_DIR"' in workdir and
        'stat -c %u "$ALD_WORK_DIR"' in workdir and 'id -u' in workdir and
        'chmod 700 "$ALD_WORK_DIR"' in workdir and
        '-L "$ald_token_file"' in workdir and
        'stat -c %u "$ald_token_file"' in workdir and
        'stat -c %h "$ald_token_file"' in workdir and
        'chmod 600 "$ald_token_file"' in workdir and
        'return 1' in workdir,
        "refresh-token workdir must reject symlinks, foreign ownership and hardlinks, and enforce private modes")

require('ald_secure_workdir || return 1' in start and
        start.index('ald_secure_workdir') < start.index('"$ALD_BIN"') and
        '--workdir "$ALD_WORK_DIR"' in start and
        'umask 077' in start and 'RUST_LOG=aliyundrive_webdav=warn' in start and
        'exec 9>&-' in start,
        "daemon launch must protect token storage, suppress token-bearing INFO logs, and close inherited lifecycle lock")

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

if daemon_has_env_credentials:
    require('set -- -I' in start and
            '"$ALD_BIN" "$@" --host "$host" --root "$root" -p "$port"' in start and
            'REFRESH_TOKEN="$refresh_token"' in start and
            'WEBDAV_AUTH_USER="$auth_user"' in start and
            'WEBDAV_AUTH_PASSWORD="$auth_password"' in start and
            '-r "$refresh_token"' not in start and
            '-U "$auth_user"' not in start and
            '-W "$auth_password"' not in start,
            "a daemon with compiled env support must receive secrets through env, never argv")
else:
    require('-r "$refresh_token" -U "$auth_user" -W "$auth_password"' in start and
            'REFRESH_TOKEN="$refresh_token"' not in start and
            'WEBDAV_AUTH_USER="$auth_user"' not in start and
            'WEBDAV_AUTH_PASSWORD="$auth_password"' not in start,
            "do not pass unsupported credential environment variables to the bundled daemon")
    print("WARNING: bundled daemon has no compiled credential environment interface; secrets remain in argv until a compatible binary is integrated.")

require('-S "$read_buf_size" --cache-size "$cache_size"' in start and
        '--cache-ttl "$cache_ttl" --workdir "$ALD_WORK_DIR"' in start,
        "buffer/cache/workdir settings must retain their individually quoted argv values")

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

require('CONFIG_STAT=y' in busybox_config and 'CONFIG_FEATURE_STAT_FORMAT=y' in busybox_config and
        'CONFIG_ID=y' in busybox_config and 'CONFIG_CHMOD=y' in busybox_config and
        'CONFIG_MKDIR=y' in busybox_config and 'CONFIG_FLOCK=y' in busybox_config and
        'nonblock' in flock_source and 'unlock' in flock_source and
        'OPT_n' in flock_source and 'OPT_u' in flock_source,
        "target BusyBox must include flock with nonblocking and unlock support")

require('exec 9>&-\n        RUST_LOG=aliyundrive_webdav=warn' in start and
        '>/dev/null 2>&1 9>&-\n      ) &' in start,
        "the background child must close the lifecycle lock before daemon exec")

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
    "FAIL: credentials were not passed as three exact environment values",
    "FAIL: a credential value leaked into daemon argv",
    "FAIL: lock contention allowed a lifecycle transition",
    "FAIL: secure workdir creation failed",
    "FAIL: existing workdir hardening failed",
    "FAIL: existing token path was not reduced to private modes",
    "FAIL: multiply-linked refresh-token file was accepted",
    "FAIL: symlink workdir was accepted",
    "FAIL: symlink refresh-token file was accepted",
    "FAIL: daemon did not inherit the private workdir umask",
    "FAIL: startup timeout was reported successful",
    "AliyunDrive WebDAV private token workdir and umask regression: PASS",
    "AliyunDrive WebDAV serialized lifecycle lock and startup readiness regression: PASS",
    "AliyunDrive WebDAV bounded stop and process-name signaling regression: PASS",
    "AliyunDrive WebDAV duplicate-start guard and argv integrity regression: PASS",
):
    require(marker in test, f"host regression missing case or result marker: {marker}")

paths = [
    "SESSION_HANDOFF.md",
    "trunk/user/aliyundrive-webdav/aliyundrive-webdav",
    "REFACTOR_ROADMAP.md",
    "dev_log.md",
    ".github/workflows/aliyundrive-webdav-lifecycle.yml",
    "scripts/check_aliyundrive_webdav_lifecycle.py",
    "scripts/test_aliyundrive_webdav_binary.py",
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

require(daemon_elf_class == 1 and daemon_elf_data == 1 and daemon_elf_machine == 8,
        "bundled daemon must be ELF32 little-endian MIPS for the Padavan toolchain")
require("CPUFLAGS = -mips32r2 -march=mips32r2" in arch_config and "HOST_TARGET = mipsel-linux" in arch_config,
        "firmware compiler target changed; re-evaluate bundled daemon compatibility")
require('TARGET_ARCH="mips"' in uclibc_config and "TARGET_ARCH_BITS=32" in uclibc_config and "CONFIG_MIPS_O32_ABI=y" in uclibc_config and "ARCH_LITTLE_ENDIAN=y" in uclibc_config,
        "uClibc target is no longer the expected MIPS O32 little-endian ABI")
require(board_templates, "board template scan found no .config files")
require("qemu-user" in workflow and "scripts/test_aliyundrive_webdav_binary.py" in workflow,
        "workflow must install QEMU and run the bundled daemon CLI probe")
for option in ("--version", "--help", "--domain-id", "--workdir", "--no-trash", "--read-only"):
    require(option in binary_test, f"QEMU probe missing {option} coverage")
require('f"[env: {name}=]"' in binary_test and "json.dumps(report" in binary_test
        and "binary_sha256" in binary_test,
        "QEMU probe must verify all credential env annotations and publish a SHA-bound capability report")
print(f"AliyunDrive WebDAV bundled ELF: {daemon_elf}; size={len(daemon_binary)}; sha256={daemon_sha256}")
print(f"AliyunDrive WebDAV credential env support (QEMU help, SHA-bound): {daemon_has_env_credentials}; missing={missing_env_names}")
print(f"AliyunDrive WebDAV QEMU-verified CLI version: {capability_report.get('version', 'unavailable')}")
print(f"AliyunDrive WebDAV board templates: total={len(board_templates)}; enabled={board_aldriver_enabled}; disabled={board_aldriver_disabled}; unset={board_aldriver_unset}")
for item in daemon_relevant_strings:
    print(f"  ELF string: {item[:180]}")
print("AliyunDrive WebDAV bounded lifecycle, credential privacy and RC integration contracts: PASS")
