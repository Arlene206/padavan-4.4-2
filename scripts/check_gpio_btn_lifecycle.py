#!/usr/bin/env python3
"""Source and call-path contract for GPIO button daemon startup supervision."""
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


gpio = read("trunk/user/rc/src/gpio_btn.c")
btn_action = read("trunk/user/rc/src/btn_action.c")
factory_reset_policy = read("trunk/user/rc/src/factory_reset_policy.h")
factory_reset_test = read("trunk/user/rc/src/tests/factory_reset_policy.c")
storage_script = read("trunk/user/scripts/files/sbin/mtd_storage.sh")
policy = read("trunk/user/rc/src/gpio_btn_daemon_policy.h")
process_inventory = read("trunk/user/shared/src/process_inventory.h")
test = read("trunk/user/rc/src/tests/gpio_btn_daemon_policy.c")
timing_policy = read("trunk/user/rc/src/gpio_btn_timing_policy.h")
timing_test = read("trunk/user/rc/src/tests/gpio_btn_timing_policy.c")
services = read("trunk/user/rc/src/services.c")
header = read("trunk/user/rc/src/rc.h")
dispatcher = read("trunk/user/rc/src/rc.c")
shutils = read("trunk/user/shared/src/shutils.c")
shutils_header = read("trunk/user/shared/src/shutils.h")
makefile = read("trunk/user/rc/src/Makefile")
package_makefile = read("trunk/user/rc/Makefile")
workflow = read(".github/workflows/gpio-btn-lifecycle.yml")

assert '#include "gpio_btn_daemon_policy.h"' in gpio
assert '#include "gpio_btn_timing_policy.h"' in gpio
assert "#include \"process_inventory.h\"" in policy
assert "#define GPIO_BTN_START_WAIT_ATTEMPTS 4" in policy
assert "daemon_presence == PROCESS_PRESENCE_RUNNING" in policy
assert "daemon_presence == PROCESS_PRESENCE_ABSENT" in policy
assert "return EIO;" in policy
assert "PROCESS_PRESENCE_UNKNOWN" in process_inventory
assert "process_snapshot_release" in process_inventory

presence_helper = function_body(gpio, "gpio_btn_process_presence(int *scan_error)")
assert 'process_inventory_scan("gpio_btn", &snapshot)' in presence_helper
assert "*scan_error = snapshot.error;" in presence_helper
assert presence_helper.index("*scan_error = snapshot.error;") < presence_helper.index("process_snapshot_release(&snapshot)")

start = function_body(gpio, "int start_gpio_btn(void)")
existing = start.index("presence = gpio_btn_process_presence(&scan_error);")
running_gate = start.index("presence == PROCESS_PRESENCE_RUNNING", existing)
unknown_gate = start.index("presence == PROCESS_PRESENCE_UNKNOWN", running_gate)
launch = start.index('launch_status = eval("/sbin/gpio_btn");')
launch_error = start.index("if (launch_status != 0)", launch)
grace = start.index("sleep(1);", launch_error)
retry = start.index("attempt < GPIO_BTN_START_WAIT_ATTEMPTS", grace)
first_probe = start.index("presence = gpio_btn_process_presence(&scan_error);", retry)
assert existing < running_gate < unknown_gate < launch < launch_error < grace < retry < first_probe
assert 'gpio_btn_start_policy_status(launch_status, presence)' in start
assert "if (presence == PROCESS_PRESENCE_UNKNOWN)" in start[first_probe:]
unknown_post = start.index("if (presence == PROCESS_PRESENCE_UNKNOWN)", first_probe)
assert start.index("break;", unknown_post) < start.index("sleep(1);", unknown_post)
assert "pids(" not in start
assert "return status;" in start
assert "Button daemon did not remain running" in start
assert "Cannot verify button daemon state" in start

main = function_body(gpio, "int btn_main(int argc, char *argv[])")
assert main.index("daemon(0, 0)") < main.index("fd = btn_uevent_init()")
assert main.index("fd = btn_uevent_init()") < main.index('logmessage("gpio_btn", "service started.")')
socket_init = function_body(gpio, "static int btn_uevent_init(void)")
assert "socket(PF_NETLINK" in socket_init
assert "bind(fd," in socket_init
assert "setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO" in socket_init
assert "tv.tv_usec = GPIO_BTN_HOLD_INTERVAL_US;" in socket_init
assert "exit(1);" in socket_init
assert "exit(EXIT_FAILURE);" in socket_init

