#!/usr/bin/env python3
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SCUT_SCRIPT = ROOT / "trunk/user/scutclient/scripts/scutclient.sh"
MENTOHUST_SCRIPT = ROOT / "trunk/user/mentohust/scripts/mentohust.sh"
PROCESS_LIFECYCLE_LIB = ROOT / "trunk/user/shared/scripts/process_lifecycle.sh"
PROCESS_LIFECYCLE_TEST = ROOT / "trunk/user/shared/tests/process_lifecycle.sh"
SHARED_MAKEFILE = ROOT / "trunk/user/shared/Makefile"
VLM_SCRIPT = ROOT / "trunk/user/vlmcsd/vlmcsd.sh"
RC_SERVICES_SOURCE = ROOT / "trunk/user/rc/src/services.c"
RC_SOURCE = ROOT / "trunk/user/rc/src/rc.c"
IAPPD_SCRIPT = ROOT / "trunk/user/ralinkiappd/iappd.sh"
IAPPD_TEST = ROOT / "trunk/user/ralinkiappd/tests/iappd_lifecycle.sh"
IAPPD_MAKEFILE = ROOT / "trunk/user/ralinkiappd/Makefile"
PROCESS_WORKFLOW = ROOT / ".github/workflows/process-lifecycle-regression.yml"


def executable(path, content):
    path.write_text(content)
    path.chmod(0o755)


def mock_bin(directory):
    directory.mkdir()
    executable(directory / "logger", """#!/bin/sh
printf '%s\\n' "$*" >> "$MOCK_LOG"
exit 0
""")
    return directory


def run(command, env):
    return subprocess.run(
        command, env=env, text=True, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE, timeout=10, check=False
    )


def extract_c_function_body(source, signature):
    """Return one C function body while ignoring braces in comments and literals."""
    signature_start = source.index(signature)
    opening = source.index("{", signature_start)
    depth = 0
    state = "code"
    quote = ""
    index = opening

    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""

        if state == "code":
            if char == "/" and next_char == "*":
                state = "block_comment"
                index += 2
                continue
            if char == "/" and next_char == "/":
                state = "line_comment"
                index += 2
                continue
            if char in ("'", '"'):
                quote = char
                state = "quoted"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[opening + 1:index]
        elif state == "quoted":
            if char == "\\":
                index += 2
                continue
            if char == quote:
                state = "code"
        elif state == "line_comment":
            if char == "\n":
                state = "code"
        elif state == "block_comment":
            if char == "*" and next_char == "/":
                state = "code"
                index += 2
                continue

        index += 1

    raise AssertionError(f"unterminated C function body: {signature}")


def check_dns_forwarder_rc_shutdown_registration():
    services = RC_SERVICES_SOURCE.read_text()
    start_body = extract_c_function_body(
        services, "start_services_once(int is_ap_mode)"
    )
    stop_body = extract_c_function_body(services, "stop_services(int stopall)")

    start_guard = start_body.index("#if defined(APP_DNSFORWARDER)")
    start_end = start_body.index("#endif", start_guard)
    start_block = start_body[start_guard:start_end]
    assert "start_dnsforwarder()" in start_block
    assert "rc_first_service_start_status(start_status, dnsforwarder_start_status)" in start_block

    stop_guard = stop_body.index("#if defined(APP_DNSFORWARDER)")
    stop_end = stop_body.index("#endif", stop_guard)
    stop_block = stop_body[stop_guard:stop_end]
    assert 'record_service_stop_status(&stop_status, "DNS Forwarder", stop_dnsforwarder());' in stop_block
    assert stop_body.index("return stop_status;") > stop_guard

    stop_wrapper = extract_c_function_body(services, "stop_dnsforwarder(void)")
    assert 'eval("/usr/bin/dns-forwarder.sh","stop")' in stop_wrapper

    rc_source = RC_SOURCE.read_text()
    shutdown_body = extract_c_function_body(rc_source, "shutdown_router(int level)")
    assert "service_stop_status = stop_services(use_halt);" in shutdown_body
    assert "service_stop_status == 0" in shutdown_body

    firmware_update_body = extract_c_function_body(rc_source, "flash_firmware(void)")
    assert "if (stop_services(0) != 0)" in firmware_update_body
    assert "firmware update aborted: checked services did not stop" in firmware_update_body
    print("DNS Forwarder RC shutdown registration and failure gates: PASS")


