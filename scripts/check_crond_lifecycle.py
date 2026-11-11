#!/usr/bin/env python3
"""Source and call-path contract for BusyBox crond startup and restart safety."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(relative):
    return (ROOT / relative).read_text()


def function_body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    pos = brace
    state = "code"
    while pos < len(source):
        ch = source[pos]
        next_ch = source[pos + 1] if pos + 1 < len(source) else ""
        if state == "code":
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "/" and next_ch == "*":
                state = "block_comment"
                pos += 1
            elif ch == "/" and next_ch == "/":
                state = "line_comment"
                pos += 1
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return source[brace + 1:pos]
        elif state in ("string", "char"):
            if ch == "\\":
                pos += 1
            elif state == "string" and ch == '"':
                state = "code"
            elif state == "char" and ch == "'":
                state = "code"
        elif state == "block_comment":
            if ch == "*" and next_ch == "/":
                state = "code"
                pos += 1
        elif state == "line_comment" and ch == "\n":
            state = "code"
        pos += 1
    raise AssertionError("unterminated function: " + signature)


services = read("trunk/user/rc/src/services.c")
policy = read("trunk/user/rc/src/crond_daemon_policy.h")
test = read("trunk/user/rc/src/tests/crond_daemon_policy.c")
header = read("trunk/user/rc/src/rc.h")
dispatcher = read("trunk/user/rc/src/rc.c")
common = read("trunk/user/rc/src/common_ex.c")
lifecycle = read("trunk/user/rc/src/service_lifecycle_policy.h")
pids = read("trunk/user/shared/src/pids.c")
crond = read("trunk/user/busybox/busybox-1.24.x/miscutils/crond.c")
daemonizer = read("trunk/user/busybox/busybox-1.24.x/libbb/vfork_daemon_rexec.c")
workflow = read(".github/workflows/crond-lifecycle-stability.yml")

assert '#include "crond_daemon_policy.h"' in services
assert "#define CROND_START_INITIAL_WAIT_SECONDS 1" in policy
assert "#define CROND_START_RETRY_WAIT_SECONDS 1" in policy
assert "#define CROND_START_PROBE_ATTEMPTS 4" in policy
assert "return process_present ? 0 : -1;" in policy
assert "return stop_status == 0;" in policy

stop = function_body(services, "stop_crond(void)")
assert "return kill_services(svcs, 3, 1);" in stop

start = function_body(services, "start_crond(void)")
assert 'if (nvram_invmatch("crond_enable", "1"))' in start
assert "return 1;" in start
assert start.index("return 1;") < start.index("launch_status = _eval(crond_argv")
launch_failure = function_body(start, "if (launch_status != 0)")
assert "return crond_start_postcondition(launch_status, 0);" in launch_failure
assert start.index("launch_status = _eval(crond_argv") < start.index(
    "sleep(CROND_START_INITIAL_WAIT_SECONDS)"
) < start.index('pids("crond")')
assert "attempt < CROND_START_PROBE_ATTEMPTS" in start
assert "attempt + 1 < CROND_START_PROBE_ATTEMPTS" in start
assert "sleep(CROND_START_RETRY_WAIT_SECONDS)" in start
assert "crond_start_postcondition(launch_status, pids(\"crond\"))" in start
assert "crond daemon did not remain running after launch" in start
assert "return status;" in start

restart = function_body(services, "restart_crond(void)")
stop_call = restart.index("stop_status = stop_crond();")
gate = restart.index("crond_restart_can_start(stop_status)")
launch = restart.index("return start_crond();")
assert stop_call < gate < launch
assert "refusing crond restart: previous daemon did not stop" in restart
assert "return stop_status;" in restart

boot = function_body(services, "start_services_once(int is_ap_mode)")
assert "start_crond();" in boot
assert "int start_crond(void);" in header
assert "int stop_crond(void);" in header
assert "int restart_crond(void);" in header
assert "stop_crond();" in services

# kill_services forwards the bounded TERM/KILL lifecycle result.
kill = function_body(
    common, "kill_services(char* svc_name[], int wtimeout, int forcekill)"
)
assert "service_lifecycle_stop(svc_name, wtimeout, forcekill, &ops, NULL)" in kill
assert "return result;" in kill
assert "service_lifecycle_wait_for_exit" in lifecycle
assert "if (!forcekill)" in lifecycle

# BusyBox's parent exits after forking, before the child can fail during
# chdir, crontab scanning or pidfile publication.
crond_main = function_body(
    crond, "int crond_main(int argc UNUSED_PARAM, char **argv)"
)
daemon_call = crond_main.index("bb_daemonize_or_rexec(DAEMON_CLOSE_EXTRA_FDS, argv);")
chdir = crond_main.index("xchdir(G.crontab_dir_name)")
rescan = crond_main.index("rescan_crontab_dir();")
pidfile = crond_main.index('write_pidfile(CONFIG_PID_FILE_PATH "/crond.pid");')
assert daemon_call < chdir < rescan < pidfile

daemon_body = function_body(
    daemonizer, "void FAST_FUNC bb_daemonize_or_rexec(int flags, char **argv)"
)
parent_fork = daemon_body.index("if (fork_or_rexec(argv))")
parent_exit = daemon_body.index("exit(EXIT_SUCCESS); /* parent */", parent_fork)
child_setup = daemon_body.index("setsid();", parent_exit)
assert parent_fork < parent_exit < child_setup

# pids() matches comm/argv0 and excludes zombies, giving a live-process check.
assert "comm_match(p, procName)" in pids
assert "strcmp(bb_basename(p->argv0), procName) == 0" in pids
assert "p->state[0] != 'Z'" in pids

init_router = function_body(dispatcher, "init_router(void)")
assert "if (restart_crond() < 0)" in init_router
assert "crond restart after initial crontab update failed" in init_router

crond_notify = dispatcher.split(
    "else if (strcmp(entry->d_name, RCN_RESTART_CROND) == 0)", 1
)[1].split("else if", 1)[0]
assert "action_result = restart_crond();" in crond_notify

time_notify = dispatcher.split(
    "else if (strcmp(entry->d_name, RCN_RESTART_TIME) == 0)", 1
)[1].split("else if", 1)[0]
assert "if (restart_crond() < 0)" in time_notify
assert "action_result = -1;" in time_notify
assert "if (action_result < 0)" in dispatcher
assert "completion marker retained" in dispatcher

for marker in (
    "test_launcher_failure_is_not_hidden",
    "test_detached_child_must_be_present",
    "test_restart_waits_for_complete_stop",
    "crond daemon startup and restart policy: PASS",
):
    assert marker in test, marker

paths = (
    ".github/workflows/crond-lifecycle-stability.yml",
    "trunk/user/rc/src/services.c",
    "trunk/user/rc/src/rc.h",
    "trunk/user/rc/src/rc.c",
    "trunk/user/rc/src/crond_daemon_policy.h",
    "trunk/user/rc/src/tests/crond_daemon_policy.c",
    "trunk/user/rc/src/common_ex.c",
    "trunk/user/rc/src/service_lifecycle_policy.h",
    "trunk/user/shared/src/pids.c",
    "trunk/user/busybox/busybox-1.24.x/miscutils/crond.c",
    "trunk/user/busybox/busybox-1.24.x/libbb/vfork_daemon_rexec.c",
    "scripts/check_crond_lifecycle.py",
)
filters = workflow.split("jobs:", 1)[0]
push_filters, pr_filters = filters.split("  pull_request:", 1)
for path in paths:
    quoted = '"' + path + '"'
    assert push_filters.count(quoted) == 1, "push path count: " + path
    assert pr_filters.count(quoted) == 1, "pull_request path count: " + path

print("crond daemon startup, stop-before-start and RC call-path contracts: PASS")
