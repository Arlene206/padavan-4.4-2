#!/usr/bin/env python3
"""Pin checked NVRAM reads before USB printer daemon lifecycle side effects."""
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
RC=ROOT/"trunk/user/rc/src"
SOURCE=(RC/"services_usb.c").read_text()
POLICY=(RC/"usb_printer_nvram_policy.h").read_text()
TEST=(RC/"tests/usb_printer_nvram_policy.c").read_text()
P910_POLICY=(RC/"usb_printer_p910nd_policy.h").read_text()
P910_TEST=(RC/"tests/usb_printer_p910nd_policy.c").read_text()
WORKFLOW=(ROOT/".github/workflows/usb-modem-identity-stability.yml").read_text()
def body(signature):
    start=SOURCE.index(signature); opening=SOURCE.index("{",start); depth=0
    for pos in range(opening,len(SOURCE)):
        if SOURCE[pos]=="{": depth+=1
        elif SOURCE[pos]=="}":
            depth-=1
            if depth==0:return SOURCE[opening+1:pos]
    raise AssertionError("unterminated "+signature)
assert '#include "usb_printer_nvram_policy.h"' in SOURCE
assert "nvram_get_checked(name, value)" in SOURCE
assert "usb_printer_nvram_copy_setting_status(" in SOURCE
assert "usb_printer_nvram_write_status" in POLICY
for marker in ("test_write_status_accepts_success",
               "test_write_status_preserves_backend_errno",
               "test_write_status_without_errno_defaults_to_eio",
               "test_write_status_rejects_unexpected_result"):
    assert marker in TEST and marker + "();" in TEST
u2ec = body("start_u2ec(void)")
marker = u2ec.index('marker_result = nvram_set("apps_u2ec_ex", "1");')
saved = u2ec.index("marker_errno = errno;", marker)
normalize = u2ec.index("usb_printer_nvram_write_status(marker_result, marker_errno)", saved)
failure = u2ec.index("if (marker_status != 0)", normalize)
failure_return = u2ec.index("return marker_status;", failure)
success_return = u2ec.index("return 0;", failure_return)
assert marker < saved < normalize < failure < failure_return < success_return
assert "u2ec started but runtime marker publication failed" in u2ec
assert "if (result < 0)" in POLICY and "EIO" in POLICY
assert "if (result == 0)" in POLICY and "ENAMETOOLONG" in POLICY
for name,key,launch,pid_file in (
    ("start_u2ec(void)","u2ec_enable",
     'start_usb_printer_daemon(argv, "u2ec", "/var/run/u2ec.pid")',
     'unlink("/var/run/u2ec.pid")'),
    ("start_lpd(void)","lprd_enable",
     'start_usb_printer_daemon(argv, "lpd", "/var/run/lpdparent.pid")',
     'unlink("/var/run/lpdparent.pid")')):
    fn=body(name); read=fn.index("read_usb_printer_setting_status(")
    gate=fn.index("if (setting_status != 0)",read)
    prop=fn.index("return setting_status;",gate)
    decision=fn.index('strcmp(setting, "0")',prop)
    effect=fn.index(launch,decision)
    assert f'"{key}"' in fn and read<gate<prop<decision<effect
    assert pid_file not in fn
    assert f'nvram_match("{key}"' not in fn

