#!/usr/bin/env python3
"""Source contract for BusyBox PID 1 deferred child reaping."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
INIT_DIR = ROOT / "trunk/user/busybox/busybox-1.24.x/init"
SOURCE = (INIT_DIR / "init.c").read_text()
QUEUE = (INIT_DIR / "init_reap_queue.h").read_text()
TEST = (INIT_DIR / "tests/init_reap_queue.c").read_text()


def section(text, start_marker, end_marker):
    start = text.index(start_marker)
    end = text.index(end_marker, start)
    return text[start:end]


stop_handler = section(
    SOURCE,
    "static void stop_handler(int sig UNUSED_PARAM)",
    "#if ENABLE_FEATURE_USE_INITTAB",
)
assert "wait_any_nohang(NULL)" in stop_handler
assert "init_reap_queue_has_capacity" in stop_handler
assert "init_reap_queue_push" in stop_handler
stop_handler_code = re.sub(r"/\*.*?\*/|//[^\n]*", "", stop_handler, flags=re.DOTALL)
assert "mark_terminated(" not in stop_handler_code
assert "update_utmp_DEAD_PROCESS" not in stop_handler_code
assert stop_handler.index("init_reap_queue_has_capacity") < stop_handler.index(
    "wait_any_nohang(NULL)"
)

reaper = section(
    SOURCE,
    "static void flush_pending_reaped_children(void)",
    "static void waitfor(pid_t pid)",
)
assert "sigprocmask(SIG_BLOCK" in reaper
assert "sigaddset(&tstp_set, SIGTSTP)" in reaper
assert "init_reap_queue_drain" in reaper
assert "mark_terminated(pending_pids[i])" in reaper

waitfor = section(SOURCE, "static void waitfor(pid_t pid)", "/* Run all commands")
assert waitfor.index("kill(pid, 0)") < waitfor.index("wait(NULL)")
assert "errno == ESRCH" in waitfor
assert "wpid < 0 && errno != EINTR" in waitfor
assert "flush_pending_reaped_children();" in waitfor

reload = section(SOURCE, "static void reload_inittab(void)", "static int check_delayed_sigs(void)")
assert reload.index("sigprocmask(SIG_BLOCK") < reload.index("parse_inittab();")
assert reload.index("flush_pending_reaped_children();") < reload.index("parse_inittab();")
assert "sigprocmask(SIG_SETMASK, &old_set, NULL)" in reload

main = SOURCE[SOURCE.index("int init_main("):]
assert main.index("run_actions(SYSINIT);") < main.index("flush_pending_reaped_children();")
wait_actions = main.index("run_actions(WAIT);")
assert wait_actions < main.index("flush_pending_reaped_children();", wait_actions)
assert "run_actions(RESPAWN | ASKFIRST);" in main
assert main.count("flush_pending_reaped_children();") >= 6

assert "#define INIT_REAP_QUEUE_CAPACITY 64" in QUEUE
assert "volatile sig_atomic_t count" in QUEUE
assert "volatile sig_atomic_t pids[INIT_REAP_QUEUE_CAPACITY]" in QUEUE
assert "queue->count = 0;" in QUEUE

for test_name in (
    "test_empty_queue",
    "test_rejects_invalid_pids",
    "test_fifo_drain_and_reuse",
    "test_full_queue_and_short_destination",
    "test_rejects_corrupt_queue_count",
):
    assert f"static void {test_name}(void)" in TEST, test_name
    assert f"{test_name}();" in TEST, test_name

print("BusyBox PID1 deferred reap queue and signal-context contract: PASS")