def check_ralink_iappd_rc_shutdown_registration():
    services = RC_SERVICES_SOURCE.read_text()
    start_body = extract_c_function_body(
        services, "start_services_once(int is_ap_mode)"
    )
    stop_body = extract_c_function_body(services, "stop_services(int stopall)")

    assert 'system("/usr/bin/iappd.sh restart")' in start_body
    assert 'system("/usr/bin/iappd.sh test")' in start_body

    stop_call = (
        'record_service_stop_status(&stop_status, "Ralink IAPP", '
        'system("/usr/bin/iappd.sh stop"));'
    )
    assert stop_call in " ".join(stop_body.split())
    assert stop_body.index("system(\"/usr/bin/iappd.sh stop\")") < stop_body.index(
        "return stop_status;"
    )

    makefile = IAPPD_MAKEFILE.read_text()
    assert "$(INSTALL_BIN) ./iappd.sh $(ROMFSDIR)/usr/bin/iappd.sh" in makefile
    script_test = IAPPD_TEST.read_text()
    for token in (
        "UNKNOWN inventory",
        "launcher failure",
        "failed killall",
        "stop postcondition false positive",
        "restart ran after failed stop",
    ):
        assert token in script_test, f"Ralink IAPP regression missing {token}"

    workflow = PROCESS_WORKFLOW.read_text()
    assert '"trunk/user/ralinkiappd/iappd.sh"' in workflow
    assert '"trunk/user/ralinkiappd/tests/iappd_lifecycle.sh"' in workflow
    assert "sh trunk/user/ralinkiappd/tests/iappd_lifecycle.sh" in workflow

    for path in (IAPPD_SCRIPT, IAPPD_TEST):
        syntax = run(["/bin/sh", "-n", str(path)], os.environ.copy())
        assert syntax.returncode == 0, syntax.stderr
    result = run(["/bin/sh", str(IAPPD_TEST)], os.environ.copy())
    assert result.returncode == 0, result.stderr
    assert "Ralink IAPP daemon lifecycle regressions: PASS" in result.stdout
    print("Ralink IAPP RC shutdown registration and lifecycle regression: PASS")


def check_vlmcsd():
    with tempfile.TemporaryDirectory(prefix="vlmcsd-start-") as tmp:
        base = Path(tmp)
        bin_dir = mock_bin(base / "bin")
        executable(bin_dir / "vlmcsd", """#!/bin/sh
exit "${MOCK_LAUNCH_STATUS:-0}"
""")
        log = base / "logger.log"
        env = os.environ.copy()
        env.update({
            "PATH": str(bin_dir) + os.pathsep + env.get("PATH", ""),
            "MOCK_LOG": str(log),
            "MOCK_LAUNCH_STATUS": "23",
        })
        failed = run(["/bin/sh", str(VLM_SCRIPT), "start"], env)
        assert failed.returncode == 23, (
            f"vlmcsd must propagate launcher status 23, got "
            f"{failed.returncode}: {failed.stderr}"
        )
        assert "launch failed (status=23)" in log.read_text()

        log.write_text("")
        env["MOCK_LAUNCH_STATUS"] = "0"
        succeeded = run(["/bin/sh", str(VLM_SCRIPT), "start"], env)
        assert succeeded.returncode == 0, succeeded.stderr
        assert "KMS Activation server is running." in log.read_text()


