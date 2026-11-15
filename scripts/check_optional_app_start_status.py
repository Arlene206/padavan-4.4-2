#!/usr/bin/env python3
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SCUT_SCRIPT = ROOT / "trunk/user/scutclient/scripts/scutclient.sh"
VLM_SCRIPT = ROOT / "trunk/user/vlmcsd/vlmcsd.sh"


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
    with tempfile.TemporaryDirectory(prefix="scutclient-start-") as tmp:
        base = Path(tmp)
        bin_dir = mock_bin(base / "bin")
        executable(bin_dir / "nvram", """#!/bin/sh
if [ "$1" = get ]; then
    case "$2" in
        scutclient_debug|scutclient_skip_udp_hb) echo 0 ;;
        wan_dns1_x) echo 1.1.1.1 ;;
        *) echo test ;;
    esac
fi
""")
        executable(bin_dir / "start-stop-daemon", """#!/bin/sh
exit "${MOCK_START_STATUS:-0}"
""")
        log = base / "logger.log"
        pidfile = base / "scutclient.pid"
        env = os.environ.copy()
        env.update({
            "PATH": str(bin_dir) + os.pathsep + env.get("PATH", ""),
            "MOCK_LOG": str(log),
            "MOCK_START_STATUS": "29",
            "SCUTCLIENT_PIDFILE": str(pidfile),
        })
        failed = run(["/bin/sh", str(SCUT_SCRIPT), "start"], env)
        assert failed.returncode == 29, (
            f"SCUT must propagate launcher status 29, got "
            f"{failed.returncode}: {failed.stderr}"
        )
        assert "[FAILED]" in failed.stdout
        assert "Daemon launch failed (status=29)" in log.read_text()
        assert not pidfile.exists(), "temporary SCUT pidfile was not cleaned"

        log.write_text("")
        env["MOCK_START_STATUS"] = "0"
        succeeded = run(["/bin/sh", str(SCUT_SCRIPT), "start"], env)
        assert succeeded.returncode == 0, succeeded.stderr
        assert "[  OK  ]" in succeeded.stdout
        assert "Daemon is started" in log.read_text()





IPERF_SCRIPT = ROOT / "trunk/user/iperf3/iperf3.sh"


def check_iperf3():
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
    with tempfile.TemporaryDirectory(prefix="mentohust-lifecycle-") as tmp:
        base = Path(tmp)
        bin_dir = mock_bin(base / "bin")
        executable(bin_dir / "start-stop-daemon", """#!/bin/sh
exit "${MOCK_START_STATUS:-0}"
""")
        executable(bin_dir / "killall", """#!/bin/sh
exit "${MOCK_STOP_STATUS:-0}"
""")
        config = base / "existing-mentohust.conf"
        config.write_text("[MentoHUST]\\n")
        pidfile = base / "mentohust.pid"
        log = base / "logger.log"
        env = os.environ.copy()
        env.update({
            "PATH": str(bin_dir) + os.pathsep + env.get("PATH", ""),
            "MOCK_START_STATUS": "43",
            "MOCK_STOP_STATUS": "47",
            "MOCK_LOG": str(log),
            "MENTOHUST_PIDFILE": str(pidfile),
            "MENTOHUST_BIN_CONF_FILE": str(config),
        })

        failed_start = run(["/bin/sh", str(MENTOHUST_SCRIPT), "start"], env)
        assert failed_start.returncode == 43, (
            f"mentohust must propagate launch status 43, got {failed_start.returncode}"
        )
        assert "[FAILED]" in failed_start.stdout
        assert "Daemon launch failed (status=43)" in log.read_text()
        assert not pidfile.exists(), "temporary mentohust pidfile was not cleaned"

        log.write_text("")
        env["MOCK_START_STATUS"] = "0"
        started = run(["/bin/sh", str(MENTOHUST_SCRIPT), "start"], env)
        assert started.returncode == 0, started.stderr
        assert "[  OK  ]" in started.stdout
        assert "Daemon is started" in log.read_text()

        log.write_text("")
        stopped = run(["/bin/sh", str(MENTOHUST_SCRIPT), "stop"], env)
        assert stopped.returncode == 47, (
            f"mentohust must propagate stop status 47, got {stopped.returncode}"
        )
        assert "[FAILED]" in stopped.stdout
        assert "Daemon stop failed (status=47)" in log.read_text()




TTYD_SCRIPT = ROOT / "trunk/user/ttyd/ttyd.sh"


def check_ttyd():
    with tempfile.TemporaryDirectory(prefix="ttyd-lifecycle-") as tmp:
        base = Path(tmp)
        bin_dir = mock_bin(base / "bin")
        executable(bin_dir / "nvram", """#!/bin/sh
if [ "$1" = get ]; then
    echo 7681
fi
exit 0
""")
        executable(bin_dir / "start-stop-daemon", """#!/bin/sh
exit "${MOCK_START_STATUS:-0}"
""")
        executable(bin_dir / "killall", """#!/bin/sh
exit "${MOCK_STOP_STATUS:-0}"
""")
        env = os.environ.copy()
        env.update({
            "PATH": str(bin_dir) + os.pathsep + env.get("PATH", ""),
            "MOCK_START_STATUS": "53",
            "MOCK_STOP_STATUS": "54",
        })
        failed_start = run(["/bin/sh", str(TTYD_SCRIPT), "start"], env)
        assert failed_start.returncode == 53, (
            f"ttyd script must propagate start-stop-daemon status 53, got {failed_start.returncode}"
        )
        env["MOCK_START_STATUS"] = "0"
        started = run(["/bin/sh", str(TTYD_SCRIPT), "start"], env)
        assert started.returncode == 0, started.stderr
        stopped = run(["/bin/sh", str(TTYD_SCRIPT), "stop"], env)
        assert stopped.returncode == 54, (
            f"ttyd script must propagate killall status 54, got {stopped.returncode}"
        )



SSHD_SCRIPT = ROOT / "trunk/user/openssh/sshd.sh"


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


if __name__ == "__main__":
    check_scutclient()
    check_vlmcsd()
    check_iperf3()
    check_mentohust()
    check_ttyd()
    check_sshd()
    print("optional app launch/lifecycle status regressions: PASS")
