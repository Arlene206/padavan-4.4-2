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
OPTIONAL_DAEMON_POLICY = ROOT / "trunk/user/rc/src/optional_daemon_policy.h"
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
    source = VLM_SCRIPT.read_text()
    assert '. "$PROCESS_LIFECYCLE_LIB" || exit 127' in source
    assert "pl_process_state vlmcsd" in source
    assert "pl_wait_started vlmcsd 1 4 1" in source
    assert "pl_stop_named vlmcsd 3 2" in source

    with tempfile.TemporaryDirectory(prefix="vlmcsd-start-") as tmp:
        base = Path(tmp)
        bin_dir = mock_bin(base / "bin")
        state = base / "state"
        state.write_text("absent")
        starts = base / "starts.log"
        sleeps = base / "sleeps.log"
        kill_log = base / "killall.log"
        executable(bin_dir / "vlmcsd", """#!/bin/sh
echo "$*" >> "$MOCK_START_LOG"
status="${MOCK_LAUNCH_STATUS:-0}"
[ "$status" -eq 0 ] || exit "$status"
[ -n "${MOCK_START_STATE:-}" ] && echo "$MOCK_START_STATE" > "$MOCK_STATE_FILE"
exit 0
""")
        executable(bin_dir / "pidof", """#!/bin/sh
case "$(cat "$MOCK_STATE_FILE")" in
    running) echo 123; exit 0 ;;
    absent) exit 1 ;;
    unknown) exit 2 ;;
    *) exit 2 ;;
esac
""")
        executable(bin_dir / "sleep", """#!/bin/sh
echo "$1" >> "$MOCK_SLEEP_LOG"
exit "${MOCK_SLEEP_STATUS:-0}"
""")
        executable(bin_dir / "killall", """#!/bin/sh
echo "$*" >> "$MOCK_KILL_LOG"
case " $* " in
    *" -9 "*) next_state="$MOCK_KILL_STATE" ;;
    *) next_state="$MOCK_TERM_STATE" ;;
esac
echo "$next_state" > "$MOCK_STATE_FILE"
exit "${MOCK_KILLALL_STATUS:-0}"
""")
        log = base / "logger.log"
        env = os.environ.copy()
        env.update({
            "PATH": str(bin_dir) + os.pathsep + env.get("PATH", ""),
            "MOCK_LOG": str(log),
            "MOCK_START_LOG": str(starts),
            "MOCK_SLEEP_LOG": str(sleeps),
            "MOCK_KILL_LOG": str(kill_log),
            "MOCK_STATE_FILE": str(state),
            "MOCK_LAUNCH_STATUS": "23",
            "MOCK_KILLALL_STATUS": "0",
            "MOCK_TERM_STATE": "absent",
            "MOCK_KILL_STATE": "absent",
            "PROCESS_LIFECYCLE_LIB": str(PROCESS_LIFECYCLE_LIB),
        })

        # The detached launcher must preserve its own failure status.
        failed = run(["/bin/sh", str(VLM_SCRIPT), "start"], env)
        assert failed.returncode == 23, (
            f"vlmcsd must propagate launcher status 23, got "
            f"{failed.returncode}: {failed.stderr}"
        )
        assert "launch failed (status=23)" in log.read_text()
        assert not sleeps.exists(), "failed launcher must not enter readiness polling"

        # A successful daemon fork is not proof the detached process survived.
        log.write_text("")
        env["MOCK_LAUNCH_STATUS"] = "0"
        env["MOCK_START_STATE"] = "running"
        state.write_text("absent")
        started = run(["/bin/sh", str(VLM_SCRIPT), "start"], env)
        assert started.returncode == 0, started.stderr
        assert "KMS Activation server is running." in log.read_text()
        assert len(starts.read_text().splitlines()) == 2

        # A pre-existing daemon is a no-op; do not start a duplicate.
        log.write_text("")
        state.write_text("running")
        starts_before = len(starts.read_text().splitlines())
        duplicate = run(["/bin/sh", str(VLM_SCRIPT), "start"], env)
        assert duplicate.returncode == 0, duplicate.stderr
        assert len(starts.read_text().splitlines()) == starts_before
        assert "already running" in log.read_text()

        # An UNKNOWN initial inventory must fail closed without launching.
        log.write_text("")
        state.write_text("unknown")
        unknown_preflight = run(["/bin/sh", str(VLM_SCRIPT), "start"], env)
        assert unknown_preflight.returncode == 2, unknown_preflight.stderr
        assert len(starts.read_text().splitlines()) == starts_before
        assert "state is unknown" in log.read_text()

        # If daemonization succeeds but no process appears, bounded readiness times out.
        log.write_text("")
        state.write_text("absent")
        del env["MOCK_START_STATE"]
        if sleeps.exists():
            sleeps.unlink()
        timeout = run(["/bin/sh", str(VLM_SCRIPT), "start"], env)
        assert timeout.returncode == 1, (
            f"vlmcsd accepted an absent daemon after launch: {timeout.returncode}"
        )
        assert "startup timed out" in log.read_text()
        assert len(sleeps.read_text().splitlines()) == 4, "readiness wait must be bounded to four sleeps"

        # UNKNOWN during readiness is distinct from a confirmed absence timeout.
        log.write_text("")
        state.write_text("absent")
        env["MOCK_START_STATE"] = "unknown"
        sleeps.unlink()
        unknown_ready = run(["/bin/sh", str(VLM_SCRIPT), "start"], env)
        assert unknown_ready.returncode == 2, unknown_ready.stderr
        assert "could not be verified" in log.read_text()
        assert len(sleeps.read_text().splitlines()) == 1

        # Stop must wait for the verified postcondition, even if killall reports a signal error.
        log.write_text("")
        state.write_text("running")
        env["MOCK_TERM_STATE"] = "absent"
        env["MOCK_KILLALL_STATUS"] = "17"
        stopped = run(["/bin/sh", str(VLM_SCRIPT), "stop"], env)
        assert stopped.returncode == 0, stopped.stderr
        assert state.read_text().strip() == "absent"
        assert "stopped." in log.read_text()
        assert len(kill_log.read_text().splitlines()) == 1

        # A failed stop must prevent restart from launching a second daemon.
        log.write_text("")
        state.write_text("running")
        env["MOCK_TERM_STATE"] = "running"
        env["MOCK_KILL_STATE"] = "running"
        env["MOCK_KILLALL_STATUS"] = "0"
        starts_before = len(starts.read_text().splitlines())
        restart = run(["/bin/sh", str(VLM_SCRIPT), "restart"], env)
        assert restart.returncode == 1, (
            f"vlmcsd restart ignored a stuck daemon: {restart.returncode}"
        )
        assert len(starts.read_text().splitlines()) == starts_before, (
            "vlmcsd restart launched after stop postcondition failed"
        )
        assert "restart aborted" in log.read_text()