def check_scutclient():
    source = SCUT_SCRIPT.read_text()
    assert '. "$PROCESS_LIFECYCLE_LIB" || exit 127' in source
    assert 'pl_stop_named "$scutclient_exec" 3 2' in source
    assert 'pl_wait_started "$scutclient_exec" 1 4 1' in source
    with tempfile.TemporaryDirectory(prefix="scutclient-lifecycle-") as tmp:
        base = Path(tmp)
        bin_dir = mock_bin(base / "bin")
        executable(bin_dir / "nvram", """#!/bin/sh
if [ "$1" = get ]; then
    case "$2" in
        scutclient_debug|scutclient_skip_udp_hb) echo 0 ;;
        wan_dns1_x) echo 1.1.1.1 ;;
        wan_ifname) echo br0 ;;
        *) echo test ;;
    esac
fi
""")
        executable(bin_dir / "pidof", """#!/bin/sh
case "$(cat "$MOCK_STATE_FILE")" in
    running) echo 123; exit 0 ;;
    absent) exit 1 ;;
    unknown) exit 2 ;;
    *) exit 2 ;;
esac
""")
        executable(bin_dir / "start-stop-daemon", """#!/bin/sh
echo "$*" >> "$MOCK_START_LOG"
status="$MOCK_START_STATUS"
[ "$status" -eq 0 ] || exit "$status"
if [ -n "${MOCK_START_STATE:-}" ]; then
    echo "$MOCK_START_STATE" > "$MOCK_STATE_FILE"
elif [ "$MOCK_START_APPEARS" = 1 ]; then
    echo running > "$MOCK_STATE_FILE"
fi
exit 0
""")
        executable(bin_dir / "killall", """#!/bin/sh
echo "$*" >> "$MOCK_STOP_LOG"
echo "$MOCK_TERM_STATE" > "$MOCK_STATE_FILE"
exit "$MOCK_STOP_STATUS"
""")
        executable(bin_dir / "sleep", """#!/bin/sh
count=0
[ -r "$MOCK_SLEEP_COUNT_FILE" ] && count=$(cat "$MOCK_SLEEP_COUNT_FILE")
count=$((count + 1))
echo "$count" > "$MOCK_SLEEP_COUNT_FILE"
if [ "${MOCK_SLEEP_APPEARS_AFTER:-0}" -gt 0 ] && [ "$count" -ge "$MOCK_SLEEP_APPEARS_AFTER" ]; then
    echo running > "$MOCK_STATE_FILE"
fi
exit "$MOCK_SLEEP_STATUS"
""")
        executable(bin_dir / "bin_scutclient", """#!/bin/sh
echo logout >> "$MOCK_LOGOUT_LOG"
exit 0
""")
        state = base / "state"
        state.write_text("absent")
        start_log = base / "start.log"
        stop_log = base / "stop.log"
        logout_log = base / "logout.log"
        sleep_count = base / "sleep.count"
        sleep_count.write_text("0")
        log = base / "logger.log"
        env = os.environ.copy()
        env.update({
            "PATH": str(bin_dir) + os.pathsep + env.get("PATH", ""),
            "MOCK_LOG": str(log),
            "MOCK_STATE_FILE": str(state),
            "MOCK_START_LOG": str(start_log),
            "MOCK_STOP_LOG": str(stop_log),
            "MOCK_LOGOUT_LOG": str(logout_log),
            "MOCK_START_STATUS": "29",
            "MOCK_START_APPEARS": "1",
            "MOCK_STOP_STATUS": "47",
            "MOCK_TERM_STATE": "absent",
            "MOCK_SLEEP_STATUS": "0",
            "MOCK_SLEEP_COUNT_FILE": str(sleep_count),
            "MOCK_SLEEP_APPEARS_AFTER": "0",
            "PROCESS_LIFECYCLE_LIB": str(PROCESS_LIFECYCLE_LIB),
        })

        failed = run(["/bin/sh", str(SCUT_SCRIPT), "start"], env)
        assert failed.returncode == 29, (
            f"SCUT must propagate launcher status 29, got {failed.returncode}: {failed.stderr}"
        )
        assert "[FAILED]" in failed.stdout
        assert "Daemon launch failed (status=29)" in log.read_text()
        assert state.read_text().strip() == "absent"

        env["MOCK_START_STATUS"] = "0"
        started = run(["/bin/sh", str(SCUT_SCRIPT), "start"], env)
        assert started.returncode == 0, started.stderr
        assert state.read_text().strip() == "running"
        starts_before = len(start_log.read_text().splitlines())
        duplicate = run(["/bin/sh", str(SCUT_SCRIPT), "start"], env)
        assert duplicate.returncode == 0, duplicate.stderr
        assert len(start_log.read_text().splitlines()) == starts_before

        # A nonzero signal result is harmless when a fresh scan proves ABSENT.
        stopped = run(["/bin/sh", str(SCUT_SCRIPT), "stop"], env)
        assert stopped.returncode == 0, stopped.stderr
        assert state.read_text().strip() == "absent"
        assert len(stop_log.read_text().splitlines()) == 1
        assert len(logout_log.read_text().splitlines()) == 1

        # A zero background-fork status is not proof that the child survived.
        env["MOCK_START_APPEARS"] = "0"
        sleep_count.write_text("0")
        missing_child = run(["/bin/sh", str(SCUT_SCRIPT), "start"], env)
        assert missing_child.returncode == 1, f"SCUT accepted absent child: {missing_child.returncode}"
        assert "startup readiness timed out" in log.read_text()
        assert state.read_text().strip() == "absent"
        assert sleep_count.read_text().strip() == "4"
        sleep_count.write_text("0")
        env["MOCK_SLEEP_APPEARS_AFTER"] = "2"
        delayed_child = run(["/bin/sh", str(SCUT_SCRIPT), "start"], env)
        assert delayed_child.returncode == 0, delayed_child.stderr
        assert state.read_text().strip() == "running"
        assert sleep_count.read_text().strip() == "2"
        stopped_delayed = run(["/bin/sh", str(SCUT_SCRIPT), "stop"], env)
        assert stopped_delayed.returncode == 0, stopped_delayed.stderr
        assert state.read_text().strip() == "absent"
        assert len(logout_log.read_text().splitlines()) == 2
        env["MOCK_SLEEP_APPEARS_AFTER"] = "0"
        env["MOCK_START_STATE"] = "unknown"
        sleep_count.write_text("0")
        unknown_child = run(["/bin/sh", str(SCUT_SCRIPT), "start"], env)
        assert unknown_child.returncode == 2, f"SCUT accepted UNKNOWN child: {unknown_child.returncode}"
        assert "startup readiness could not be verified" in log.read_text()
        assert sleep_count.read_text().strip() == "1"
        del env["MOCK_START_STATE"]

        # UNKNOWN blocks start, stop, restart, signal and logoff.
        state.write_text("unknown")
        starts_before = len(start_log.read_text().splitlines())
        stops_before = len(stop_log.read_text().splitlines())
        for action in ("start", "stop", "restart"):
            result = run(["/bin/sh", str(SCUT_SCRIPT), action], env)
            assert result.returncode == 2, (
                f"SCUT {action} must reject UNKNOWN, got {result.returncode}"
            )
        assert len(start_log.read_text().splitlines()) == starts_before
        assert len(stop_log.read_text().splitlines()) == stops_before
        assert len(logout_log.read_text().splitlines()) == 2


