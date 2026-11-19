#!/usr/bin/env python3
"""Check PID 1 gates router startup on required boot environment setup."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
init = (root / "trunk/user/rc/src/init.c").read_text()
script = (root / "trunk/user/scripts/files/sbin/dev_init.sh").read_text()
workflow = (root / ".github/workflows/process-lifecycle-regression.yml").read_text()
retry_policy = (root / "trunk/user/rc/src/init_mdev_retry_policy.h").read_text()
retry_test = (root / "trunk/user/rc/src/tests/init_mdev_retry_policy.c").read_text()
shell_test = (root / "trunk/user/scripts/tests/dev_init_boot_policy.sh").read_text()


def shell_function(name):
    start = script.index(name + "()")
    brace = script.index("{", start)
    depth = 0
    for index in range(brace, len(script)):
        if script[index] == "{":
            depth += 1
        elif script[index] == "}":
            depth -= 1
            if depth == 0:
                return script[start:index + 1]
    raise AssertionError("unterminated shell function: " + name)


def c_function(signature):
    start = init.index(signature)
    brace = init.index("{", start)
    depth = 0
    for index in range(brace, len(init)):
        if init[index] == "{":
            depth += 1
        elif init[index] == "}":
            depth -= 1
            if depth == 0:
                return init[start:index + 1]
    raise AssertionError("unterminated C function: " + signature)


mount_policy = shell_function("required_mount")
dir_policy = shell_function("required_dir")
assert 'mount "$@"' in mount_policy and "return 1" in mount_policy
assert 'mkdir -p "$@"' in dir_policy and 'chmod "$dir_mode" "$@"' in dir_policy
assert dir_policy.count("return 1") == 2

normalized = [" ".join(line.split()) for line in script.splitlines()]
required_mounts = (
    "required_mount -t proc proc /proc || exit 1",
    "required_mount -t sysfs sysfs /sys || exit 1",
    "required_mount -t tmpfs tmpfs /dev -o size=8K || exit 1",
    "required_mount -t tmpfs tmpfs /etc -o size=$size_etc,noatime || exit 1",
    "required_mount -t tmpfs tmpfs /tmp -o size=$size_tmp || exit 1",
    "required_mount -t tmpfs tmpfs /var -o size=$size_var || exit 1",
    "required_mount -t devpts devpts /dev/pts || exit 1",
)
for command in required_mounts:
    assert command in normalized, "missing required mount guard: " + command

for path in ("/tmp/rc_notification", "/tmp/rc_action_incomplete",
             "/tmp/.httpd-private"):
    assert any(line.endswith(path + " || exit 1") and
               line.startswith("required_dir 700 ")
               for line in normalized), "missing private directory guard: " + path
assert "required_dir 1777 /tmp || exit 1" in normalized
assert "chmod 700 /tmp/rc_notification /tmp/rc_action_incomplete" not in script
assert "mdev -s || exit 1" in normalized
assert script.rstrip().endswith("exit 0")
assert script.index("/etc/storage/start_script.sh") < script.rindex("exit 0")

loop = c_function("init_main_loop(void)")
assert 'dev_init_status = system("dev_init.sh")' in loop
assert 'dev_init_status = system("dev_init.sh -l")' in loop
assert "if (dev_init_status != 0)" in loop
assert re.search(r"router_ready\s*=\s*\(dev_init_status\s*==\s*0\s*&&\s*nvram_node_result\s*==\s*0\s*&&\s*init_router\(\)\s*==\s*0\)", loop)
assert loop.index("dev_init_status = system") < loop.index("init_router()")
assert '#include "init_mdev_retry_policy.h"' in init
assert "mdev_status = init_mdev();" in loop
assert "init_mdev_retry_state_begin(&mdev_retry_state, dev_init_status" in loop
assert "init_mdev_retry_state_record_result(&mdev_retry_state" in loop
assert loop.count("alarm(INIT_MDEV_RETRY_DELAY_SECONDS)") == 2
assert loop.index("init_signals();") < loop.index("alarm(INIT_MDEV_RETRY_DELAY_SECONDS)")
alarm_block = loop[loop.index("if (signal_state.alarm_pending)"):]
assert alarm_block.index("init_mdev_retry_state_record_result") < alarm_block.index("on_deferred_hotplug_dev")
assert "dev_init.sh already completed mdev -s" in alarm_block
assert "dev_init_status == 0 && mdev_init_status != 0" in retry_policy
for token in ("-EIO", "-EAGAIN", "INIT_MDEV_RETRY_DELAY_SECONDS == 5",
              "PID 1 deferred mdev hotplug recovery policy: PASS"):
    assert token in retry_test
assert "init_mdev_retry_policy.h" in workflow
assert "trunk/user/rc/src/tests/init_mdev_retry_policy.c" in workflow
assert "mount_status=1" in shell_test and "chmod_status=1" in shell_test

print("PID 1 required boot environment gating integration: PASS")