def check_rc_optional_daemon_enable_reads():
    services = RC_SERVICES_SOURCE.read_text()
    reader = extract_c_function_body(
        services, "\nservice_enabled_mode_status(const char *service_name"
    )
    assert "nvram_get_checked(nvram_key, &value)" in reader
    assert "rc_wan_nvram_read_int_status(" in reader
    assert "rc_service_enabled_mode_decide(" in reader
    assert "preserving existing daemon state" in reader

    rollback_helper = extract_c_function_body(
        services, "optional_daemon_rollback_failed_start("
    )
    assert "detached_daemon_start_rollback_with_ops(" in rollback_helper
    assert "optional_daemon_rollback_stop" in rollback_helper
    assert "optional_daemon_rollback_scan" in rollback_helper
    assert "errno = saved_errno;" in rollback_helper

    for signature, key in (
        ("start_infosvr(void)", "adsc_enable"),
        ("start_crond(void)", "crond_enable"),
        ("start_rstats(void)", "rstats_enable"),
        ("start_lltd(void)", "lltd_enable"),
    ):
        start_body = extract_c_function_body(services, signature)
        compact_start = "".join(start_body.split())
        read_call = (
            'service_nvram_match_status("' + key + '","1",0,&enabled);'
        )
        assert read_call in compact_start, signature
        assert compact_start.index(read_call) < compact_start.index("if(config_status!=0)")
        assert compact_start.index("if(config_status!=0)") < compact_start.index("if(!enabled)")
        assert "nvram_invmatch(" not in start_body
        assert "nvram_match(" not in start_body
        assert "nvram_get_int(" not in start_body
        if key == "crond_enable":
            assert 'service_nvram_match_status("crond_log","0",0,&debug_logging);' in compact_start
            assert "cannot read crond logging mode" in start_body
            assert "continuing with default logging" in start_body

    detached_start_cases = (
        ("start_ttyd_with_mode(int ttyd_mode)", "ttyd", "start_status", "start_errno"),
        ("start_rstats(void)", "rstats", "start_status", "start_errno"),
        ("start_lltd(void)", "lld2d", "start_status", "start_errno"),
        ("start_skipdbv2(void)", "skipd", "launch_status", "launch_errno"),
    )
    for signature, process_name, status_var, errno_var in detached_start_cases:
        start_body = extract_c_function_body(services, signature)
        compact_start_body = "".join(start_body.split())
        assert (
            'optional_daemon_presence("' + process_name + '",&scan_error)'
            in compact_start_body
        )
        for call in (
            'optional_daemon_rollback_failed_start("' + process_name + '",' +
            status_var + "," + errno_var + ")",
            'optional_daemon_rollback_failed_start("' + process_name + '",-1,' +
            errno_var + ")",
            'optional_daemon_rollback_failed_start("' + process_name + '",-1,ETIMEDOUT)',
        ):
            assert call in compact_start_body, signature + ": " + call
        assert "PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS" in start_body
        assert "PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS" in start_body
        assert "PROCESS_PRESENCE_UNKNOWN" in start_body

    telnetd_recovery = extract_c_function_body(
        services, "telnetd_start_failure_recover(int failure_status, const char *phase)"
    )
    assert "stop_telnetd()" in telnetd_recovery
    assert 'optional_daemon_presence("telnetd", &scan_error)' in telnetd_recovery
    assert "optional_daemon_failed_start_may_survive(after_stop)" in telnetd_recovery
    assert telnetd_recovery.index("stop_telnetd()") < telnetd_recovery.index(
        'optional_daemon_presence("telnetd", &scan_error)'
    ) < telnetd_recovery.index("optional_daemon_failed_start_may_survive(after_stop)") < telnetd_recovery.index(
        "errno = failure_errno;"
    ) < telnetd_recovery.index("return failure_status;")

    telnetd_start = extract_c_function_body(services, "start_telnetd(void)")
    for recovery_call in (
        'telnetd_start_failure_recover(start_status, "launcher")',
        'telnetd_start_failure_recover(start_status, "initial startup")',
        'telnetd_start_failure_recover(-1, "process inventory")',
        'telnetd_start_failure_recover(start_status, "retry")',
    ):
        assert recovery_call in telnetd_start
    assert "return process_presence_start_postcondition(start_status," in telnetd_start
    assert "optional_daemon_failed_start_may_survive" in OPTIONAL_DAEMON_POLICY.read_text()

    for name, service_name, key in (
        ("adguardhome", "AdGuardHome", "adg_enable"),
        ("ddnsto", "DDNS.to", "ddnsto_enable"),
        ("sqm", "SQM", "sqm_enable"),
        ("aldriver", "AliyunDrive WebDAV", "aliyundrive_enable"),
        ("wireguard", "WireGuard", "wireguard_enable"),
        ("adbyby", "Adbyby", "adbyby_enable"),
    ):
        start_body = extract_c_function_body(services, "start_" + name + "(void)")
        compact_start = "".join(start_body.split())
        mode_var = {
            "adguardhome": "adg_mode",
            "zerotier": "zerotier_enable",
            "ddnsto": "ddnsto_enable",
            "sqm": "sqm_mode",
            "aldriver": "aldriver_enable",
            "wireguard": "wireguard_enable",
            "adbyby": "adbyby_mode",
        }[name]
        compact_service_name = service_name.replace(" ", "")
        read_call = (
            'service_enabled_mode_status("' + compact_service_name + '","' + key +
            '",&' + mode_var + ');'
        )
        assert read_call in compact_start, name
        assert compact_start.index(read_call) < compact_start.index("if(config_status!=0)"), name
        if name == "adbyby":
            assert compact_start.index("if(config_status!=0)") < compact_start.index("return;"), name
            assert "nvram_get_int(" not in start_body
            assert "nvram_safe_get(" not in start_body
        else:
            assert compact_start.index("if(config_status!=0)") < compact_start.index(
                "returnconfig_status;"
            ), name
            assert compact_start.index("if(config_status!=0)") < compact_start.index("return0;"), name
            assert "nvram_get_int(" not in start_body
            assert "nvram_safe_get(" not in start_body

    telnetd_start = extract_c_function_body(services, "start_telnetd(void)")
    telnetd_compact = "".join(telnetd_start.split())
    telnetd_read = 'service_nvram_match_status("telnetd","1",0,&telnet_enabled);'
    assert telnetd_read in telnetd_compact
    assert telnetd_compact.index(telnetd_read) < telnetd_compact.index("if(config_status!=0)")
    assert telnetd_compact.index("if(config_status!=0)") < telnetd_compact.index(
        "if(!telnet_enabled)"
    )
    assert 'nvram_match("telnetd", "1")' not in telnetd_start

    zerotier_start = extract_c_function_body(services, "start_zerotier(void)")
    zerotier_compact = "".join(zerotier_start.split())
    zerotier_read = 'service_nvram_match_status("zerotier_enable","1",0,&zerotier_enabled);'
    assert zerotier_read in zerotier_compact
    assert zerotier_compact.index(zerotier_read) < zerotier_compact.index("if(config_status!=0)")
    assert zerotier_compact.index("if(config_status!=0)") < zerotier_compact.index(
        "if(!zerotier_enabled)"
    )
    assert zerotier_compact.index("if(!zerotier_enabled)") < zerotier_compact.index(
        'returneval("/usr/bin/zerotier.sh","start");'
    )
    assert "nvram_get_int(" not in zerotier_start
    assert "nvram_match(" not in zerotier_start
    assert "nvram_safe_get(" not in zerotier_start

    smartdns_start = extract_c_function_body(services, "start_smartdns(void)")
    smartdns_compact = "".join(smartdns_start.split())
    smartdns_read = 'service_enabled_mode_status("SmartDNS","sdns_enable",&smartdns_mode);'
    assert smartdns_read in smartdns_compact
    assert smartdns_compact.index(smartdns_read) < smartdns_compact.index("if(config_status!=0)")
    assert smartdns_compact.index("if(config_status!=0)") < smartdns_compact.index(
        'returneval("/usr/bin/smartdns.sh","start");'
    )
    assert "nvram_get_int(" not in smartdns_start
    assert "nvram_safe_get(" not in smartdns_start

    aliddns_start = extract_c_function_body(services, "start_aliddns(void)")
    aliddns_compact = "".join(aliddns_start.split())
    aliddns_read = 'service_enabled_mode_status("AliDDNS","aliddns_enable",&aliddns_mode);'
    assert aliddns_read in aliddns_compact
    assert aliddns_compact.index(aliddns_read) < aliddns_compact.index("if(config_status!=0)")
    assert aliddns_compact.index("if(config_status!=0)") < aliddns_compact.index(
        "if(aliddns_mode==1)"
    )
    assert "nvram_get_int(" not in aliddns_start
    assert "nvram_safe_get(" not in aliddns_start

    # These optional service start APIs used to be void: a checked enable-read
    # failure or shell launcher failure was logged/swallowed instead of being
    # observable by the lifecycle caller.
    for name, service_name, key, mode_var, script in (
        ("adguardhome", "AdGuardHome", "adg_enable", "adg_mode", "adguardhome.sh"),
        ("ddnsto", "DDNS.to", "ddnsto_enable", "ddnsto_enable", "ddnsto.sh"),
        ("aliddns", "AliDDNS", "aliddns_enable", "aliddns_mode", "aliddns.sh"),
    ):
        body = extract_c_function_body(services, "start_" + name + "(void)")
        compact = "".join(body.split())
        read = (
            'service_enabled_mode_status("' + service_name.replace(" ", "") +
            '","' + key + '",&' + mode_var + ');'
        )
        assert read in compact, name
        assert "if(config_status!=0)returnconfig_status;" in compact, name
        assert "return0;" in compact, name
        assert 'returneval("/usr/bin/' + script + '","start");' in compact, name
        assert "nvram_safe_get(" not in body and "nvram_get_int(" not in body

    frp_start = extract_c_function_body(services, "start_frp(void)")
    assert 'returneval("/usr/bin/frp.sh","start");' in "".join(frp_start.split())
    for name in ("adguardhome", "ddnsto", "aliddns", "frp"):
        assert "int start_" + name + "(void)" in services, name
        assert "int start_" + name + "(void);" in header, name

    for name, key, stop_name, start_helper in (
        ("scutclient", "scutclient_enable", "stop_scutclient()", "start_scutclient_with_mode(enabled)"),
        ("mentohust", "mentohust_enable", "stop_mentohust()", "start_mentohust_with_mode(enabled)"),
    ):
        service_name = "SCUT client" if name == "scutclient" else "MentoHUST"
        start_body = extract_c_function_body(services, "start_" + name + "(void)")
        restart_body = extract_c_function_body(services, "restart_" + name + "(void)")
        compact_start = "".join(start_body.split())
        compact_restart = "".join(restart_body.split())
        compact_service_name = service_name.replace(" ", "")
        read_call = (
            'service_enabled_mode_status("' + compact_service_name + '","' + key + '",&enabled);'
        )
        assert read_call in compact_start
        assert read_call in compact_restart
        assert compact_start.index(read_call) < compact_start.index("if(config_status!=0)")
        assert compact_start.index("if(config_status!=0)") < compact_start.index("return" + start_helper + ";")
        assert compact_restart.index(read_call) < compact_restart.index("if(config_status!=0)")
        assert compact_restart.index("if(config_status!=0)") < compact_restart.index(stop_name)
        assert compact_restart.index(stop_name) < compact_restart.index("if(stop_status!=0)")
        assert compact_restart.index("if(stop_status!=0)") < compact_restart.index("return" + start_helper + ";")
        for token in ("nvram_get_int(", "nvram_safe_get("):
            assert token not in start_body
            assert token not in restart_body

    ttyd_start = extract_c_function_body(services, "start_ttyd(void)")
    ttyd_restart = extract_c_function_body(services, "restart_ttyd(void)")
    compact_ttyd_start = "".join(ttyd_start.split())
    compact_ttyd_restart = "".join(ttyd_restart.split())
    ttyd_read = 'service_enabled_mode_status("ttyd","ttyd_enable",&ttyd_mode);'
    assert ttyd_read in compact_ttyd_start
    assert ttyd_read in compact_ttyd_restart
    assert compact_ttyd_start.index(ttyd_read) < compact_ttyd_start.index("if(config_status!=0)")
    assert compact_ttyd_start.index("if(config_status!=0)") < compact_ttyd_start.index(
        "returnstart_ttyd_with_mode(ttyd_mode);"
    )
    assert compact_ttyd_restart.index(ttyd_read) < compact_ttyd_restart.index("if(config_status!=0)")
    assert compact_ttyd_restart.index("if(config_status!=0)") < compact_ttyd_restart.index(
        "stop_status=stop_ttyd();"
    )
    assert compact_ttyd_restart.index("stop_status=stop_ttyd();") < compact_ttyd_restart.index(
        "if(stop_status!=0)"
    )
    assert compact_ttyd_restart.index("if(stop_status!=0)") < compact_ttyd_restart.index(
        "returnstart_ttyd_with_mode(ttyd_mode);"
    )
    assert "ttyd_mode=nvram_get_int(" not in compact_ttyd_start
    assert "ttyd_mode=nvram_get_int(" not in compact_ttyd_restart

    for name, service_name, key, mode_var, stop_name, start_helper in (
        ("ss", "Shadowsocks", "ss_enable", "ss_enable", "stop_ss()", "start_ss_with_mode(ss_enable)"),
        ("ss_tunnel", "Shadowsocks tunnel", "ss-tunnel_enable", "ss_tunnel_mode",
         "stop_ss_tunnel()", "start_ss_tunnel_with_mode(ss_tunnel_mode)"),
    ):
        start_body = extract_c_function_body(services, "start_" + name + "(void)")
        restart_body = extract_c_function_body(services, "restart_" + name + "(void)")
        compact_start = "".join(start_body.split())
        compact_restart = "".join(restart_body.split())
        compact_service_name = service_name.replace(" ", "")
        read_call = (
            'service_enabled_mode_status("' + compact_service_name + '","' + key + '",&' + mode_var + ');'
        )
        assert read_call in compact_start
        assert read_call in compact_restart
        assert compact_start.index(read_call) < compact_start.index("if(config_status!=0)")
        assert compact_start.index("if(config_status!=0)") < compact_start.index("return" + start_helper + ";")
        assert compact_restart.index(read_call) < compact_restart.index("if(config_status!=0)")
        assert compact_restart.index("if(config_status!=0)") < compact_restart.index(stop_name)
        assert compact_restart.index(stop_name) < compact_restart.index("if(stop_status!=0)")
        assert compact_restart.index("if(stop_status!=0)") < compact_restart.index("return" + start_helper + ";")
        for token in ("nvram_get_int(", "nvram_safe_get("):
            assert token not in start_body
            assert token not in restart_body