IPERF_SCRIPT = ROOT / "trunk/user/iperf3/iperf3.sh"
IPERF_MAKEFILE = ROOT / "trunk/user/iperf3/Makefile"


def check_iperf3():
    # ROMFS is invoked with "make -C iperf3", where THISDIR is unset.
    # CURDIR is GNU make's defined current-directory variable for this submake.
    package_makefile = IPERF_MAKEFILE.read_text()
    expected_script_install = (
        "$(INSTALL_BIN) $(CURDIR)/iperf3.sh $(ROMFSDIR)/usr/bin/iperf3.sh"
    )
    assert expected_script_install in package_makefile
    assert "$(THISDIR)/iperf3.sh" not in package_makefile

    with tempfile.TemporaryDirectory(prefix="iperf3-lifecycle-") as tmp:
        base = Path(tmp)
        bin_dir = mock_bin(base / "bin")
        binary = base / "iperf3"
        executable(binary, "#!/bin/sh\nexit 0\n")
        state = base / "running"
        start_log = base / "start.log"
        executable(bin_dir / "pidof", """#!/bin/sh
[ -f "$MOCK_STATE_FILE" ] || exit 1
echo 123
""")
        executable(bin_dir / "sleep", "#!/bin/sh\nexit 0\n")
        executable(bin_dir / "start-stop-daemon", """#!/bin/sh
printf '%s\\n' "$*" >> "$MOCK_START_LOG"
status="${MOCK_START_STATUS:-0}"
[ "$status" -eq 0 ] || exit "$status"
[ "${MOCK_START_APPEARS:-1}" = 1 ] && : > "$MOCK_STATE_FILE"
exit 0
""")
        executable(bin_dir / "killall", """#!/bin/sh
status="${MOCK_STOP_STATUS:-0}"
[ "$status" -eq 0 ] || exit "$status"
rm -f "$MOCK_STATE_FILE"
exit 0
""")
        log = base / "logger.log"
        env = os.environ.copy()
        env.update({
            "PATH": str(bin_dir) + os.pathsep + env.get("PATH", ""),
            "IPERF3_BIN": str(binary),
            "MOCK_STATE_FILE": str(state),
            "MOCK_START_LOG": str(start_log),
            "MOCK_LOG": str(log),
            "MOCK_START_STATUS": "31",
            "MOCK_START_APPEARS": "1",
            "MOCK_STOP_STATUS": "0",
        })

        failed = run(["/bin/sh", str(IPERF_SCRIPT), "start"], env)
        assert failed.returncode == 31, (
            f"iperf3 must propagate launcher status 31, got "
            f"{failed.returncode}: {failed.stderr}"
        )
        assert "server launcher failed (status=31)" in log.read_text()
        assert not state.exists()

        env["MOCK_START_STATUS"] = "0"
        started = run(["/bin/sh", str(IPERF_SCRIPT), "start"], env)
        assert started.returncode == 0, started.stderr
        assert state.exists()
        assert "server started" in log.read_text()
        starts_before = start_log.read_text().count("-S -b")
        env["MOCK_START_STATUS"] = "31"
        duplicate = run(["/bin/sh", str(IPERF_SCRIPT), "start"], env)
        assert duplicate.returncode == 0, duplicate.stderr
        assert start_log.read_text().count("-S -b") == starts_before, (
            "repeated start must not launch a second iperf3 process"
        )

        stopped = run(["/bin/sh", str(IPERF_SCRIPT), "stop"], env)
        assert stopped.returncode == 0, stopped.stderr
        assert not state.exists()

        env["MOCK_START_STATUS"] = "0"
        env["MOCK_START_APPEARS"] = "0"
        missing = run(["/bin/sh", str(IPERF_SCRIPT), "start"], env)
        assert missing.returncode == 1, missing.stderr
        assert "no server process appeared" in log.read_text()

        state.write_text("running")
        env["MOCK_STOP_STATUS"] = "41"
        stop_failed = run(["/bin/sh", str(IPERF_SCRIPT), "stop"], env)
        assert stop_failed.returncode == 41, (
            f"iperf3 must propagate a failed stop signal while still running, "
            f"got {stop_failed.returncode}"
        )
        assert "server stop signal failed (status=41)" in log.read_text()

        # Missing lifecycle tooling is UNKNOWN, never a successful no-op.
        missing_bin = base / "missing-bin"
        missing_bin.mkdir()
        executable(missing_bin / "logger", """#!/bin/sh
echo "$*" >> "$MOCK_LOG"
exit 0
""")
        missing_env = env.copy()
        missing_env["PATH"] = str(missing_bin)
        missing_tools = run(["/bin/sh", str(IPERF_SCRIPT), "start"], missing_env)
        assert missing_tools.returncode == 127, (
            f"iperf3 must fail closed if pidof is missing, got {missing_tools.returncode}"
        )
        assert "pidof is unavailable" in log.read_text()




