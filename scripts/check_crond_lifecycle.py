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
wait_policy = read("trunk/user/rc/src/service_wait_policy.h")
wait_test = read("trunk/user/rc/src/tests/service_wait_policy.c")
header = read("trunk/user/rc/src/rc.h")
dispatcher = read("trunk/user/rc/src/rc.c")
common = read("trunk/user/rc/src/common_ex.c")
lifecycle = read("trunk/user/rc/src/service_lifecycle_policy.h")
inventory = read("trunk/user/shared/src/process_inventory.c")
inventory_header = read("trunk/user/shared/src/process_inventory.h")
pids = read("trunk/user/shared/src/pids.c")
crond = read("trunk/user/busybox/busybox-1.24.x/miscutils/crond.c")
daemonizer = read("trunk/user/busybox/busybox-1.24.x/libbb/vfork_daemon_rexec.c")
workflow = read(".github/workflows/crond-lifecycle-stability.yml")

assert '#include "crond_daemon_policy.h"' in services
assert "#define CROND_START_INITIAL_WAIT_SECONDS 1" in policy
assert "#define CROND_START_RETRY_WAIT_SECONDS 1" in policy
assert "#define CROND_START_PROBE_ATTEMPTS 4" in policy
assert '#include <process_inventory.h>' in policy
assert "if (process_presence == PROCESS_PRESENCE_RUNNING)" in policy
assert "return -1;" in policy
assert "return stop_status == 0;" in policy

stop = function_body(services, "stop_crond(void)")
assert "return kill_services(svcs, 3, 1);" in stop

wait_helper = function_body(
    services, "service_start_probe_wait(const char *service_name, unsigned int seconds)"
)
assert "service_wait_failure_status(status)" in wait_helper
assert "errno = status;" not in wait_helper
assert "return status;" not in wait_helper

start = function_body(services, "start_crond(void)")
disabled = start.split('if (nvram_invmatch("crond_enable", "1"))', 1)[1].split(
    "/* Do not spawn a second scheduler", 1
)[0]
assert "return 0;" in disabled
assert "return 1;" not in disabled
launch_failure = function_body(start, "if (launch_status != 0)")
assert 'optional_daemon_rollback_failed_start("crond", -1, launch_errno)' in launch_failure
assert "launch_errno = errno;" in launch_failure
initial_wait = start.index(
    'service_start_probe_wait("crond", CROND_START_INITIAL_WAIT_SECONDS)'
)
initial_wait_failure = start.index("if (status != 0)", initial_wait)
initial_wait_return = start.index('return optional_daemon_rollback_failed_start("crond", status, launch_errno);', initial_wait_failure)
assert start.index("launch_status = _eval(crond_argv") < initial_wait < initial_wait_failure < initial_wait_return < start.index("crond_process_presence(&scan_error)")
assert "attempt < CROND_START_PROBE_ATTEMPTS" in start
assert "attempt + 1 < CROND_START_PROBE_ATTEMPTS" in start
retry_wait = start.index(
    'service_start_probe_wait("crond", CROND_START_RETRY_WAIT_SECONDS)'
)
retry_wait_failure = start.index("if (status != 0)", retry_wait)
assert retry_wait_failure < start.index('return optional_daemon_rollback_failed_start("crond", status, launch_errno);', retry_wait_failure)
presence_helper = function_body(services, "crond_process_presence(int *scan_error)")
assert 'process_inventory_scan("crond", &snapshot)' in presence_helper
assert "*scan_error = snapshot.error;" in presence_helper
assert presence_helper.index("*scan_error = snapshot.error;") < presence_helper.index(
    "process_snapshot_release(&snapshot)"
)
assert "PROCESS_PRESENCE_UNKNOWN" in inventory_header
assert "PROCESS_PRESENCE_UNKNOWN" in inventory
assert "presence = crond_process_presence(&scan_error);" in start
assert "crond_start_postcondition(launch_status, presence)" in start
assert "presence == PROCESS_PRESENCE_RUNNING" in start
unknown = start.index("presence == PROCESS_PRESENCE_UNKNOWN")
assert start.index('return optional_daemon_rollback_failed_start("crond", status, errno);', unknown) < start.index('service_start_probe_wait("crond", CROND_START_RETRY_WAIT_SECONDS)', unknown)
assert 'pids("crond")' not in start
assert "crond daemon did not remain running after launch" in start
assert 'errno = ETIMEDOUT;' in start
assert 'return optional_daemon_rollback_failed_start("crond", status, ETIMEDOUT);' in start

