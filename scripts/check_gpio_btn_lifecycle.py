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
policy = read("trunk/user/rc/src/gpio_btn_daemon_policy.h")
test = read("trunk/user/rc/src/tests/gpio_btn_daemon_policy.c")
services = read("trunk/user/rc/src/services.c")
header = read("trunk/user/rc/src/rc.h")
dispatcher = read("trunk/user/rc/src/rc.c")
shutils = read("trunk/user/shared/src/shutils.c")
shutils_header = read("trunk/user/shared/src/shutils.h")
pids = read("trunk/user/shared/src/pids.c")
makefile = read("trunk/user/rc/src/Makefile")
package_makefile = read("trunk/user/rc/Makefile")
workflow = read(".github/workflows/gpio-btn-lifecycle.yml")

assert '#include "gpio_btn_daemon_policy.h"' in gpio
assert "#define GPIO_BTN_START_WAIT_ATTEMPTS 4" in policy
assert "return launch_status;" in policy
assert "return daemon_running ? 0 : ESRCH;" in policy

start = function_body(gpio, "int start_gpio_btn(void)")
existing = start.index('if (pids("gpio_btn"))')
launch = start.index('launch_status = eval("/sbin/gpio_btn");')
launch_error = start.index("if (launch_status != 0)", launch)
grace = start.index("sleep(1);", launch_error)
first_probe = start.index(
    'gpio_btn_start_policy_status(launch_status, pids("gpio_btn"))', grace
)
retry = start.index("attempt < GPIO_BTN_START_WAIT_ATTEMPTS", first_probe)
assert existing < launch < launch_error < grace < first_probe < retry
assert 'return gpio_btn_start_policy_status(launch_status, pids("gpio_btn"));' in start
assert "if (status == 0)" in start
assert "return status;" in start
assert "daemon did not remain running" in start
assert "sleep(1);" in start[retry:]
assert start.count('pids("gpio_btn")') >= 3

main = function_body(gpio, "int btn_main(int argc, char *argv[])")
assert main.index("daemon(0, 0)") < main.index("fd = btn_uevent_init()")
assert main.index("fd = btn_uevent_init()") < main.index('logmessage("gpio_btn", "service started.")')
socket_init = function_body(gpio, "static int btn_uevent_init(void)")
assert "socket(PF_NETLINK" in socket_init
assert "bind(fd," in socket_init
assert "setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO" in socket_init
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
assert "comm_match(p, procName)" in pids
assert "p->state[0] != 'Z'" in pids
assert "strcmp(bb_basename(p->argv0), procName) == 0" in pids

services_start = function_body(services, "start_services_once(int is_ap_mode)")
assert "start_gpio_btn();" in services_start
assert "int  start_gpio_btn(void);" in header
assert '"gpio_btn"' in dispatcher
assert "gpio_btn.o" in makefile
assert '$(LN) rc $(ROMFSDIR)/sbin/gpio_btn' in package_makefile

for marker in (
    "test_running_daemon_satisfies_start",
    "test_missing_daemon_fails_start",
    "test_launcher_error_is_preserved",
    "GPIO button daemon startup postcondition policy: PASS",
):
    assert marker in test, marker

paths = (
    ".github/workflows/gpio-btn-lifecycle.yml",
    "trunk/user/rc/src/gpio_btn.c",
    "trunk/user/rc/src/gpio_btn_daemon_policy.h",
    "trunk/user/rc/src/tests/gpio_btn_daemon_policy.c",
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

print("GPIO button daemon startup and launcher postcondition contracts: PASS")