# The shared launcher must never erase a live PID file or duplicate a daemon.
launcher = body("start_usb_printer_daemon(char *const argv[], const char *process_name,\n                         const char *pid_file)")
scan = launcher.index("usb_printer_daemon_scan((void *)process_name, &scan_error)")
action = launcher.index("process_presence_start_action_for_state(presence)")
status = launcher.index("process_presence_start_status_for_action(")
unknown = launcher.index("if (start_status != 0)", status)
running = launcher.index("if (start_action == PROCESS_PRESENCE_START_NOTIFY)", unknown)
cleanup = launcher.index("unlink(pid_file)", running)
fork_pipe = launcher.index("pipe(exec_pipe)", cleanup)
assert scan<action<status<unknown<running<cleanup<fork_pipe
assert "already running; skipping duplicate launch" in launcher
assert "cannot remove stale %s PID file" in launcher
assert "errno != ENOENT" in launcher
rollback_stop = body("usb_printer_daemon_rollback_stop(void *context)")
assert "kill_services(service_names, 3, 1)" in rollback_stop
recovery = body("usb_printer_daemon_start_failure_recover(const char *process_name,")
assert "detached_daemon_start_rollback_with_ops(" in recovery
assert "usb_printer_daemon_rollback_stop" in recovery
assert "usb_printer_daemon_scan" in recovery
assert "PROCESS_PRESENCE_ABSENT" in recovery
assert "startup rollback incomplete" in recovery
assert "usb_printer_daemon_start_failure_recover(process_name," in launcher
partial_status = launcher.index("/* A partial status write")
partial_reap = launcher.index(
    "usb_printer_reap_failed_exec_child(child, &child_status, waitpid)",
    partial_status,
)
partial_recovery = launcher.index(
    "usb_printer_daemon_start_failure_recover(process_name, EIO)",
    partial_reap,
)
assert partial_status < partial_reap < partial_recovery
raw=body("start_p910nd(char *devlp)")
read=raw.index('read_usb_printer_setting_status("rawd_enable"')
gate=raw.index("if (setting_status != 0)",read)
prop=raw.index("return setting_status;",gate)
disabled=raw.index('strcmp(setting, "0")',prop)
port_prepare=raw.index("usb_printer_p910nd_port_from_device(")
process_name=raw.index("usb_printer_p910nd_process_name(")
mode=raw.index('strcmp(setting, "2")',disabled)
bidir_launch=raw.index('eval("/usr/sbin/p910nd", "-b", "-f", devlp, port)')
single_launch=raw.index('eval("/usr/sbin/p910nd", "-f", devlp, port)')
readiness_port=raw.index("start_status = usb_printer_daemon_postcondition(process_name);")
failure_gate=raw.index("if (start_status != 0)",readiness_port)
rollback=raw.index("usb_printer_daemon_start_failure_recover(",failure_gate)
assert read<gate<prop<disabled<port_prepare<process_name<mode<bidir_launch<readiness_port<failure_gate<rollback
assert mode<single_launch<readiness_port
raw_compact = "".join(raw.split())
assert "usb_printer_daemon_start_failure_recover(process_name,start_status)" in raw_compact
assert "return start_status;" in raw[rollback:]
stop_raw=body("stop_p910nd(void)")
for index in range(10):
    assert f'"p910{index}d"' in stop_raw
assert '"p910nd"' in stop_raw
assert "usb_printer_p910nd_port_from_device" in P910_POLICY
assert "usb_printer_p910nd_process_name" in P910_POLICY
p910_test_compact = "".join(P910_TEST.split())
for marker in ("test_each_supported_device_maps_to_its_port",
               "test_invalid_device_paths_fail_without_mutation",
               "test_process_name_tracks_port_number",
               "test_invalid_process_names_fail_without_mutation"):
    assert marker in P910_TEST and ("CHECK(" + marker + "()==0);") in p910_test_compact
assert 'nvram_match("rawd_enable"' not in raw
for marker in ("test_present_setting_is_copied_from_shared_cache","test_missing_setting_preserves_empty_legacy_default","test_backend_error_is_not_collapsed_to_default","test_errno_less_backend_failure_becomes_eio","test_unexpected_result_and_null_cache_are_rejected","test_overlong_setting_fails_before_use","test_invalid_arguments_are_rejected"):
    assert marker in TEST and marker+"();" in TEST
FILTERS=WORKFLOW.split("permissions:",1)[0]
for path in (".github/workflows/usb-modem-identity-stability.yml","trunk/user/rc/src/services_usb.c","trunk/user/rc/src/usb_printer_nvram_policy.h","trunk/user/rc/src/tests/usb_printer_nvram_policy.c","trunk/user/rc/src/usb_printer_p910nd_policy.h","trunk/user/rc/src/tests/usb_printer_p910nd_policy.c","scripts/check_usb_printer_nvram.py"):
    assert FILTERS.count(path)==2,f"workflow filter missing {path}"
assert "usb_printer_nvram_policy.c" in WORKFLOW
assert "python3 scripts/check_usb_printer_nvram.py" in WORKFLOW
assert "usb_printer_p910nd_policy.c" in WORKFLOW
print("USB printer checked NVRAM lifecycle contract: PASS")