MENTOHUST_SCRIPT = ROOT / "trunk/user/mentohust/scripts/mentohust.sh"


def check_mentohust():
    source = MENTOHUST_SCRIPT.read_text()
    assert '. "$PROCESS_LIFECYCLE_LIB" || exit 127' in source
    assert 'pl_stop_named "$mentohust_exec" 3 2' in source
    assert 'pl_wait_started "$mentohust_exec" 1 4 1' in source
    with tempfile.TemporaryDirectory(prefix="mentohust-lifecycle-") as tmp:
        base = Path(tmp)
        bin_dir = mock_bin(base / "bin")
        executable(bin_dir / "pidof", """#!/bin/sh
case "$(cat "$MOCK_STATE_FILE")" in
    running) echo 456; exit 0 ;;
    absent) exit 1 ;;
    unknown) exit 2 ;;
    *) exit 2 ;;
esac
""")
        executable(bin_dir / "start-stop-daemon", """#!/bin/sh
echo "$*" >> "$MOCK_START_LOG"
status="$MOCK_START_STATUS"
[ "$status" -eq 0 ] || exit "$status"
if [ -n "${MOCK_START_STATE:-}" ]; then
    echo "$MOCK_START_STATE" > "$MOCK_STATE_FILE"
elif [ "$MOCK_START_APPEARS" = 1 ]; then
    echo running > "$MOCK_STATE_FILE"
fi
exit 0
""")
        executable(bin_dir / "killall", """#!/bin/sh
echo "$*" >> "$MOCK_STOP_LOG"
echo "$MOCK_TERM_STATE" > "$MOCK_STATE_FILE"
exit "$MOCK_STOP_STATUS"
""")
        executable(bin_dir / "sleep", """#!/bin/sh
count=0
[ -r "$MOCK_SLEEP_COUNT_FILE" ] && count=$(cat "$MOCK_SLEEP_COUNT_FILE")
count=$((count + 1))
echo "$count" > "$MOCK_SLEEP_COUNT_FILE"
if [ "${MOCK_SLEEP_APPEARS_AFTER:-0}" -gt 0 ] && [ "$count" -ge "$MOCK_SLEEP_APPEARS_AFTER" ]; then
    echo running > "$MOCK_STATE_FILE"
fi
exit "$MOCK_SLEEP_STATUS"
""")
        config = base / "existing-mentohust.conf"
        config.write_text("[MentoHUST]\n")
        state = base / "state"
        state.write_text("absent")
        start_log = base / "start.log"
        stop_log = base / "stop.log"
        sleep_count = base / "sleep.count"
        sleep_count.write_text("0")
        log = base / "logger.log"
        env = os.environ.copy()
        env.update({
            "PATH": str(bin_dir) + os.pathsep + env.get("PATH", ""),
            "MOCK_START_LOG": str(start_log),
            "MOCK_STOP_LOG": str(stop_log),
            "MOCK_STATE_FILE": str(state),
            "MOCK_START_STATUS": "43",
            "MOCK_START_APPEARS": "1",
            "MOCK_STOP_STATUS": "47",
            "MOCK_TERM_STATE": "absent",
            "MOCK_SLEEP_STATUS": "0",
            "MOCK_SLEEP_COUNT_FILE": str(sleep_count),
            "MOCK_SLEEP_APPEARS_AFTER": "0",
            "MOCK_LOG": str(log),
            "MENTOHUST_BIN_CONF_FILE": str(config),
            "PROCESS_LIFECYCLE_LIB": str(PROCESS_LIFECYCLE_LIB),
        })

        failed_start = run(["/bin/sh", str(MENTOHUST_SCRIPT), "start"], env)
        assert failed_start.returncode == 43, (
            f"mentohust must propagate launch status 43, got {failed_start.returncode}"
        )
        assert "Daemon launch failed (status=43)" in log.read_text()
        assert state.read_text().strip() == "absent"

        env["MOCK_START_STATUS"] = "0"
        started = run(["/bin/sh", str(MENTOHUST_SCRIPT), "start"], env)
        assert started.returncode == 0, started.stderr
        assert state.read_text().strip() == "running"
        starts_before = len(start_log.read_text().splitlines())
        duplicate = run(["/bin/sh", str(MENTOHUST_SCRIPT), "start"], env)
        assert duplicate.returncode == 0, duplicate.stderr
        assert len(start_log.read_text().splitlines()) == starts_before

        # killall returns nonzero, but confirmed process absence is authoritative.
        stopped = run(["/bin/sh", str(MENTOHUST_SCRIPT), "stop"], env)
        assert stopped.returncode == 0, stopped.stderr
        assert "[  OK  ]" in stopped.stdout
        assert state.read_text().strip() == "absent"

        # A successful background fork must be followed by a visible child.
        env["MOCK_START_APPEARS"] = "0"
        sleep_count.write_text("0")
        missing_child = run(["/bin/sh", str(MENTOHUST_SCRIPT), "start"], env)
        assert missing_child.returncode == 1, f"mentohust accepted absent child: {missing_child.returncode}"
        assert "startup readiness timed out" in log.read_text()
        assert state.read_text().strip() == "absent"
        assert sleep_count.read_text().strip() == "4"
        sleep_count.write_text("0")
        env["MOCK_SLEEP_APPEARS_AFTER"] = "2"
        delayed_child = run(["/bin/sh", str(MENTOHUST_SCRIPT), "start"], env)
        assert delayed_child.returncode == 0, delayed_child.stderr
        assert state.read_text().strip() == "running"
        assert sleep_count.read_text().strip() == "2"
        stopped_delayed = run(["/bin/sh", str(MENTOHUST_SCRIPT), "stop"], env)
        assert stopped_delayed.returncode == 0, stopped_delayed.stderr
        assert state.read_text().strip() == "absent"
        env["MOCK_SLEEP_APPEARS_AFTER"] = "0"
        env["MOCK_START_STATE"] = "unknown"
        sleep_count.write_text("0")
        unknown_child = run(["/bin/sh", str(MENTOHUST_SCRIPT), "start"], env)
        assert unknown_child.returncode == 2, f"mentohust accepted UNKNOWN child: {unknown_child.returncode}"
        assert "startup readiness could not be verified" in log.read_text()
        assert sleep_count.read_text().strip() == "1"
        del env["MOCK_START_STATE"]

        # UNKNOWN blocks both stop and restart; no signal or replacement launch.
        state.write_text("unknown")
        starts_before = len(start_log.read_text().splitlines())
        stops_before = len(stop_log.read_text().splitlines())
        for action in ("stop", "restart"):
            result = run(["/bin/sh", str(MENTOHUST_SCRIPT), action], env)
            assert result.returncode == 2, (
                f"mentohust {action} must reject UNKNOWN, got {result.returncode}"
            )
        assert len(start_log.read_text().splitlines()) == starts_before
        assert len(stop_log.read_text().splitlines()) == stops_before