# eval() waits for the initial exec'd rc process, while daemon() detaches its
# child. The caller therefore needs an independent process-presence check.
assert '#define eval(cmd, args...)' in shutils_header
assert "_eval(argv, NULL, 0, NULL)" in shutils_header
eval_body = function_body(shutils, "_eval(char *const argv[], char *path, int timeout, int *ppid)")
assert "waitpid(pid, &status, 0)" in eval_body
assert "WEXITSTATUS(status)" in eval_body
assert "daemon(0, 0)" in main

services_start = function_body(services, "start_services_once(int is_ap_mode)")
assert "rc_first_service_start_status(start_status, start_gpio_btn())" in services_start
assert "int  start_gpio_btn(void);" in header
assert '"gpio_btn"' in dispatcher
assert "gpio_btn.o" in makefile
assert '$(LN) rc $(ROMFSDIR)/sbin/gpio_btn' in package_makefile

for marker in (
    "test_running_daemon_satisfies_start",
    "test_missing_daemon_fails_start",
    "test_launcher_error_is_preserved",
    "test_unknown_inventory_fails_closed",
    "PROCESS_PRESENCE_UNKNOWN",
    "GPIO button daemon startup postcondition policy: PASS",
):
    assert marker in test, marker

# The initial press is tick one: include the full 700-799 ms bucket and exclude 800 ms.
assert "#define GPIO_BTN_HOLD_INTERVAL_US 100000" in timing_policy
assert "#define GPIO_BTN_SHORT_MAX_COUNT 8" in timing_policy
update = function_body(gpio, "static void btn_update_press_count")
assert "gpio_btn_elapsed_count(sec, nsec)" in update
assert "gpio_btn_reset_press_count(count)" in update
assert "gpio_btn_long_press_count(count)" in update
press = function_body(gpio, "static void btn_ev_handle")
assert "gpio_btn_elapsed_count(0, 0)" in press
release = function_body(gpio, "static void btn_released_handle")
assert "gpio_btn_short_press_count(count)" in release
assert "gpio_btn_long_press_count(count)" in release
assert "gpio_btn_reset_press_count(count)" in release
assert "CANCEL_COUNT" not in gpio and "LONG_COUNT" not in gpio and "RESET_COUNT" not in gpio
for marker in (
    "test_short_press_deadline",
    "699999999LL",
    "700000000LL",
    "799999999LL",
    "800000000LL",
    "gpio_btn_elapsed_count(2, 999999999LL)",
    "gpio_btn_elapsed_count(3, 0)",
    "gpio_btn_elapsed_count(4, 999999999LL)",
    "gpio_btn_elapsed_count(5, 0)",
    "test_clock_delta_normalization_and_saturation",
    "GPIO button hold timing and threshold policy: PASS",
):
    assert marker in timing_test, marker
assert "gpio_btn_timing_policy.c" in workflow

# A failed flash erase must not commit the NVRAM reset marker or signal PID 1.
assert '#include "factory_reset_policy.h"' in btn_action
erase_storage_body = function_body(dispatcher, "int\nerase_storage(void)")
assert 'return doSystem("/sbin/mtd_storage.sh %s", "erase");' in erase_storage_body
assert "int erase_storage(void);" in header

reset_restore = function_body(btn_action, "btn_reset_restore_rstats(void)")
assert 'nvram_match("rstats_enable", "1")' in reset_restore
assert "start_status = start_rstats();" in reset_restore
assert "if (start_status != 0)" in reset_restore
reset_action = function_body(btn_action, "int\nbtn_reset_action(void)")
storage_call = reset_action.index("storage_status = erase_storage();")
storage_gate = reset_action.index("factory_reset_storage_allows_nvram(storage_status)")
nvram_call = reset_action.index("nvram_status = erase_nvram();")
reboot_gate = reset_action.index("factory_reset_results_allow_reboot(storage_status, nvram_status)")
exit_call = reset_action.index("exit_status = sys_exit();")
assert storage_call < storage_gate < nvram_call < reboot_gate < exit_call
assert reset_action.count("btn_reset_restore_rstats();") == 3
assert "return storage_status;" in reset_action
assert "return nvram_status;" in reset_action
assert "return exit_status;" in reset_action