def check_scutclient():
    source = SCUT_SCRIPT.read_text()
    assert '. "$PROCESS_LIFECYCLE_LIB" || exit 127' in source
    assert 'pl_stop_named "$scutclient_exec" 3 2' in source
    assert 'pl_wait_started "$scutclient_exec" 1 4 1' in source
    assert source.index("if start-stop-daemon ") < source.index(
        'if pl_wait_started "$scutclient_exec" 1 4 1'
    ) < source.index('if pl_stop_named "$scutclient_exec" 3 2')
    assert "primary_status=$readiness_status" in source
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
count=0
[ -r "$MOCK_PIDOF_COUNT_FILE" ] && count=$(cat "$MOCK_PIDOF_COUNT_FILE")
count=$((count + 1))
echo "$count" > "$MOCK_PIDOF_COUNT_FILE"
if [ "${MOCK_PIDOF_RUNNING_AT:-0}" -gt 0 ] &&
   [ "$count" -eq "$MOCK_PIDOF_RUNNING_AT" ]; then
    echo running > "$MOCK_STATE_FILE"
fi
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
case " $* " in
    *" -9 "*) echo "$MOCK_KILL_STATE" > "$MOCK_STATE_FILE" ;;
    *) echo "$MOCK_TERM_STATE" > "$MOCK_STATE_FILE" ;;