restart = function_body(services, "restart_crond(void)")
stop_call = restart.index("stop_status = stop_crond();")
gate = restart.index("crond_restart_can_start(stop_status)")
launch = restart.index("return start_crond();")
assert stop_call < gate < launch
assert "refusing crond restart: previous daemon did not stop" in restart
assert "return stop_status;" in restart

boot = function_body(services, "start_services_once(int is_ap_mode)")
assert "rc_first_service_start_status(start_status, start_crond())" in boot
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
assert "if (!forcekill || result != SERVICE_LIFECYCLE_TIMEOUT)" in lifecycle

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
assert "int crontab_status = init_crontab();" in init_router
assert "if (init_crontab())" not in init_router
crontab_persist = function_body(
    init_router, "if (rc_crontab_update_requires_persistence(crontab_status))"
)
assert "storage_save_status = write_storage_to_mtd();" in crontab_persist
assert "boot_services_status = rc_first_service_start_status(" in crontab_persist
assert "crond_status = restart_crond();" in crontab_persist
assert "if (crond_status != 0)" in crontab_persist
assert "initial crontab changed but MTD storage persistence failed (status=%d)" in crontab_persist
assert "crond restart after initial crontab update failed (status=%d)" in crontab_persist
assert crontab_persist.index("storage_save_status = write_storage_to_mtd();") < crontab_persist.index(
    "boot_services_status = rc_first_service_start_status("
) < crontab_persist.index("crond_status = restart_crond();")
crontab_error = function_body(init_router, "else if (crontab_status != 0)")
assert "boot_services_status = rc_first_service_start_status(" in crontab_error
assert "initial crontab update failed (status=%d)" in crontab_error
assert init_router.index("int crontab_status = init_crontab();") < init_router.index(
    'one or more boot service startup operations failed'
)

crond_notify = dispatcher.split(
    "else if (strcmp(entry->d_name, RCN_RESTART_CROND) == 0)", 1
)[1].split("else if", 1)[0]
assert "action_result = rc_notify_merge_action_result(" in crond_notify
assert "action_result, restart_crond()" in crond_notify

time_notify = dispatcher.split(
    "else if (strcmp(entry->d_name, RCN_RESTART_TIME) == 0)", 1
)[1].split("else if", 1)[0]
assert "crond_status = restart_crond();" in time_notify
assert "if (crond_status != 0)" in time_notify
assert "action_result, crond_status" in time_notify
assert "if (restart_crond() < 0)" not in dispatcher
assert "if (action_result < 0)" in dispatcher
assert "completion marker retained" in dispatcher

for marker in (
    "test_launcher_failure_is_not_hidden",
    "test_detached_child_must_be_present",
    "test_restart_waits_for_complete_stop",
    "crond daemon startup and restart policy: PASS",
):
    assert marker in test, marker

assert "service_wait_failure_status" in wait_policy
assert "service_wait_failure_status(EACCES)" in wait_test
assert "service_wait_failure_status(0)" in wait_test
assert "service_wait_failure_status(-1)" in wait_test

paths = (
    ".github/workflows/crond-lifecycle-stability.yml",
    "trunk/user/rc/src/services.c",
    "trunk/user/rc/src/rc.h",
    "trunk/user/rc/src/rc.c",
    "trunk/user/rc/src/crond_daemon_policy.h",
    "trunk/user/rc/src/tests/crond_daemon_policy.c",
    "trunk/user/rc/src/common_ex.c",
    "trunk/user/rc/src/service_lifecycle_policy.h",
    "trunk/user/shared/src/process_inventory.c",
    "trunk/user/shared/src/process_inventory.h",
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
