#!/usr/bin/env python3
"""Check retry-safe LAN DHCP deconfig and static hostname restoration."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
lan = (root / "trunk/user/rc/src/net_lan.c").read_text()
wan = (root / "trunk/user/rc/src/net_wan.c").read_text()
policy = (root / "trunk/user/rc/src/dhcp_lease_policy.h").read_text()
test = (root / "trunk/user/rc/src/tests/dhcp_lease_policy.c").read_text()
kernel_policy = (root / "trunk/user/rc/src/lan_kernel_name_policy.h").read_text()
kernel_test = (root / "trunk/user/rc/src/tests/lan_kernel_name_policy.c").read_text()
callback_policy = (root / "trunk/user/rc/src/lan_dhcp_callback_policy.h").read_text()
callback_test = (root / "trunk/user/rc/src/tests/lan_dhcp_callback_policy.c").read_text()
busybox = (root / "trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c").read_text()
workflow = (root / ".github/workflows/wan-dhcp-callback-stability.yml").read_text()
xy = (root / ".github/workflows/xy-c1-firmware-validation.yml").read_text()


def c_function_body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:position]
    raise AssertionError("unterminated C function: " + signature)


def compact(source):
    return " ".join(source.split())


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

# A committed DHCP lease must not become permanently stuck with stale hosts
# after one failed ancillary write: immediate PID 1 retry plus same-value
# renewal retry are both required, without repeating unrelated side effects.
bound = c_function_body(lan, "udhcpc_lan_bound(char *lan_ifname, int is_renew)")
publish = c_function_body(lan, "lan_dhcp_publish_hosts_and_schedule_retry(")
update_ap = c_function_body(lan, "update_hosts_ap(void)")
assert "refresh_hosts = lan_dhcp_hosts_refresh_required( is_changed, is_renew," in compact(bound)
assert 'get_ap_mode() && nvram_match("lan_hosts_retry", "1")' in bound
assert "if (refresh_hosts)" in bound
assert "lan_dhcp_publish_hosts_and_schedule_retry(" in bound
assert bound.index("if (refresh_hosts)") < bound.index("lan_dhcp_publish_hosts_and_schedule_retry(")
assert "return lan_dhcp_bound_callback_status(0, hosts_status);" in bound

# Publish all seven volatile DHCP lease tuples under the shared NVRAM lock.
assert '#include <nvram_linux.h>' in lan
nvram_setter = c_function_body(
    lan, "lan_dhcp_nvram_transaction_set("
)
assert "nvram_transaction_set(transaction, name, value, 1)" in nvram_setter
assert "return errno ? errno : EIO;" in nvram_setter
nvram_restore = c_function_body(lan, "lan_dhcp_nvram_restore_values(")
assert "nvram_transaction_begin(&transaction)" in nvram_restore
assert "dhcp_lease_nvram_restore(values, count," in nvram_restore
assert "nvram_transaction_end(&transaction)" in nvram_restore
assert "dhcp_lease_nvram_restore(" in policy
assert "for (i = count; i > 0; --i)" in policy
assert "values[i - 1].previous_value" in policy
assert "dhcp_lease_nvram_restore(values, 3" in test
assert "fake.fail_restore_after_write = 1;" in test
nvram_publish = c_function_body(
    lan, "lan_dhcp_nvram_state_publish("
)
assert "nvram_transaction_begin(&transaction)" in nvram_publish
assert "dhcp_lease_nvram_publish(values, count," in nvram_publish
assert "nvram_transaction_end(&transaction)" in nvram_publish
assert "lan_dhcp_nvram_restore_values(" in nvram_publish
assert "struct dhcp_lease_nvram_value" in policy
assert "for (j = i + 1; j > 0; --j)" in policy
assert "values[j - 1].previous_value" in policy
for marker in (
    "test_lan_dhcp_nvram_publish_compensates_partial_updates",
    'assert(strcmp(fake.events, "ABCcba") == 0)',
    "assert(rollback_status == EIO);",
):
    assert marker in test, marker
nvram_state_call = bound.index("status = lan_dhcp_nvram_state_publish(")
assert "network_transaction_applied = 1;" in bound[:nvram_state_call]
assert 'lease_state[6].name = "lan_lease_t";' in bound
for key in (
    "lan_ipaddr_t", "lan_netmask_t", "lan_gateway_t", "lan_dns_t",
    "lan_domain_t", "lan_wins_t", "lan_lease_t",
):
    assert key in bound
nvram_failure = bound[nvram_state_call:]
assert "if (status != 0)" in nvram_failure
assert "dhcp_lease_bound_apply(" in nvram_failure
assert "old_ipaddr, old_netmask, old_gateway" in nvram_failure
assert "context.lan_gateway = old_gateway;" in nvram_failure
assert "context.lan_domain = old_domain;" in nvram_failure
assert "context.lan_dns = old_dns;" in nvram_failure
assert "LAN DHCP NVRAM restoration incomplete" in nvram_failure
assert "LAN DHCP kernel/resolver rollback failed" in nvram_failure
assert nvram_failure.index("if (status != 0)") < nvram_failure.index("dhcp_lease_bound_apply(") < nvram_failure.index("return status;")
assert 'nvram_set_temp("lan_ipaddr_t", lease_ip)' not in bound
iappd_start = bound.index('int iappd_status = system("/usr/bin/iappd.sh restart");')
iappd_failure = bound.index("if (iappd_status != 0)", iappd_start)
iappd_log = bound.index("IAPP daemon restart after lease update failed", iappd_failure)
networkmap_gate = bound.index("if (!is_renew)", iappd_log)
networkmap_start = bound.index("int networkmap_status = restart_networkmap();", networkmap_gate)
networkmap_failure = bound.index("if (networkmap_status != 0)", networkmap_start)
networkmap_log = bound.index("networkmap refresh after lease update failed", networkmap_failure)
callback_return = bound.index("return lan_dhcp_bound_callback_status(0, hosts_status);", networkmap_log)
assert iappd_start < iappd_failure < iappd_log < networkmap_gate < networkmap_start < networkmap_failure < networkmap_log < callback_return
assert bound.count('system("/usr/bin/iappd.sh restart")') == 1
assert bound.count("restart_networkmap()") == 1
assert "test_successful_lease_commit_ignores_ancillary_refresh_failure" in callback_test
assert publish.count('nvram_set_int_temp("lan_hosts_retry", "1")') == 2
assert publish.count("notify_rc_status(RCN_RESTART_DHCPD)") == 2
assert "marker_status = nvram_set_int_temp" in publish
assert "notification_status = notify_rc_status(RCN_RESTART_DHCPD);" in publish
assert "marker_status = 0" not in publish
assert "notification_status = 0" not in publish
assert "lan_dhcp_retry_schedule_status(" in publish
assert "no AP hostname-file recovery retry could be scheduled" in publish
assert "retrying" in publish
assert "AP hostname-file retry marker remains unavailable" in publish
assert "AP hostname-file recovery remains unqueued" in publish
assert "lan_dhcp_retry_schedule_status(int marker_status, int notification_status)" in callback_policy
assert "lan_dhcp_retry_schedule_status(EIO, 0) == 0" in callback_test
assert "lan_dhcp_retry_schedule_status(0, ENOSPC) == 0" in callback_test
assert "lan_dhcp_retry_schedule_status(EIO, ENOSPC) == EIO" in callback_test
assert publish.index('nvram_set_int_temp("lan_hosts_retry", "1")') < publish.index("notify_rc_status(RCN_RESTART_DHCPD)")
assert 'nvram_set_int_temp("lan_hosts_retry", "0")' in update_ap
assert update_ap.index("if (status == 0)") < update_ap.index('nvram_set_int_temp("lan_hosts_retry", "0")')
assert "lan_dhcp_hosts_refresh_required(int lease_changed, int is_renew," in callback_policy
assert "test_hosts_refresh_retries_only_a_pending_same_value_renew" in callback_test
assert "lan_dhcp_hosts_refresh_required(0, 1, 0) == 0" in callback_test
assert "lan_dhcp_hosts_refresh_required(0, 1, 1) == 1" in callback_test

assert "#define DECONFIG_PENDING 7" in busybox
pending = busybox.rindex("case DECONFIG_PENDING:")
pending_end = busybox.index("case RELEASED:", pending)
pending_body = busybox[pending:pending_end]
assert 'udhcp_run_script(NULL, "deconfig")' in pending_body
assert "udhcpc_deconfig_retry_required(deconfig_status)" in pending_body
assert "state = deconfig_resume_state;" in pending_body

# A pending DHCP deconfig belongs to udhcpc: do not SIGKILL it before its
# built-in retry. Isolate DHCP stop policy from ordinary daemon escalation.
wan_stop = compact(c_function_body(wan, "stop_wan_locked(void)"))
wan_dhcp_stop = 'kill_services_with_pidfiles(NULL, udhcpc_pidfiles, "udhcpc", 0, 30, 0)'
wan_daemon_stop = "kill_services(svcs_wan, 3, 1)"
assert wan_dhcp_stop in wan_stop
assert wan_daemon_stop in wan_stop
assert wan_stop.index('process_pidfile_signal(udhcpc_pidfiles[0], "udhcpc", SIGUSR2)') < wan_stop.index(wan_dhcp_stop)
assert wan_stop.index('process_pidfile_signal(udhcpc_pidfiles[1], "udhcpc", SIGUSR2)') < wan_stop.index(wan_dhcp_stop)
assert wan_stop.index(wan_dhcp_stop) < wan_stop.index(wan_daemon_stop)

wan_pidfile_stop = compact(c_function_body(wan, "stop_udhcpc_pidfile(const char *path)"))
assert 'kill_services_with_pidfiles(NULL, pidfiles, "udhcpc", 0, 30, 0)' in wan_pidfile_stop

lan_stop = compact(c_function_body(lan, "stop_lan(int is_ap_mode)"))
lan_dhcp_stop = "stop_udhcpc_lan()"
assert lan_dhcp_stop in lan_stop
assert "kill_services(svcs, 3, 1)" in lan_stop
assert lan_stop.index(lan_dhcp_stop) < lan_stop.index("kill_services(svcs, 3, 1)")

lan_pidfile_stop = compact(c_function_body(lan, "stop_udhcpc_lan(void)"))
assert 'process_pidfile_signal(pidfiles[0], "udhcpc", SIGUSR2)' in lan_pidfile_stop
assert "errno != ENOENT && errno != ESRCH" in lan_pidfile_stop
assert '#include "service_wait_policy.h"' in lan
assert "service_wait_microseconds(300000U)" in lan_pidfile_stop
assert "if (wait_status != 0)" in lan_pidfile_stop
assert "wait_status = errno > 0 ? errno : EIO;" in lan_pidfile_stop
assert "refusing LAN DHCP stop: deconfig grace wait failed" in lan_pidfile_stop
assert "errno = wait_status;" in lan_pidfile_stop
assert "return wait_status;" in lan_pidfile_stop
assert "usleep(300000)" not in lan_pidfile_stop
assert lan_pidfile_stop.index("process_pidfile_signal") < lan_pidfile_stop.index("service_wait_microseconds(300000U)") < lan_pidfile_stop.index("if (wait_status != 0)") < lan_pidfile_stop.index("refusing LAN DHCP stop: deconfig grace wait failed") < lan_pidfile_stop.index('kill_services_with_pidfiles(NULL, pidfiles, "udhcpc", 0, 30, 0)')

# WAN stop covers both WAN/VIPTV clients in one PID-file batch; USB WAN and
# AP LAN use one dedicated helper each.
assert (wan + lan).count('"udhcpc", 0, 30, 0') == 3
assert '"udhcpc", 0, 3, 1' not in (wan + lan)

# SIGTERM jumps to ret0. If a deconfig is pending, ret0 retries it rather
# than letting the client exit immediately; default -A is 20 seconds.
assert 'int tryagain_timeout = 20;' in busybox
sigterm_start = busybox.index("case SIGTERM:")
sigterm_end = busybox.index("/* Is it a packet? */", sigterm_start)
assert "goto ret0;" in busybox[sigterm_start:sigterm_end]
ret0 = busybox[busybox.index(" ret0:"):]
assert "state == DECONFIG_PENDING" in ret0
assert 'udhcp_run_script(NULL, "deconfig")' in ret0
assert "while (udhcpc_deconfig_retry_required(release_status))" in ret0
assert "sleep(retry_delay);" in ret0

filters = workflow.split("jobs:", 1)[0]
push, pr = filters.split("  pull_request:", 1)
required = (
    ".github/workflows/wan-dhcp-callback-stability.yml",
    "trunk/user/rc/src/net_lan.c",
    "trunk/user/rc/src/dhcp_lease_policy.h",
    "trunk/user/rc/src/tests/dhcp_lease_policy.c",
    "scripts/check_lan_dhcp_deconfig.py",
)
callback_required = (
    "trunk/user/rc/src/lan_dhcp_callback_policy.h",
    "trunk/user/rc/src/tests/lan_dhcp_callback_policy.c",
)
for path in required + callback_required:
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
assert "!lan_hname || !is_valid_hostname(lan_hname)" in host_body
assert "lan_kernel_names_apply(lan_hname, lan_dname," in host_body
assert "lan_sethostname_status" in lan
assert "sethostname(name, length)" in lan
assert "lan_setdomainname_status" in lan
assert "setdomainname(name, length)" in lan
assert host_body.index("lan_kernel_names_apply") < host_body.index("dnsmasq_lifecycle_lock_release(hosts_lock)")
apply_status = host_body.index("lan_kernel_names_apply")
lock_release = host_body.index("dnsmasq_lifecycle_lock_release(hosts_lock)")
first_error_log = host_body.index("if (saved_errno != 0)", apply_status)
final_error = host_body.index("if (saved_errno != 0) {", lock_release)
assert apply_status < first_error_log < lock_release < final_error
assert "errno = saved_errno;" in host_body
assert "return saved_errno;" in host_body
assert "sethostname(lan_hname" not in host_body
assert "setdomainname(lan_dname" not in host_body

print("LAN DHCP deconfig static host restoration and retry contract: PASS")
