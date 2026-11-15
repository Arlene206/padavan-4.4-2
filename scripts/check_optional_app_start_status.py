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
        failed = run(["sh", str(VLM_SCRIPT), "start"], env)
        assert failed.returncode == 23, (
            f"vlmcsd must propagate launcher status 23, got "
            f"{failed.returncode}: {failed.stderr}"
        )
        assert "launch failed (status=23)" in log.read_text()

        log.write_text("")
        env["MOCK_LAUNCH_STATUS"] = "0"
        succeeded = run(["sh", str(VLM_SCRIPT), "start"], env)
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
        failed = run(["sh", str(SCUT_SCRIPT), "start"], env)
        assert failed.returncode == 29, (
            f"SCUT must propagate launcher status 29, got "
            f"{failed.returncode}: {failed.stderr}"
        )
        assert "[FAILED]" in failed.stdout
        assert "Daemon launch failed (status=29)" in log.read_text()
        assert not pidfile.exists(), "temporary SCUT pidfile was not cleaned"

        log.write_text("")
        env["MOCK_START_STATUS"] = "0"
        succeeded = run(["sh", str(SCUT_SCRIPT), "start"], env)
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

        failed = run(["sh", str(IPERF_SCRIPT), "start"], env)
        assert failed.returncode == 31, (
            f"iperf3 must propagate launcher status 31, got "
            f"{failed.returncode}: {failed.stderr}"
        )
        assert "server launcher failed (status=31)" in log.read_text()
        assert not state.exists()

        env["MOCK_START_STATUS"] = "0"
        started = run(["sh", str(IPERF_SCRIPT), "start"], env)
        assert started.returncode == 0, started.stderr
        assert state.exists()
        assert "server started" in log.read_text()
        starts_before = start_log.read_text().count("-S -b")
        env["MOCK_START_STATUS"] = "31"
        duplicate = run(["sh", str(IPERF_SCRIPT), "start"], env)
        assert duplicate.returncode == 0, duplicate.stderr
        assert start_log.read_text().count("-S -b") == starts_before, (
            "repeated start must not launch a second iperf3 process"
        )

        stopped = run(["sh", str(IPERF_SCRIPT), "stop"], env)
        assert stopped.returncode == 0, stopped.stderr
        assert not state.exists()

        env["MOCK_START_STATUS"] = "0"
        env["MOCK_START_APPEARS"] = "0"
        missing = run(["sh", str(IPERF_SCRIPT), "start"], env)
        assert missing.returncode == 1, missing.stderr
        assert "no server process appeared" in log.read_text()

        state.write_text("running")
        env["MOCK_STOP_STATUS"] = "41"
        stop_failed = run(["sh", str(IPERF_SCRIPT), "stop"], env)
        assert stop_failed.returncode == 41, (
            f"iperf3 must propagate a failed stop signal while still running, "
            f"got {stop_failed.returncode}"
        )
        assert "server stop signal failed (status=41)" in log.read_text()


if __name__ == "__main__":
    check_scutclient()
    check_vlmcsd()
    check_iperf3()
    print("optional app launch/lifecycle status regressions: PASS")