TTYD_SCRIPT = ROOT / "trunk/user/ttyd/ttyd.sh"
TTYD_TEST = ROOT / "trunk/user/ttyd/tests/ttyd_lifecycle.sh"


def check_ttyd():
    for path in (TTYD_SCRIPT, TTYD_TEST):
        syntax_check = run(["/bin/sh", "-n", str(path)], os.environ.copy())
        assert syntax_check.returncode == 0, syntax_check.stderr

    env = os.environ.copy()
    env["PROCESS_LIFECYCLE_LIB"] = str(
        ROOT / "trunk/user/shared/scripts/process_lifecycle.sh"
    )
    result = run(["/bin/sh", str(TTYD_TEST)], env)
    assert result.returncode == 0, (
        f"ttyd tri-state lifecycle regression failed: {result.stderr}\n{result.stdout}"
    )
    assert "ttyd tri-state lifecycle, restart gate and NVRAM initialization regression: PASS" in result.stdout


SSHD_SCRIPT = ROOT / "trunk/user/openssh/sshd.sh"
DROPBEAR_SSHD_SCRIPT = ROOT / "trunk/user/dropbear/sshd.sh"


def check_sshd():
    syntax_check = run(["/bin/sh", "-n", str(SSHD_SCRIPT)], os.environ.copy())
    assert syntax_check.returncode == 0, syntax_check.stderr
    with tempfile.TemporaryDirectory(prefix="sshd-lifecycle-") as tmp:
        base = Path(tmp)
        bin_dir = mock_bin(base / "bin")
        storage = base / "storage"
        storage.mkdir()
        old_path = base / "old-storage"
        old_path.mkdir()
        for name in ("ssh_host_rsa_key", "ssh_host_dsa_key", "ssh_host_ed25519_key"):
            (storage / name).write_text("test-key")
        config = storage / "sshd_config"
        config.write_text("Port 22\\n")
        run_dir = base / "run"
        run_dir.mkdir()
        sshd_log = base / "sshd.log"
        executable(bin_dir / "sshd", """#!/bin/sh
echo "$*" >> "$MOCK_SSHD_LOG"
exit "${MOCK_SSHD_STATUS:-0}"
""")
        executable(bin_dir / "killall", """#!/bin/sh
exit "${MOCK_KILLALL_STATUS:-0}"
""")
        env = os.environ.copy()
        env.update({
            "PATH": str(bin_dir) + os.pathsep + env.get("PATH", ""),
            "MOCK_SSHD_LOG": str(sshd_log),
            "MOCK_LOG": str(base / "logger.log"),
            "MOCK_SSHD_STATUS": "59",
            "MOCK_KILLALL_STATUS": "61",
            "SSHD_STORAGE_DIR": str(storage),
            "SSHD_CONFIG_FILE": str(config),
            "SSHD_RSA_KEY": str(storage / "ssh_host_rsa_key"),
            "SSHD_DSA_KEY": str(storage / "ssh_host_dsa_key"),
            "SSHD_ED25519_KEY": str(storage / "ssh_host_ed25519_key"),
            "SSHD_OLD_PATH": str(old_path),
            "SSHD_BIN": str(bin_dir / "sshd"),
            "SSHD_UTMP_FILE": str(run_dir / "utmp"),
            "SSHD_PID_FILE": str(run_dir / "sshd.pid"),
        })
        failed_start = run(["/bin/sh", str(SSHD_SCRIPT), "start"], env)
        assert failed_start.returncode == 59, (
            f"sshd must propagate launch status 59, got {failed_start.returncode}"
        )
        assert "daemon launch failed (status=59)" in (base / "logger.log").read_text()
        env["MOCK_SSHD_STATUS"] = "0"
        started = run(["/bin/sh", str(SSHD_SCRIPT), "start"], env)
        assert started.returncode == 0, started.stderr
        stopped = run(["/bin/sh", str(SSHD_SCRIPT), "stop"], env)
        assert stopped.returncode == 61, (
            f"sshd must propagate killall status 61, got {stopped.returncode}"
        )


