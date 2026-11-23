#!/usr/bin/env python3
"""Source contract for BusyBox PID 1 deferred child reaping."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
INIT_DIR = ROOT / "trunk/user/busybox/busybox-1.24.x/init"
SOURCE = (INIT_DIR / "init.c").read_text()
QUEUE = (INIT_DIR / "init_reap_queue.h").read_text()
TEST = (INIT_DIR / "tests/init_reap_queue.c").read_text()
SIGNAL_PENDING = (INIT_DIR / "init_signal_pending.h").read_text()
SIGNAL_TEST = (INIT_DIR / "tests/init_signal_pending.c").read_text()
WORKFLOW = (ROOT / ".github/workflows/busybox-init-reap-queue.yml").read_text()


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
assert "init_wait_for_child(pid, waitpid, &child_reaped)" in waitfor
assert "kill(pid, 0)" not in waitfor
assert "wait(NULL)" not in waitfor
assert "mark_terminated(pid)" in waitfor
assert "waitpid(-1, NULL, WNOHANG)" in waitfor
assert "wpid < 0 && errno == EINTR" in waitfor
assert "flush_pending_reaped_children();" in waitfor
assert "init_wait_for_child" in QUEUE
for test_name in (
    "test_wait_for_exact_child_only",
    "test_wait_for_exact_child_retries_eintr",
    "test_wait_for_exact_child_accepts_signal_handler_reap",
    "test_wait_for_exact_child_propagates_errors",
    "test_wait_for_exact_child_rejects_invalid_inputs",
    "test_wait_for_exact_child_does_not_reap_peer",
):
    assert f"static void {test_name}(void)" in TEST, test_name
    assert f"{test_name}();" in TEST, test_name

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


signal_handler = section(
    SOURCE,
    "static void record_init_delayed_signal(int signo)",
    "static int init_add_delayed_signal(sigset_t *set, int signo)",
)
assert "init_signal_pending_record(&pending_init_signals, signo)" in signal_handler
signal_install = section(
    SOURCE,
    "static int init_install_delayed_signal_handlers(void)",
    "static int check_delayed_sigs(void)",
)
assert "sigfillset(&sa.sa_mask)" in signal_install
assert "sa.sa_flags = 0" in signal_install
assert "bb_signals_recursive_norestart" not in signal_install
for signal_name in ("SIGINT", "SIGQUIT", "SIGUSR1", "SIGTERM", "SIGUSR2"):
    assert f"init_install_one_delayed_signal({signal_name}, &sa)" in signal_install
assert "#ifdef SIGPWR" in signal_install
assert "#if ENABLE_FEATURE_USE_INITTAB" in signal_install

delayed = section(
    SOURCE,
    "static int check_delayed_sigs(void)",
    "#if DEBUG_SEGV_HANDLER",
)
assert delayed.index("sigprocmask(SIG_BLOCK, &pending_set, &old_set)") < delayed.index(
    "init_signal_pending_take(&pending_init_signals"
)
assert delayed.index("init_signal_pending_take(&pending_init_signals") < delayed.index(
    "sigprocmask(SIG_SETMASK, &old_set, NULL)"
)
assert "init_signal_pending_restore(&pending_init_signals" in delayed
assert "bb_got_signal" not in delayed
assert delayed.index("INIT_SIGNAL_PENDING_SHUTDOWN") < delayed.index(
    "INIT_SIGNAL_PENDING_QUIT"
)
assert delayed.index("INIT_SIGNAL_PENDING_QUIT") < delayed.index(
    "INIT_SIGNAL_PENDING_HUP"
)
assert delayed.index("INIT_SIGNAL_PENDING_HUP") < delayed.index(
    "INIT_SIGNAL_PENDING_INT"
)
assert "init_install_delayed_signal_handlers()" in SOURCE
assert "bb_signals_recursive_norestart(0" not in SOURCE
assert "signal(SIGCONT, record_signo)" in stop_handler
assert "volatile sig_atomic_t events" in SIGNAL_PENDING
assert "pending->events |= flag" in SIGNAL_PENDING
assert "pending->events = 0;" in SIGNAL_PENDING
assert "The main loop" in SIGNAL_PENDING
for test_name in (
    "test_collects_distinct_delayed_events",
    "test_coalesces_repeated_events_without_losing_other_events",
    "test_retains_shutdown_mode_and_clears_each_batch",
    "test_restores_a_batch_without_overwriting_newer_shutdown",
    "test_ignores_unhandled_signals_and_rejects_invalid_state",
):
    assert f"static void {test_name}(void)" in SIGNAL_TEST, test_name
    assert f"{test_name}();" in SIGNAL_TEST, test_name
assert "init_signal_pending.h" in WORKFLOW
assert "tests/init_signal_pending.c" in WORKFLOW
assert "Compile and run delayed signal accumulator regression" in WORKFLOW

print("BusyBox PID1 deferred reap queue and signal-context contract: PASS")
