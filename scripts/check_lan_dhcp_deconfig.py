#!/usr/bin/env python3
"""Check retry-safe LAN DHCP deconfig and static hostname restoration."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
lan = (root / "trunk/user/rc/src/net_lan.c").read_text()
policy = (root / "trunk/user/rc/src/dhcp_lease_policy.h").read_text()
test = (root / "trunk/user/rc/src/tests/dhcp_lease_policy.c").read_text()
kernel_policy = (root / "trunk/user/rc/src/lan_kernel_name_policy.h").read_text()
kernel_test = (root / "trunk/user/rc/src/tests/lan_kernel_name_policy.c").read_text()
busybox = (root / "trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c").read_text()
workflow = (root / ".github/workflows/wan-dhcp-callback-stability.yml").read_text()
xy = (root / ".github/workflows/xy-c1-firmware-validation.yml").read_text()

helper_start = policy.index("dhcp_lease_deconfig_address_ready(int route_status, int ifconfig_status)")
helper_end = policy.index("}", helper_start)
helper = policy[helper_start:helper_end]
assert "dhcp_lease_route_delete_is_ok(route_status)" in helper
assert "ifconfig_status == 0" in helper

for marker in (
    "dhcp_lease_deconfig_address_ready(0, 0)",
    "dhcp_lease_deconfig_address_ready(ESRCH, 0)",
    "dhcp_lease_deconfig_address_ready(EIO, 0)",
    "dhcp_lease_deconfig_address_ready(0, EIO)",
):
    assert marker in test, marker

start = lan.index("lan_down_auto(char *lan_ifname)")
end = lan.index("return 0;", start)
cleanup = lan[start:end]
status_update = cleanup.index("status = dhcp_lease_deconfig_status(route_status, ifconfig_status,")
ready = cleanup.index("if (dhcp_lease_deconfig_address_ready(route_status, ifconfig_status))", status_update)
restore = cleanup.index('hosts_status = create_hosts_lan(nvram_safe_get("lan_ipaddr"),', ready)
domain = cleanup.index('nvram_safe_get("lan_domain")', restore)
hosts_error = cleanup.index("if (hosts_status != 0)", domain)
preserve = cleanup.index("if (status == 0)", hosts_error)
failure = cleanup.index("if (status != 0)", preserve)
assert status_update < ready < restore < domain < hosts_error < preserve < failure
assert "status = hosts_status;" in cleanup[preserve:failure]
assert "failed to restore static LAN hostname/hosts during deconfig" in cleanup

for effect in (
    "notify_pause_detect_internet();",
    "update_lan_status(0);",
    'nvram_set_temp("lan_dns_t", "");',
    'nvram_set_temp("lan_wins_t", "");',
    'nvram_set_temp("lan_lease_t", "");',
):
    assert failure < cleanup.index(effect), effect
update = cleanup.index("update_lan_status(0);")
for key in ("lan_dns_t", "lan_wins_t", "lan_lease_t"):
    assert update < cleanup.index('nvram_set_temp("' + key + '", "");')

deconfig_start = lan.index("udhcpc_lan_deconfig(char *lan_ifname)")
deconfig_end = lan.index("udhcpc_lan_bound(", deconfig_start)
deconfig = lan[deconfig_start:deconfig_end]
assert "status = lan_down_auto(lan_ifname);" in deconfig
assert "return status;" in deconfig

assert "#define DECONFIG_PENDING 7" in busybox
pending = busybox.rindex("case DECONFIG_PENDING:")
pending_end = busybox.index("case RELEASED:", pending)
pending_body = busybox[pending:pending_end]
assert 'udhcp_run_script(NULL, "deconfig")' in pending_body
assert "udhcpc_deconfig_retry_required(deconfig_status)" in pending_body
assert "state = deconfig_resume_state;" in pending_body

filters = workflow.split("jobs:", 1)[0]
push, pr = filters.split("  pull_request:", 1)
required = (
    ".github/workflows/wan-dhcp-callback-stability.yml",
    "trunk/user/rc/src/net_lan.c",
    "trunk/user/rc/src/dhcp_lease_policy.h",
    "trunk/user/rc/src/tests/dhcp_lease_policy.c",
    "scripts/check_lan_dhcp_deconfig.py",
)
for path in required:
    assert path in push, "missing push path: " + path
    assert path in pr, "missing pull_request path: " + path
for path in required[1:]:
    assert xy.count('"' + path + '"') == 1, "missing/duplicate XY-C1 path: " + path

# Kernel hostname/domain syscalls are part of the LAN host publication
# contract: both must be attempted and their first errno must reach deconfig.
assert '#include "lan_kernel_name_policy.h"' in lan
assert "typedef int (*lan_kernel_name_setter_t)(const char *name, size_t length);" in kernel_policy
assert "status = ops->set_hostname(hostname, strlen(hostname));" in kernel_policy
assert "status = ops->set_domainname(domainname, strlen(domainname));" in kernel_policy
assert "if (status != 0 && first_status == 0)" in kernel_policy
assert "return EINVAL;" in kernel_policy
for marker in (
    "test_success_updates_both_kernel_names",
    "test_hostname_failure_does_not_skip_domain_update",
    "test_domain_failure_is_reported_after_hostname_update",
    "test_invalid_inputs_fail_before_system_calls",
    "test_missing_setter_fails_closed",
    "LAN kernel hostname/domain status regression: PASS",
):
    assert marker in kernel_test, marker

host_start = lan.index("create_hosts_lan(const char *lan_ipaddr, const char *lan_dname)")
host_end = lan.index("\nint\nupdate_hosts_ap", host_start)
host_body = lan[host_start:host_end]
assert "lan_kernel_names_apply(lan_hname, lan_dname," in host_body
assert "lan_sethostname_status" in lan
assert "sethostname(name, length)" in lan
assert "lan_setdomainname_status" in lan
assert "setdomainname(name, length)" in lan
assert host_body.index("lan_kernel_names_apply") < host_body.index("dnsmasq_lifecycle_lock_release(hosts_lock)")
assert host_body.index("dnsmasq_lifecycle_lock_release(hosts_lock)") < host_body.index("if (saved_errno != 0)")
assert "errno = saved_errno;" in host_body
assert "return saved_errno;" in host_body
assert "sethostname(lan_hname" not in host_body
assert "setdomainname(lan_dname" not in host_body

print("LAN DHCP deconfig static host restoration and retry contract: PASS")
