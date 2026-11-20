#!/usr/bin/env python3
"""Check PID 1 gates router startup on required boot environment setup."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
init = (root / "trunk/user/rc/src/init.c").read_text()
net = (root / "trunk/user/rc/src/net.c").read_text()
net6 = (root / "trunk/user/rc/src/net6.c").read_text()
rc_header = (root / "trunk/user/rc/src/rc.h").read_text()
tcp_policy = (root / "trunk/user/rc/src/tcp_tweaks_policy.h").read_text()
tcp_test = (root / "trunk/user/rc/src/tests/tcp_tweaks_policy.c").read_text()
gai_policy = (root / "trunk/user/rc/src/libc_gai_policy.h").read_text()
gai_test = (root / "trunk/user/rc/src/tests/libc_gai_policy.c").read_text()
rc = (root / "trunk/user/rc/src/rc.c").read_text()
result_policy = (root / "trunk/user/rc/src/init_router_result_policy.h").read_text()
result_test = (root / "trunk/user/rc/src/tests/init_router_result_policy.c").read_text()
script = (root / "trunk/user/scripts/files/sbin/dev_init.sh").read_text()
copyscripts = (root / "trunk/user/scripts/files/usr/bin/copyscripts.sh").read_text()
workflow = (root / ".github/workflows/process-lifecycle-regression.yml").read_text()
retry_policy = (root / "trunk/user/rc/src/init_mdev_retry_policy.h").read_text()
retry_test = (root / "trunk/user/rc/src/tests/init_mdev_retry_policy.c").read_text()
boot_device_policy = (root / "trunk/user/rc/src/boot_device_node_policy.h").read_text()
boot_device_test = (root / "trunk/user/rc/src/tests/boot_device_node_policy.c").read_text()
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


def c_function(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
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

nodes = c_function(init, "init_nodes(int *optional_status)")
node_prepare = c_function(init, "init_optional_device_node(const char *path, dev_t device)")
node_recorder = c_function(init, "init_nodes_record_optional_status(int first_status, const char *path,")
loop = c_function(init, "init_main_loop(void)")
sysctl = c_function(init, "init_sysctl(void)")
sysctl_recorder = c_function(init, "init_sysctl_record_status(")
router = c_function(rc, "init_router(void)")
assert '#include "boot_device_node_policy.h"' in init
assert "mknod(path, S_IFCHR | RC_BOOT_DEVICE_NODE_MODE, device)" in node_prepare
assert "errno != EEXIST" in node_prepare
assert "lstat(path, &st)" in node_prepare
assert "rc_boot_device_node_is_secure(&st, device)" in node_prepare
assert "return -EPERM;" in node_prepare
assert "mknod(\"/dev/hwnat0\"" not in nodes and "mknod(\"/dev/crypto\"" not in nodes
assert "init_optional_device_node(\"/dev/hwnat0\"" in nodes
assert "init_optional_device_node(\"/dev/crypto\"" in nodes
assert "init_openvpn_tun_node()" in nodes
assert "init_nodes_record_optional_status" in nodes
assert "return nvram_result;" in nodes
assert "optional boot device" in node_recorder
assert "if (first_status == 0)" in node_recorder
assert 'nvram_node_result = init_nodes(&optional_node_status);' in loop
assert "if (sysctl_status == 0) sysctl_status = optional_node_status;" in " ".join(loop.split())
assert loop.index("sysctl_status = init_sysctl();") < loop.index("sysctl_status = optional_node_status;") < loop.index("router_init_status = init_router();")
assert "rc_boot_device_node_is_secure" in boot_device_policy
for token in ("S_ISCHR", "st_rdev != expected_device", "st_nlink != 1",
              "(mode & 0600) != 0600", "(mode & 0111) != 0",
              "(mode & 07000) != 0"):
    assert token in boot_device_policy, "missing optional device-node policy: " + token
for token in ("rc_boot_device_node_is_secure(&st, expected)",
              "makedev(220, 0)", "makedev(220, 1)",
              "st.st_uid = 1000", "st.st_gid = 1000",
              "st.st_nlink = 2", "rc_boot_device_node_is_secure(NULL"):
    assert token in boot_device_test, "missing optional node policy regression: " + token
assert 'dev_init_status = system("dev_init.sh -l")' in loop
assert "if (dev_init_status != 0)" in loop
assert "router_ready = (dev_init_status == 0 && nvram_node_result == 0 && init_router_status_is_ready(router_init_status))" in " ".join(loop.split())
assert "router_init_status = INIT_ROUTER_RESULT_FATAL;" in loop
assert "if (dev_init_status == 0 && nvram_node_result == 0)" in loop
assert "router_init_status = init_router();" in loop
assert "sysctl_status = init_sysctl();" in loop
assert "init_router_result_merge_sysctl_status" in loop
assert loop.index("router_init_status = init_router();") < loop.index("init_router_result_merge_sysctl_status")
assert loop.index("init_router_result_merge_sysctl_status") < loop.index("router_ready =")
assert "return first_status;" in sysctl
assert "early boot sysctl initialization failed" in sysctl_recorder
assert "if (first_status == 0)" in sysctl_recorder
for setting in (
    "/proc/sys/kernel/panic",
    "/proc/sys/net/core/rmem_max",
    "/proc/sys/net/core/wmem_max",
    "net.ipv4.conf.all.rp_filter",
    "/proc/sys/vm/min_free_kbytes",
    "/proc/sys/vm/overcommit_memory",
):
    assert setting in sysctl, "missing checked early sysctl: " + setting
assert "status = control_if_ipv6_all(0);" in sysctl
assert "status = set_tcp_tweaks();" in sysctl
assert 'init_sysctl_record_status(first_status, "TCP performance sysctls", status)' in sysctl
assert "int set_tcp_tweaks(void);" in rc_header
assert '#include "tcp_tweaks_policy.h"' in net
tcp_tweaks = c_function(net, "set_tcp_tweaks(void)")
assert "return rc_tcp_tweaks_apply(write_tcp_tweak, NULL);" in tcp_tweaks
for setting in ("tcp_fin_timeout", "tcp_keepalive_intvl", "tcp_keepalive_probes",
                "tcp_keepalive_time", "tcp_retries2", "tcp_syn_retries",
                "tcp_synack_retries", "tcp_tw_reuse", "tcp_rfc1337"):
    assert setting in tcp_policy
assert "test_failures_do_not_skip_later_tweaks_and_preserve_first_error" in tcp_test
assert "assert(writer.calls == RC_TCP_TWEAK_COUNT);" in tcp_test
assert "int set_libc_gai(int ipv4_first);" in rc_header
gai_writer = c_function(net, "write_libc_gai_precedence(void *context)")
gai_remover = c_function(net, "unlink_libc_gai_override(void *context)")
gai_function = c_function(net, "set_libc_gai(int ipv4_first)")
assert 'fput_string("/etc/gai.conf", "precedence ipv4\\n")' in gai_writer
assert 'unlink("/etc/gai.conf")' in gai_remover
assert "return rc_libc_gai_apply(ipv4_first" not in gai_function
assert "status = rc_libc_gai_apply(ipv4_first, write_libc_gai_precedence" in gai_function
assert "status = set_libc_gai(1);" in sysctl
assert 'init_sysctl_record_status(first_status, "/etc/gai.conf", status)' in sysctl
ipv6_init = c_function(net6, "init_ipv6(void)")
ipv6_restart = c_function(net6, "full_restart_ipv6(int ipv6_type_old)")
assert "gai_status = set_libc_gai(" in ipv6_init
assert "rc_first_service_start_status(conf_status, gai_status)" in ipv6_init
assert "operation_status = set_libc_gai(1);" in ipv6_restart
assert "operation_status = set_libc_gai(0);" in ipv6_restart
assert ipv6_restart.count("rc_first_service_start_status(restart_status,") >= 2
assert "test_disable_treats_missing_override_as_success" in gai_test
assert "status == ENOENT ? 0 : status" in gai_policy
assert "init_router_result_merge_sysctl_status" in result_policy
assert "else if (init_router_status_is_degraded(router_init_status))" in loop
assert "PID 1 event handling remains enabled" in loop
assert loop.count("router_init_status = init_router();") == 1
assert "if (router_ready)" in loop and "handle_notifications();" in loop
assert "if (router_ready)" in loop and "on_deferred_hotplug_dev();" in loop
assert '#include "init_router_result_policy.h"' in init
assert '#include "init_router_result_policy.h"' in rc
assert "return init_router_result_from_boot_status(boot_services_status);" in router
assert router.count("return INIT_ROUTER_RESULT_FATAL;") == 5
assert "nvram_setup_status = nvram_convert_misc_values();" in router
wan0_setup_gate = router.split(
    "if (nvram_setup_status != 0)", 1
)[1].split("init_gpio_leds_buttons();", 1)[0]
assert "WAN0 NVRAM setup failed" in wan0_setup_gate
assert "return INIT_ROUTER_RESULT_FATAL;" in wan0_setup_gate
assert "return -1;" not in router
router_compact = "".join(router.split())
assert 'int copyscripts_status = system("/usr/bin/copyscripts.sh");' in router
assert "rc_first_service_start_status(boot_services_status,copyscripts_status)" in router_compact
assert "rc_first_service_start_status(boot_services_status,started_script_status)" in router_compact
assert "rc_first_service_start_status(boot_services_status,autostart_status)" in router_compact
assert router.index('system("/usr/bin/copyscripts.sh")') < router.index('system("/etc/storage/started_script.sh &")')
assert router.index('system("/etc/storage/started_script.sh &")') < router.index('system("/usr/bin/autostart.sh &")')
assert router.index('system("/usr/bin/autostart.sh &")') < router.index("one or more boot service startup operations failed")
assert router.index("one or more boot service startup operations failed") < router.index("return init_router_result_from_boot_status")
assert "copyscripts_ca_status=0" in copyscripts
assert "if ! install_ca_trust_store; then" in copyscripts
assert "copyscripts_ca_status=1" in copyscripts
assert 'exit "$copyscripts_ca_status"' in copyscripts
assert "autostart_status" in router and "started_script_status" in router
assert "init_router_status_is_ready" in result_policy and "init_router_status_is_degraded" in result_policy
for token in ("init_router_result_from_boot_status(0)", "init_router_result_from_boot_status(EIO)", "init_router_result_from_boot_status(-EINTR)", "init_router_status_is_ready(INIT_ROUTER_RESULT_DEGRADED)", "!init_router_status_is_ready(INIT_ROUTER_RESULT_FATAL)", "init_router_status_is_degraded(INIT_ROUTER_RESULT_DEGRADED)", "init_router_result_merge_sysctl_status(INIT_ROUTER_RESULT_READY, EIO)", "init_router_result_merge_sysctl_status(INIT_ROUTER_RESULT_FATAL, EIO)"):
    assert token in result_test, "missing router result regression: " + token
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
push_paths, pull_request_section = workflow.split("  pull_request:", 1)
pull_paths = pull_request_section.split("\njobs:", 1)[0]
for path in ("trunk/user/rc/src/boot_device_node_policy.h",
             "trunk/user/rc/src/tests/boot_device_node_policy.c"):
    assert push_paths.count(path) == 1 and pull_paths.count(path) == 1
push_paths, pull_request_section = workflow.split("  pull_request:", 1)
pull_paths = pull_request_section.split("\njobs:", 1)[0]
for path in ("trunk/user/rc/src/init_router_result_policy.h",
             "trunk/user/rc/src/tests/init_router_result_policy.c",
             "trunk/user/rc/src/tcp_tweaks_policy.h",
             "trunk/user/rc/src/tests/tcp_tweaks_policy.c",
             "trunk/user/rc/src/libc_gai_policy.h",
             "trunk/user/rc/src/tests/libc_gai_policy.c"):
    assert push_paths.count(path) == 1 and pull_paths.count(path) == 1, (
        "Process workflow must watch router result policy exactly once: " + path)
assert "init-router-result-policy" in workflow
assert "tcp-tweaks-policy" in workflow
assert "libc-gai-policy" in workflow
assert "trunk/user/rc/src/tests/init_mdev_retry_policy.c" in workflow
assert "mount_status=1" in shell_test and "chmod_status=1" in shell_test

print("PID 1 required boot environment gating integration: PASS")