def check_dropbear():
    syntax = run(["/bin/sh", "-n", str(DROPBEAR_SSHD_SCRIPT)], os.environ.copy())
    assert syntax.returncode == 0, syntax.stderr
    makefile = (ROOT / "trunk/user/dropbear/Makefile").read_text()
    assert "$(INSTALL_BIN) ./sshd.sh $(ROMFSDIR)/usr/bin/sshd.sh" in makefile

    with tempfile.TemporaryDirectory(prefix="dropbear-lifecycle-") as tmp:
        base = Path(tmp)
        bin_dir = mock_bin(base / "bin")
        storage = base / "storage"
        storage.mkdir()
        for name in ("rsa_host_key", "dss_host_key", "ecdsa_host_key", "ed25519_host_key"):
            (storage / name).write_text("fixture-key")
        (storage / "extra_args").write_text("-p 2222")
        legacy = base / "legacy"
        legacy.mkdir()
        daemon_log = base / "dropbear.log"
        executable(bin_dir / "dropbear", """#!/bin/sh
printf '%s\\n' "$*" >> "$MOCK_DROPBEAR_LOG"
exit "$MOCK_DROPBEAR_STATUS"
""")
        executable(bin_dir / "dropbearkey", """#!/bin/sh
exit "$MOCK_KEYGEN_STATUS"
""")
        executable(bin_dir / "killall", """#!/bin/sh
exit "$MOCK_KILLALL_STATUS"
""")
        env = os.environ.copy()
        env.update({
            "PATH": str(bin_dir) + os.pathsep + env.get("PATH", ""),
            "MOCK_LOG": str(base / "logger.log"),
            "MOCK_DROPBEAR_LOG": str(daemon_log),
            "MOCK_DROPBEAR_STATUS": "53",
            "MOCK_KEYGEN_STATUS": "71",
            "MOCK_KILLALL_STATUS": "54",
            "DROPBEAR_STORAGE_DIR": str(storage),
            "DROPBEAR_OLD_KEY_PREFIX": str(legacy / "dropbear_"),
            "DROPBEAR_BIN": str(bin_dir / "dropbear"),
            "DROPBEAR_KEYGEN_BIN": str(bin_dir / "dropbearkey"),
        })

        failed = run(["/bin/sh", str(DROPBEAR_SSHD_SCRIPT), "start"], env)
        assert failed.returncode == 53, (
            f"dropbear must preserve daemon status 53, got {failed.returncode}"
        )
        assert daemon_log.read_text().splitlines()[-1] == "-p 2222"
        assert "daemon launch failed (status=53)" in (base / "logger.log").read_text()

        env["MOCK_DROPBEAR_STATUS"] = "0"
        started = run(["/bin/sh", str(DROPBEAR_SSHD_SCRIPT), "start", "-s"], env)
        assert started.returncode == 0, started.stderr
        assert daemon_log.read_text().splitlines()[-1] == "-s -p 2222"

        stopped = run(["/bin/sh", str(DROPBEAR_SSHD_SCRIPT), "stop"], env)
        assert stopped.returncode == 54, (
            f"dropbear must propagate killall status 54, got {stopped.returncode}"
        )
        assert "daemon stop failed (status=54)" in (base / "logger.log").read_text()

        keygen = run(["/bin/sh", str(DROPBEAR_SSHD_SCRIPT), "newkeys"], env)
        assert keygen.returncode == 71, (
            f"dropbear must propagate keygen status 71, got {keygen.returncode}"
        )
        assert "host-key generation failed for rsa (status=71)" in (
            base / "logger.log"
        ).read_text()

        invalid = run(["/bin/sh", str(DROPBEAR_SSHD_SCRIPT), "invalid"], env)
        assert invalid.returncode == 1, (
            f"dropbear invalid action must return 1, got {invalid.returncode}"
        )