long_event = function_body(btn_action, "void\nbtn_event_long(int btn_id)")
assert "case 15: // Reset settings" in long_event
assert "btn_reset_action();" in long_event
assert "erase_nvram();" not in long_event and "erase_storage();" not in long_event
reset_release = function_body(gpio, "static void btn_released_handle")
assert "reset_status = btn_reset_action();" in reset_release
assert "if (reset_status != 0)" in reset_release
assert "LED_CONTROL(LED_PWR, get_state_led_pwr());" in reset_release

for marker in (
    "test_storage_failure_blocks_nvram_reset",
    "test_both_persistent_steps_gate_reboot",
    "Factory reset fail-closed sequencing policy: PASS",
):
    assert marker in factory_reset_test, marker

erase_start = storage_script.index("func_erase()")
erase_end = storage_script.index("\nfunc_reset()", erase_start)
erase_script = storage_script[erase_start:erase_end]
erase_branch_start = storage_script.index("\nerase)\n")
erase_branch_end = storage_script.index("\nreset)\n", erase_branch_start)
erase_case = storage_script[erase_branch_start:erase_branch_end]
assert "func_stop_apps" in erase_script
assert "mtd_write erase" in erase_script
assert "result=1" in erase_script
assert "func_erase" in erase_case
assert "exit $result" in storage_script

paths = (
    ".github/workflows/gpio-btn-lifecycle.yml",
    "trunk/user/rc/src/gpio_btn.c",
    "trunk/user/rc/src/btn_action.c",
    "trunk/user/rc/src/factory_reset_policy.h",
    "trunk/user/rc/src/tests/factory_reset_policy.c",
    "trunk/user/scripts/files/sbin/mtd_storage.sh",
    "trunk/user/rc/src/gpio_btn_daemon_policy.h",
    "trunk/user/rc/src/tests/gpio_btn_daemon_policy.c",
    "trunk/user/rc/src/gpio_btn_timing_policy.h",
    "trunk/user/rc/src/tests/gpio_btn_timing_policy.c",
    "trunk/user/rc/src/rc.c",
    "trunk/user/rc/src/rc.h",
    "trunk/user/rc/src/services.c",
    "trunk/user/rc/src/Makefile",
    "trunk/user/rc/Makefile",
    "trunk/user/shared/src/pids.c",
    "trunk/user/shared/src/shutils.c",
    "trunk/user/shared/src/shutils.h",
    "scripts/check_gpio_btn_lifecycle.py",
)
filters = workflow.split("jobs:", 1)[0]
push_filters, pr_filters = filters.split("  pull_request:", 1)
for path in paths:
    quoted = '"' + path + '"'
    assert push_filters.count(quoted) == 1, "push path count: " + path
    assert pr_filters.count(quoted) == 1, "pull_request path count: " + path

misc_stop_source = (ROOT / "trunk/user/rc/src/services.c").read_text()
gpio_source = (ROOT / "trunk/user/rc/src/gpio_btn.c").read_text()
dispatcher_source = (ROOT / "trunk/user/rc/src/rc.c").read_text()

misc_stop = function_body(misc_stop_source, "stop_misc(void)")
shutdown = function_body(dispatcher_source, "shutdown_router(int level)")
button_signal_handler = function_body(gpio_source, "static void sig_handle(int signum)")

assert 'char *svcs[] = { "ntpd", "detect_wan", "watchdog", "gpio_btn", NULL };' in misc_stop
assert "int stop_status = kill_services(svcs, 4, 1);" in misc_stop
assert 'record_service_stop_status(&stop_status, "SkipDBv2", stop_skipdbv2())' in misc_stop
assert misc_stop.index("kill_services(svcs, 4, 1)") < misc_stop.index("stop_skipdbv2()")
assert "return stop_status;" in misc_stop
assert "sigaction(SIGTERM, &sa, NULL)" in gpio_source
assert "running = 0;" in button_signal_handler
assert "misc_stop_status = stop_misc();" in shutdown
assert "misc_stop_status == 0" in shutdown
assert shutdown.index("misc_stop_status = stop_misc();") < shutdown.index("storage_persistence_complete")
print("GPIO button startup, hold timing and fail-closed factory reset contracts: PASS")