esac
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
            "MOCK_PIDOF_COUNT_FILE": str(base / "pidof.count"),
            "MOCK_PIDOF_RUNNING_AT": "0",
            "MOCK_KILL_STATE": "absent",
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

        # ABSENT after the bounded wait needs no signal; a late child appearing
        # at the rollback recheck must be stopped without changing primary status.
        env["MOCK_START_APPEARS"] = "0"
        log.write_text("")
        stop_count_before = len(stop_log.read_text().splitlines())
        sleep_count.write_text("0")
        pidof_count = Path(env["MOCK_PIDOF_COUNT_FILE"])
        pidof_count.write_text("0")
        missing_child = run(["/bin/sh", str(SCUT_SCRIPT), "start"], env)
        assert missing_child.returncode == 1, f"SCUT accepted absent child: {missing_child.returncode}"
        assert "startup readiness timed out" in log.read_text()
        assert "Daemon startup rollback confirmed process absent" in log.read_text()
        assert state.read_text().strip() == "absent"
        assert sleep_count.read_text().strip() == "4"
        assert len(stop_log.read_text().splitlines()) == stop_count_before

        log.write_text("")
        env["MOCK_PIDOF_RUNNING_AT"] = "6"
        pidof_count.write_text("0")
        sleep_count.write_text("0")
        late_child = run(["/bin/sh", str(SCUT_SCRIPT), "start"], env)
        assert late_child.returncode == 1, "SCUT replaced readiness failure with rollback success"
        assert state.read_text().strip() == "absent"
        stop_lines = stop_log.read_text().splitlines()
        assert len(stop_lines) == stop_count_before + 1 and stop_lines[-1] == "-q bin_scutclient"
        assert "Daemon startup rollback confirmed process absent" in log.read_text()
        env["MOCK_PIDOF_RUNNING_AT"] = "0"
        pidof_count.write_text("0")

        log.write_text("")
        env["MOCK_PIDOF_RUNNING_AT"] = "6"
        env["MOCK_TERM_STATE"] = "running"
        env["MOCK_KILL_STATE"] = "running"
        pidof_count.write_text("0")
        sleep_count.write_text("0")
        cleanup_failed = run(["/bin/sh", str(SCUT_SCRIPT), "start"], env)
        assert cleanup_failed.returncode == 1
        assert "Daemon startup rollback incomplete (primary_status=1, rollback_status=1)" in log.read_text()
        assert stop_log.read_text().splitlines()[-1] == "-q -9 bin_scutclient"
        assert state.read_text().strip() == "running"
        env["MOCK_TERM_STATE"] = "absent"
        env["MOCK_KILL_STATE"] = "absent"
        env["MOCK_PIDOF_RUNNING_AT"] = "0"
        pidof_count.write_text("0")
        state.write_text("absent")
        stop_count_before = len(stop_log.read_text().splitlines())
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
        log.write_text("")
        sleep_count.write_text("0")
        pidof_count.write_text("0")
        stop_count_before = len(stop_log.read_text().splitlines())
        unknown_child = run(["/bin/sh", str(SCUT_SCRIPT), "start"], env)
        assert unknown_child.returncode == 2, f"SCUT accepted UNKNOWN child: {unknown_child.returncode}"
        assert "startup readiness could not be verified" in log.read_text()
        assert "Daemon startup rollback incomplete (primary_status=2, rollback_status=2)" in log.read_text()
        assert sleep_count.read_text().strip() == "1"
        assert len(stop_log.read_text().splitlines()) == stop_count_before, "UNKNOWN rollback must not signal"
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
    assert source.index("if start-stop-daemon ") < source.index(
        'if pl_wait_started "$mentohust_exec" 1 4 1'
    ) < source.index('if pl_stop_named "$mentohust_exec" 3 2')
    assert "primary_status=$readiness_status" in source
    with tempfile.TemporaryDirectory(prefix="mentohust-lifecycle-") as tmp:
        base = Path(tmp)
        bin_dir = mock_bin(base / "bin")
        executable(bin_dir / "pidof", """#!/bin/sh
count=0
[ -r "$MOCK_PIDOF_COUNT_FILE" ] && count=$(cat "$MOCK_PIDOF_COUNT_FILE")
count=$((count + 1))
echo "$count" > "$MOCK_PIDOF_COUNT_FILE"
if [ "${MOCK_PIDOF_RUNNING_AT:-0}" -gt 0 ] &&
   [ "$count" -eq "$MOCK_PIDOF_RUNNING_AT" ]; then
    echo running > "$MOCK_STATE_FILE"
fi
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
case " $* " in
    *" -9 "*) echo "$MOCK_KILL_STATE" > "$MOCK_STATE_FILE" ;;
    *) echo "$MOCK_TERM_STATE" > "$MOCK_STATE_FILE" ;;
esac
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
            "MOCK_PIDOF_COUNT_FILE": str(base / "pidof.count"),
            "MOCK_PIDOF_RUNNING_AT": "0",
            "MOCK_KILL_STATE": "absent",
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
        # Recheck/rollback closes the race where the daemon appears just after
        # the last readiness probe, while retaining the primary timeout.
        env["MOCK_START_APPEARS"] = "0"
        log.write_text("")
        stop_count_before = len(stop_log.read_text().splitlines())
        sleep_count.write_text("0")
        pidof_count = Path(env["MOCK_PIDOF_COUNT_FILE"])
        pidof_count.write_text("0")
        missing_child = run(["/bin/sh", str(MENTOHUST_SCRIPT), "start"], env)
        assert missing_child.returncode == 1, f"mentohust accepted absent child: {missing_child.returncode}"
        assert "startup readiness timed out" in log.read_text()
        assert "Daemon startup rollback confirmed process absent" in log.read_text()
        assert state.read_text().strip() == "absent"
        assert sleep_count.read_text().strip() == "4"
        assert len(stop_log.read_text().splitlines()) == stop_count_before

        log.write_text("")
        env["MOCK_PIDOF_RUNNING_AT"] = "6"
        pidof_count.write_text("0")
        sleep_count.write_text("0")
        late_child = run(["/bin/sh", str(MENTOHUST_SCRIPT), "start"], env)
        assert late_child.returncode == 1, "MentoHUST replaced readiness failure with rollback success"
        assert state.read_text().strip() == "absent"
        stop_lines = stop_log.read_text().splitlines()
        assert len(stop_lines) == stop_count_before + 1 and stop_lines[-1] == "-q bin_mentohust"
        assert "Daemon startup rollback confirmed process absent" in log.read_text()
        env["MOCK_PIDOF_RUNNING_AT"] = "0"
        pidof_count.write_text("0")

        log.write_text("")
        env["MOCK_PIDOF_RUNNING_AT"] = "6"
        env["MOCK_TERM_STATE"] = "running"
        env["MOCK_KILL_STATE"] = "running"
        pidof_count.write_text("0")
        sleep_count.write_text("0")
        cleanup_failed = run(["/bin/sh", str(MENTOHUST_SCRIPT), "start"], env)
        assert cleanup_failed.returncode == 1
        assert "Daemon startup rollback incomplete (primary_status=1, rollback_status=1)" in log.read_text()
        assert stop_log.read_text().splitlines()[-1] == "-q -9 bin_mentohust"
        assert state.read_text().strip() == "running"
        env["MOCK_TERM_STATE"] = "absent"
        env["MOCK_KILL_STATE"] = "absent"
        env["MOCK_PIDOF_RUNNING_AT"] = "0"
        pidof_count.write_text("0")
        state.write_text("absent")
        stop_count_before = len(stop_log.read_text().splitlines())
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
        log.write_text("")
        sleep_count.write_text("0")
        pidof_count.write_text("0")
        stop_count_before = len(stop_log.read_text().splitlines())
        unknown_child = run(["/bin/sh", str(MENTOHUST_SCRIPT), "start"], env)
        assert unknown_child.returncode == 2, f"mentohust accepted UNKNOWN child: {unknown_child.returncode}"
        assert "startup readiness could not be verified" in log.read_text()
        assert "Daemon startup rollback incomplete (primary_status=2, rollback_status=2)" in log.read_text()
        assert sleep_count.read_text().strip() == "1"
        assert len(stop_log.read_text().splitlines()) == stop_count_before, "UNKNOWN rollback must not signal"
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
    assert "ttyd tri-state lifecycle, detached-child readiness, restart gate and NVRAM initialization regression: PASS" in result.stdout


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


def check_vlmcsd_iperf3_checked_enable_reads():
    services = RC_SERVICES_SOURCE.read_text()
    for name, key, stop_call in (
        ("vlmcsd", "vlmcsd_enable", "stop_vlmcsd()"),
        ("iperf3", "iperf3_enable", "stop_iperf3()"),
    ):
        start_body = extract_c_function_body(services, "start_" + name + "(void)")
        restart_body = extract_c_function_body(services, "restart_" + name + "(void)")
        helper_body = extract_c_function_body(
            services, "start_" + name + "_with_mode(int enabled)"
        )
        compact_start = "".join(start_body.split())
        compact_restart = "".join(restart_body.split())
        compact_helper = "".join(helper_body.split())
        read_call = (
            'service_enabled_mode_status("' + name + '","' + key + '",&enabled);'
        )
        assert read_call in compact_start
        assert compact_start.index(read_call) < compact_start.index("if(config_status!=0)")
        assert compact_start.index("if(config_status!=0)") < compact_start.index(
            "returnstart_" + name + "_with_mode(enabled);"
        )
        assert read_call in compact_restart
        assert compact_restart.index(read_call) < compact_restart.index("if(config_status!=0)")
        assert compact_restart.index("if(config_status!=0)") < compact_restart.index(stop_call)
        assert compact_restart.index(stop_call) < compact_restart.index("if(stop_status!=0)")
        assert compact_restart.index("if(stop_status!=0)") < compact_restart.index(
            "returnstart_" + name + "_with_mode(enabled);"
        )
        assert "if(enabled!=1)" in compact_helper
        assert "return0;" in compact_helper
        assert 'optional_daemon_start_status("' + name + '", "' + name + '")' in helper_body
        assert "nvram_get_int(" not in start_body
        assert "nvram_safe_get(" not in start_body
        assert "nvram_get_int(" not in restart_body
        assert "nvram_safe_get(" not in restart_body


if __name__ == "__main__":
    check_dns_forwarder_rc_shutdown_registration()
    check_ralink_iappd_rc_shutdown_registration()
    check_shared_process_lifecycle()
    check_rc_optional_daemon_enable_reads()
    check_vlmcsd_iperf3_checked_enable_reads()
    check_scutclient()
    check_vlmcsd()
    check_iperf3()
    check_mentohust()
    check_ttyd()
    check_sshd()
    check_dropbear()
    print("optional app launch/lifecycle status regressions: PASS")