def check_shared_process_lifecycle():
    helper = PROCESS_LIFECYCLE_LIB.read_text()
    test = PROCESS_LIFECYCLE_TEST.read_text()
    makefile = SHARED_MAKEFILE.read_text()
    assert "pl_process_state()" in helper
    assert "pl_wait_stopped()" in helper
    assert "pl_wait_started()" in helper
    assert "pl_stop_named()" in helper
    assert "$(INSTALL_BIN) $(CURDIR)/scripts/process_lifecycle.sh $(ROMFSDIR)/usr/lib/process_lifecycle.sh" in makefile
    for token in ("empty-success", "malformed", "inconsistent", "post-TERM UNKNOWN accepted",
                  "sleep failure", "stubborn process", "UNKNOWN authorized a signal",
                  "KILL:-q -9 test-daemon", "ABSENT startup timeout accepted",
                  "UNKNOWN inventory accepted", "interrupted startup wait", "delayed RUNNING start"):
        assert token in test, f"shared lifecycle regression missing {token}"
    for path in (PROCESS_LIFECYCLE_LIB, PROCESS_LIFECYCLE_TEST):
        syntax = run(["/bin/sh", "-n", str(path)], os.environ.copy())
        assert syntax.returncode == 0, syntax.stderr
    result = run(["/bin/sh", str(PROCESS_LIFECYCLE_TEST)], os.environ.copy())
    assert result.returncode == 0, result.stderr
    assert "shared process lifecycle regression: PASS" in result.stdout


if __name__ == "__main__":
    check_dns_forwarder_rc_shutdown_registration()
    check_ralink_iappd_rc_shutdown_registration()
    check_shared_process_lifecycle()
    check_scutclient()
    check_vlmcsd()
    check_iperf3()
    check_mentohust()
    check_ttyd()
    check_sshd()
    check_dropbear()
    print("optional app launch/lifecycle status regressions: PASS")
