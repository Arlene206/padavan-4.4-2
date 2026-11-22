#!/usr/bin/env python3
"""Check retry-safe LAN DHCP deconfig and static hostname restoration."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
lan = (root / "trunk/user/rc/src/net_lan.c").read_text()
bridge_policy = (root / "trunk/user/rc/src/bridge_init_policy.h").read_text()
bridge_test = (root / "trunk/user/rc/src/tests/bridge_init_policy.c").read_text()
assert '#include "route_status_policy.h"' in lan
wan = (root / "trunk/user/rc/src/net_wan.c").read_text()
policy = (root / "trunk/user/rc/src/dhcp_lease_policy.h").read_text()
test = (root / "trunk/user/rc/src/tests/dhcp_lease_policy.c").read_text()
kernel_policy = (root / "trunk/user/rc/src/lan_kernel_name_policy.h").read_text()
kernel_test = (root / "trunk/user/rc/src/tests/lan_kernel_name_policy.c").read_text()
callback_policy = (root / "trunk/user/rc/src/lan_dhcp_callback_policy.h").read_text()
callback_test = (root / "trunk/user/rc/src/tests/lan_dhcp_callback_policy.c").read_text()
udhcpc_start_header = (root / "trunk/user/rc/src/udhcpc_start_readiness.h").read_text()
rc_header = (root / "trunk/user/rc/src/rc.h").read_text()
nvram_snapshot_policy = (root / "trunk/user/rc/src/wan_nvram_status_policy.h").read_text()
nvram_snapshot_test = (root / "trunk/user/rc/src/tests/wan_nvram_status_policy.c").read_text()
busybox = (root / "trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c").read_text()
workflow = (root / ".github/workflows/wan-dhcp-callback-stability.yml").read_text()
xy = (root / ".github/workflows/xy-c1-firmware-validation.yml").read_text()


def c_function_body(source, signature):
    # A static helper may have an earlier forward declaration. Skip that
    # prototype instead of treating the next function's opening brace as its
    # body, then extract the actual definition.
    search_from = 0
    while True:
        start = source.index(signature, search_from)
        opening = source.index("{", start)
        terminator = source.find(";", start)
        if terminator >= 0 and terminator < opening:
            search_from = start + len(signature)
            continue
        break

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

snapshot_helper = c_function_body(lan, "lan_dhcp_deconfig_snapshot(")
assert "rc_wan_nvram_copy_snapshot(" in snapshot_helper
assert "lan_dhcp_snapshot_begin" in snapshot_helper
assert "lan_dhcp_snapshot_get" in snapshot_helper
assert "lan_dhcp_snapshot_end" in snapshot_helper
for key in (
    "lan_gateway_t", "lan_ipaddr", "lan_netmask", "lan_gateway", "lan_domain",
    "lan_ipaddr_t", "lan_netmask_t", "lan_domain_t", "lan_dns_t",
    "lan_wins_t", "lan_lease_t", "lan_dns1", "lan_dns2",
):
    assert '"' + key + '"' in snapshot_helper
assert "struct rc_wan_nvram_snapshot_field fields[13];" in snapshot_helper
assert 'fields[10].param_name = "lan_lease_t";' in snapshot_helper
assert 'fields[11].param_name = "lan_dns1";' in snapshot_helper
assert 'fields[12].param_name = "lan_dns2";' in snapshot_helper
snapshot_policy_body = c_function_body(
    nvram_snapshot_policy, "rc_wan_nvram_copy_snapshot("
)
assert "rc_wan_nvram_copy_value(" in snapshot_policy_body
assert "end_transaction(context)" in snapshot_policy_body
assert "fields[i].destination[0] = '\\0';" in snapshot_policy_body
for marker in (
    "test_transactional_snapshot_copies_coherent_tuple",
    "test_transactional_snapshot_releases_lock_and_discards_partial_tuple",
    "test_transactional_snapshot_end_failure_invalidates_values",
):
    assert marker in nvram_snapshot_test

start = lan.index("lan_down_auto(char *lan_ifname)")
end = lan.index("return 0;", start)
cleanup = lan[start:end]
snapshot_call = cleanup.index("snapshot_status = lan_dhcp_deconfig_snapshot(")
snapshot_failure = cleanup.index("if (snapshot_status != 0)", snapshot_call)
route_delete = cleanup.index("route_status = route_del(", snapshot_failure)
static_restore = cleanup.index("ifconfig_status = ifconfig(lan_ifname, IFUP,")
status_update = cleanup.index(
    "status = dhcp_lease_deconfig_status(route_status, ifconfig_status, 0, 0);"
)
static_gateway_restore = cleanup.index(
    "static_gateway_status = route_status_idempotent_add(", status_update
)
static_gateway_add = cleanup.index(
    'route_add(lan_ifname, 0, "0.0.0.0",',
    static_gateway_restore,
)
static_gateway_arg = cleanup.index("snapshot.static_gateway", static_gateway_add)
resolver_stage = cleanup.index('lock = file_lock("resolv");', static_gateway_arg)
assert static_gateway_add < static_gateway_arg < resolver_stage
final_status_update = cleanup.index(
    "status = dhcp_lease_deconfig_status(route_status, ifconfig_status,",
    resolver_stage,
)
ready = cleanup.index("if (dhcp_lease_deconfig_address_ready(route_status, ifconfig_status))", final_status_update)
restore = cleanup.index("hosts_status = create_hosts_lan(snapshot.static_address,", ready)
hosts_error = cleanup.index("if (hosts_status != 0)", restore)
preserve = cleanup.index("if (status == 0)", hosts_error)
failure = cleanup.index("if (status != 0)", preserve)
assert snapshot_call < snapshot_failure < route_delete < static_restore < status_update < static_gateway_restore < static_gateway_add < resolver_stage < final_status_update < ready < restore < hosts_error < preserve < failure
assert "if (is_valid_ipv4(snapshot.static_gateway))" in cleanup
assert "failed to restore static LAN gateway route" in cleanup
assert "status = hosts_status;" in cleanup[preserve:failure]
for key in ("lan_gateway_t", "lan_ipaddr", "lan_netmask", "lan_domain"):
    assert 'nvram_safe_get("' + key + '")' not in cleanup
assert "failed to restore static LAN hostname/hosts during deconfig" in cleanup
assert "dnsmasq_valid_server_domain(snapshot.static_domain)" in cleanup
assert "dhcp_lease_static_resolver_uses_gateway(" in cleanup
assert "snapshot.static_dns1" in cleanup and "snapshot.static_dns2" in cleanup
assert "if (resolver_status != 0)" in cleanup and "fclose(fp)" in cleanup
resolver_domain_write = cleanup.index('fprintf(fp, "domain %s\\n", snapshot.static_domain);')
resolver_dns1_write = cleanup.index('fprintf(fp, "nameserver %s\\n", snapshot.static_dns1);')
resolver_dns2_write = cleanup.index('fprintf(fp, "nameserver %s\\n", snapshot.static_dns2);')
resolver_gateway_fallback = cleanup.index(
    'fprintf(fp, "nameserver %s\\n",', resolver_dns2_write
)
resolver_publish = cleanup.index("dnsmasq_file_publish(fp, resolv_temp,", resolver_gateway_fallback)
assert resolver_domain_write < resolver_dns1_write < resolver_dns2_write < resolver_gateway_fallback < resolver_publish
assert "dhcp_lease_static_resolver_uses_gateway(0, 1)" in test
assert "!dhcp_lease_static_resolver_uses_gateway(1, 1)" in test

metadata_prepare = cleanup.index("lan_dhcp_deconfig_metadata_prepare(")
metadata_publish = cleanup.index("metadata_status = lan_dhcp_nvram_state_publish(", metadata_prepare)
metadata_failure = cleanup.index("if (metadata_status != 0)", metadata_publish)
metadata_return = cleanup.index("return metadata_status;", metadata_failure)
monitor_pause = cleanup.index("(void)notify_pause_detect_internet();", metadata_failure)
assert failure < metadata_prepare < metadata_publish < metadata_failure < metadata_return < monitor_pause
assert "LAN_DHCP_DECONFIG_METADATA_COUNT" in cleanup
assert "snapshot.lease_gateway" in cleanup
assert "metadata_rollback_status" in cleanup
assert "LAN DHCP deconfig metadata rollback incomplete" in cleanup
assert "update_lan_status(0);" not in cleanup
assert "nvram_set_temp(" not in cleanup
for key in ("lan_dns_t", "lan_wins_t", "lan_lease_t"):
    assert 'nvram_set_temp("' + key + '", "")' not in cleanup
assert "lan_dhcp_deconfig_metadata_prepare" in callback_policy
assert '#include "dhcp_lease_policy.h"' in callback_policy
assert "test_deconfig_metadata_restores_static_values_and_clears_dhcp_options" in callback_test
assert "static_gateway_status != 0" in policy
assert "return static_gateway_status;" in policy
assert "route_status_idempotent_add(EEXIST) == 0" in test
assert "route_status_idempotent_add(EHOSTUNREACH) == EHOSTUNREACH" in test
viptv_deconfig = c_function_body(policy, "dhcp_lease_viptv_deconfig_apply(")
assert "dhcp_lease_deconfig_status(route_status, interface_status, 0, 0)" in compact(viptv_deconfig)
assert "dhcp_lease_deconfig_status(route_status, interface_status, 0);" not in viptv_deconfig

deconfig_start = lan.index("udhcpc_lan_deconfig(char *lan_ifname)")
deconfig_end = lan.index("udhcpc_lan_bound(", deconfig_start)
deconfig = lan[deconfig_start:deconfig_end]
assert "status = lan_down_auto(lan_ifname);" in deconfig
assert "return status;" in deconfig

# Detached udhcpc clients must publish a live PID/start_time identity after
# their own backgrounding fork; the short-lived _eval() parent is insufficient.
lan_dhcp_start = c_function_body(lan, "start_udhcpc_lan(char *lan_ifname)")
wan_dhcp_start = c_function_body(wan, "start_udhcpc_wan(char *wan_ifname, int unit, int wait_lease)")
viptv_dhcp_start = c_function_body(wan, "start_udhcpc_viptv(char *man_ifname)")
assert 'udhcpc_start_and_verify(dhcp_argv, "/var/run/udhcpc_lan.pid"' in compact(lan_dhcp_start)
assert "udhcpc_start_and_verify(dhcp_argv, pidfile, wan_ifname, log_prefix)" in compact(wan_dhcp_start)
assert 'udhcpc_start_and_verify(dhcp_argv, "/var/run/udhcpc_viptv.pid"' in compact(viptv_dhcp_start)
udhcpc_helper = c_function_body(udhcpc_start_header, "udhcpc_start_and_verify(")
assert 'process_pidfile_read_identity(pidfile, "udhcpc", &identity)' in compact(udhcpc_helper)
assert "process_presence_start_postcondition( launch_status, presence)" in compact(udhcpc_helper)
assert "PROCESS_PRESENCE_START_PROBE_ATTEMPTS" in udhcpc_helper
assert "PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS" in udhcpc_helper
assert "PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS" in udhcpc_helper
assert "presence == PROCESS_PRESENCE_UNKNOWN" in udhcpc_helper
assert "return ETIMEDOUT;" in udhcpc_helper
background = busybox.index("if (!(opt & OPT_f) && (opt & OPT_d) )")
pidfile_publish = busybox.index(
    "if (write_udhcpc_pidfile(client_config.pidfile) != 0)", background
)
assert background < pidfile_publish
assert "bb_daemonize(0);" in busybox[background:pidfile_publish]
client_background = c_function_body(busybox, "client_background(void)")
assert "bb_daemonize(0);" in client_background
assert "write_udhcpc_pidfile(client_config.pidfile)" in client_background
assert 'dhcp_argv[index++] = "-b";' in wan_dhcp_start
assert 'dhcp_argv[index++] = "-d";' in wan_dhcp_start
leasefail = c_function_body(lan, "udhcpc_lan_leasefail(char *lan_ifname)")
assert 'no DHCP lease available on %s; udhcpc will retry acquisition' in leasefail
assert "return 0;" in leasefail
assert "lan_down_auto(" not in leasefail
nak_callback = c_function_body(lan, "udhcpc_lan_noack(char *lan_ifname)")
assert 'Received NAK for %s' in nak_callback
assert "lan_down_auto(" not in nak_callback
assert "udhcpc_lan_deconfig(" not in nak_callback

# Every later AP RENEW is a recovery opportunity, including after both the
# volatile retry marker and the immediate PID 1 queue have failed.
bound = c_function_body(lan, "udhcpc_lan_bound(char *lan_ifname, int is_renew)")
publish = c_function_body(lan, "lan_dhcp_publish_hosts_and_schedule_retry(")
assert "lan_hosts_retry" not in lan

assert "refresh_hosts = lan_dhcp_hosts_refresh_required( is_changed, is_renew, get_ap_mode())" in compact(bound)
assert 'nvram_match("lan_hosts_retry"' not in bound
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
assert "for (i = 0; i < count; ++i)" in policy
assert "for (j = count; j > 0; --j)" in policy
assert "values[j - 1].previous_value" in policy
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
networkmap_gate = bound.index(
    "if (lan_dhcp_networkmap_refresh_required(address_changed, is_renew))",
    iappd_log,
)
networkmap_start = bound.index("int networkmap_status = restart_networkmap();", networkmap_gate)
networkmap_failure = bound.index("if (networkmap_status != 0)", networkmap_start)
networkmap_log = bound.index("networkmap refresh after lease update failed", networkmap_failure)
callback_return = bound.index("return lan_dhcp_bound_callback_status(0, hosts_status);", networkmap_log)
assert iappd_start < iappd_failure < iappd_log < networkmap_gate < networkmap_start < networkmap_failure < networkmap_log < callback_return
assert bound.count('system("/usr/bin/iappd.sh restart")') == 1
assert bound.count("restart_networkmap()") == 1
assert "lan_dhcp_networkmap_refresh_required(address_changed, is_renew)" in bound
assert "lan_dhcp_networkmap_refresh_required(int address_changed, int is_renew)" in callback_policy
assert "test_networkmap_refreshes_changed_address_renewals" in callback_test
assert "lan_dhcp_networkmap_refresh_required(1, 1) == 1" in callback_test
assert "lan_dhcp_networkmap_refresh_required(0, 1) == 0" in callback_test
assert "test_successful_lease_commit_ignores_ancillary_refresh_failure" in callback_test
assert publish.count("notify_rc_status(RCN_RESTART_DHCPD)") == 2
assert "notification_status = notify_rc_status(RCN_RESTART_DHCPD);" in publish
assert "next AP DHCP renewal will retry hostname-file publication" in publish
assert "lan_dhcp_hosts_refresh_required(int lease_changed, int is_renew," in callback_policy
assert "test_ap_same_value_renew_always_refreshes_hosts" in callback_test
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


# IPTV route teardown snapshots the gateway and all route inventories as
# one checked tuple; lease metadata publication is a separate five-key commit.
manv_route = c_function_body(wan, "manv_route_deconfig(char *man_ifname)")
assert "rc_wan_nvram_copy_snapshot(" in manv_route
assert 'fields[0].param_name = "manv_gateway";' in manv_route
assert 'fields[1].param_name = "manv_routes";' in manv_route
assert 'fields[2].param_name = "manv_routes_rfc";' in manv_route
assert 'fields[3].param_name = "manv_routes_ms";' in manv_route
assert "route_storage[3][WAN_MAN_DHCP_ROUTE_SNAPSHOT_SIZE]" in manv_route
snapshot_call = manv_route.index("route_status = rc_wan_nvram_copy_snapshot(")
snapshot_failure = manv_route.index("if (route_status != 0)", snapshot_call)
route_delete = manv_route.index("route_result = control_dhcp_routes(", snapshot_failure)
assert snapshot_call < snapshot_failure < route_delete
assert "return route_status;" in manv_route[snapshot_failure:route_delete]
assert "route_result = route_del(" in manv_route
assert "return route_status;" in manv_route
assert "preserving lease metadata for retry" in manv_route
assert "wan_dhcp_nvram_publish_values(" not in manv_route
assert "nvram_set_temp(" not in manv_route

publish_helper = c_function_body(wan, "wan_dhcp_nvram_publish_values(")
assert "nvram_transaction_begin(&transaction)" in publish_helper
assert "nvram_transaction_get(" in publish_helper
assert "wan_dhcp_nvram_transaction_set, &transaction" in publish_helper
assert "nvram_transaction_set(&transaction" not in publish_helper
assert "dhcp_lease_nvram_publish(" in publish_helper
assert "nvram_transaction_end(&transaction)" in publish_helper
assert "wan_dhcp_nvram_restore_values(" in publish_helper
assert "dhcp_lease_text_fits_buffer(" in publish_helper
assert "wan_dhcp_bound_snapshot_add(" in publish_helper
assert "nvram_safe_get((char *)names[i])" not in publish_helper
snapshot_begin = publish_helper.index("nvram_transaction_begin(&transaction)")
snapshot_get = publish_helper.index("nvram_transaction_get(")
snapshot_publish = publish_helper.index("dhcp_lease_nvram_publish(")
snapshot_end = publish_helper.index("nvram_transaction_end(&transaction)")
assert snapshot_begin < snapshot_get < snapshot_publish < snapshot_end
publish_empty = c_function_body(
    wan, "manv_viptv_deconfig_publish_empty_metadata(void *opaque)"
)
assert "return wan_dhcp_nvram_publish_values(names, values, 5);" in publish_empty
for key in ("manv_ipaddr", "manv_gateway", "manv_routes", "manv_routes_ms", "manv_routes_rfc"):
    assert '"' + key + '"' in publish_empty
viptv_policy = policy[policy.index("typedef int (*dhcp_lease_viptv_deconfig_operation)"):
                      policy.index("#endif /* PADAVAN_DHCP_LEASE_POLICY_H */")]
assert "typedef int (*dhcp_lease_viptv_deconfig_publish_operation)" in viptv_policy
assert "return ops->publish_empty_address(context);" in viptv_policy
zcip_deconfig = c_function_body(wan, "zcip_viptv_deconfig(char *man_ifname)")
assert "manv_route_deconfig(man_ifname)" in zcip_deconfig
assert "manv_viptv_deconfig_publish_empty_metadata(NULL)" in zcip_deconfig
assert "interface_status == 0 && route_status == 0" in zcip_deconfig
assert "metadata_status" in zcip_deconfig
assert "nvram_set_temp(" not in zcip_deconfig
for marker in (
    "test_dhcp_viptv_deconfig_metadata_clear_rolls_back_as_one_tuple",
    'assert(strcmp(fake.events, "ABCDE") == 0);',
    'assert(strcmp(fake.events, "ABCDEedcba") == 0);',
    "viptv_fake.publish_status = ENOSPC",
    "assert(transaction_status == ENOSPC);",
    'assert(strcmp(viptv_fake.events, "CRPCRP") == 0);',
):
    assert marker in test, marker


service_policy = (root / "trunk/user/rc/src/service_start_status.h").read_text()
service_test = (root / "trunk/user/rc/src/tests/service_start_status.c").read_text()
lan_restart = c_function_body(lan, "full_restart_lan(void)")
lan_vpn_stop = lan_restart.index("vpn_status = stop_vpn_server()")
abort_start = lan_restart.index("if (lan_status != 0)")
abort_end = lan_restart.index("reset_lan_vars();", abort_start)
abort_path = lan_restart[abort_start:abort_end]
stop_lan = lan_restart.index("lan_status = stop_lan(is_ap_mode);")
lan_vpn_stop = lan_restart.index("vpn_status = stop_vpn_server()")
dns_restore = lan_restart.index("start_dns_dhcpd_locked(is_ap_mode)", abort_start)
lock_release = lan_restart.index("dnsmasq_lifecycle_lock_release(dnsmasq_lock)", abort_start)
vpn_restore = lan_restart.index("restore VPN after aborted LAN restart", abort_start)
abort_return = lan_restart.index("return restart_status;", abort_start)
for status_name, start_call in (
    ("logger_stop_status", "start_logger(0)"),
    ("lltd_stop_status", "start_lltd()"),
    ("infosvr_stop_status", "start_infosvr()"),
    ("networkmap_stop_status", "start_networkmap(1)"),
):
    stop_call = lan_restart.index(status_name + " = stop_")
    restore_gate = lan_restart.index("rc_service_restart_after_stop(" + status_name + ")")
    restore_call = lan_restart.index(start_call, restore_gate)
    vpn_restore = lan_restart.index("restore VPN after aborted LAN restart")
    assert lan_vpn_stop < stop_call < stop_lan < abort_start < vpn_restore < restore_gate < restore_call
    assert restore_call < abort_return
assert dns_restore < lock_release < vpn_restore
assert "rc_service_restart_after_stop(int stop_status)" in service_policy
assert "return stop_status == 0;" in service_policy
assert "test_lan_restart_rollback_only_restarts_confirmed_stops" in service_test

router_restart = c_function_body(lan, "full_restart_lan(void)")
router_abort = router_restart[router_restart.index("if (lan_status != 0)"):]
router_restore = router_abort.index("start_lan(0, 1)")
router_dns_start = router_abort.index("start_dns_dhcpd_locked(is_ap_mode)")
assert "if (!is_ap_mode)" in router_abort[:router_restore]
assert router_restore < router_dns_start
assert "restore LAN interface after aborted router-mode teardown" in router_abort


ap_teardown = c_function_body(lan, "stop_lan(int is_ap_mode)")
ap_restore = c_function_body(lan, "lan_ap_teardown_abort_restore(")
ap_teardown_compact = "".join(ap_teardown.split())
assert "rc_lan_ap_teardown_recovery_required(ap_dhcp_stop_status,status)" in ap_teardown_compact
assert "status = lan_ap_teardown_abort_restore(status," in ap_teardown
assert "start_lan(1, 1)" in ap_restore
assert "notify_run_detect_internet(2)" in ap_restore
assert "rc_lan_ap_teardown_recovery_required(ap_dhcp_stop_status," in ap_teardown_compact
assert "test_ap_lan_teardown_rollback_requires_confirmed_dhcp_stop" in service_test
assert "dhcp_stop_status == 0 && cleanup_status != 0" in service_policy

runtime_snapshot = c_function_body(lan, "lan_dhcp_runtime_snapshot_copy(")
runtime_update = c_function_body(lan, "update_lan_status(int is_auto)")
assert "rc_wan_nvram_copy_snapshot(" in runtime_snapshot
assert 'fields[8].param_name = "lan_gateway_t";' in runtime_snapshot
assert "rc_wan_nvram_copy_snapshot(" in runtime_snapshot
assert "lan_dhcp_nvram_state_publish(" in runtime_update
assert "LAN_DHCP_STATUS_METADATA_COUNT" in runtime_update
assert "nvram_set_temp(" not in runtime_update
assert "int update_lan_status(int is_auto);" in rc_header
assert "test_runtime_metadata_refresh_prepares_atomic_four_key_tuple" in callback_test
lan_start = c_function_body(lan, "start_lan(int is_ap_mode, int do_wait)")
metadata_update = lan_start.index("metadata_status = update_lan_status(0)")
dhcp_launch = lan_start.index("start_udhcpc_lan(lan_ifname)")
assert metadata_update < dhcp_launch
assert "metadata_status == 0" in lan_start
assert lan_start.count("metadata_status = update_lan_status(0)") == 2
manual_start = c_function_body(lan, "lan_up_manual(char *lan_ifname, char *lan_dname)")
assert "metadata_status = update_lan_status(0)" in manual_start
assert "rc_first_service_start_status(status, metadata_status)" in manual_start

bridge_init = c_function_body(lan, "init_bridge(int is_ap_mode, int *switch_vlan_status)")
bridge_init_compact = compact(bridge_init)
wireless_marker = 'operation_status = nvram_set_int_temp("reload_svc_wl", 0);'
router_marker = 'operation_status = nvram_set_int_temp("reload_svc_rt", 0);'
wireless_record = 'rc_bridge_init_record(&bridge_status, "clear wireless service reload marker", operation_status);'
router_record = 'rc_bridge_init_record(&bridge_status, "clear router service reload marker", operation_status);'
assert wireless_marker in bridge_init_compact and router_marker in bridge_init_compact
assert wireless_record in bridge_init_compact and router_record in bridge_init_compact
assert bridge_init_compact.index(wireless_marker) < bridge_init_compact.index(wireless_record)
assert bridge_init_compact.index(router_marker) < bridge_init_compact.index(router_record)
assert "return rc_bridge_init_result(&bridge_status);" in bridge_init
assert "test_reload_marker_write_failures_are_reported_and_aggregated" in bridge_test
assert "test_reload_marker_write_failures_are_reported_and_aggregated" in bridge_policy + bridge_test

print("LAN DHCP and IPTV deconfig transactional retry contract: PASS")
