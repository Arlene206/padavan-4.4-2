#!/usr/bin/env python3
"""Verify WAN/RC startup and firewall refresh failures reach action results."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
USB_SOURCE = (RC / "usb_modem.c").read_text()
HOTPLUG_SOURCE = (RC / "hotplug_usb.c").read_text()
MODEM_NODE_POLICY_SOURCE = (RC / "modem_node_identity_policy.h").read_text()
MODEM_NODE_TEST_SOURCE = (RC / "tests/modem_node_file.c").read_text()
WAN_SOURCE = (RC / "net_wan.c").read_text()
PPP_SOURCE = (RC / "net_ppp.c").read_text()
NET_SOURCE = (RC / "net.c").read_text()
NET_LAN_SOURCE = (RC / "net_lan.c").read_text()
NET_LAN6_SOURCE = (RC / "net_lan6.c").read_text()
NET_WAN6_SOURCE = (RC / "net_wan6.c").read_text()
LAN_ADDR6_RELOAD_POLICY_SOURCE = (RC / "lan_addr6_reload_policy.h").read_text()
LAN_ADDR6_RELOAD_POLICY_TEST = (RC / "tests/lan_addr6_reload_policy.c").read_text()
FIREWALL_SOURCE = (RC / "firewall_ex.c").read_text()
FIREWALL_HOOK_SOURCE = (RC / "firewall_hook_file.h").read_text()
FIREWALL_COMMAND_STATUS_SOURCE = (RC / "firewall_command_status.h").read_text()
FIREWALL_HOOK_TEST = (RC / "tests/firewall_hook_file.c").read_text()
FIREWALL_COMMAND_STATUS_TEST = (RC / "tests/firewall_command_status.c").read_text()
SERVICES_SOURCE = (RC / "services.c").read_text()
HTTPD_POLICY_SOURCE = (RC / "httpd_daemon_policy.h").read_text()
HTTPD_POLICY_TEST_SOURCE = (RC / "tests/httpd_daemon_policy.c").read_text()
STORAGE_SOURCE = (RC / "services_stor.c").read_text()
NET6_SOURCE = (RC / "net6.c").read_text()
IPV6_GLOBAL_CONF_POLICY_SOURCE = (RC / "ipv6_global_conf_policy.h").read_text()
IPV6_GLOBAL_CONF_POLICY_TEST = (RC / "tests/ipv6_global_conf_policy.c").read_text()
VPN_SERVER_SOURCE = (RC / "vpn_server.c").read_text()
VPN_CLIENT_SOURCE = (RC / "vpn_client.c").read_text()
SERVICES_EX_SOURCE = (RC / "services_ex.c").read_text()
UPNP_POLICY_SOURCE = (RC / "upnp_restart_policy.h").read_text()
UPNP_TEST_SOURCE = (RC / "tests/upnp_restart_policy.c").read_text()
SERVICES_USB_SOURCE = (RC / "services_usb.c").read_text()
RC_SOURCE = (RC / "rc.c").read_text()
HEADER = (RC / "rc.h").read_text()
MODULE_POLICY_SOURCE = (RC / "wireless_module_load_policy.h").read_text()
MODULE_POLICY_TEST = (RC / "tests/wireless_module_load_policy.c").read_text()
COMMON_EX_SOURCE = (RC / "common_ex.c").read_text()
HWNAT_POLICY_SOURCE = (RC / "hwnat_config_policy.h").read_text()
HWNAT_POLICY_TEST = (RC / "tests/hwnat_config_policy.c").read_text()
SFE_POLICY_SOURCE = (RC / "sfe_config_policy.h").read_text()
SFE_POLICY_TEST = (RC / "tests/sfe_config_policy.c").read_text()
ROUTE_FLUSH_POLICY_SOURCE = (RC / "wan_route_flush_policy.h").read_text()
ROUTE_FLUSH_POLICY_TEST = (RC / "tests/wan_route_flush_policy.c").read_text()
POLICY_SOURCE = (RC / "modem_wan_start_policy.h").read_text()
WAN_TEST = (RC / "tests/modem_wan_start_policy.c").read_text()
PPP_VALUE_POLICY_SOURCE = (RC / "ppp_option_value_policy.h").read_text()
PPP_VALUE_POLICY_TEST = (RC / "tests/ppp_option_value_policy.c").read_text()
WAN_NVRAM_POLICY_SOURCE = (RC / "wan_nvram_status_policy.h").read_text()
WAN_NVRAM_POLICY_TEST = (RC / "tests/wan_nvram_status_policy.c").read_text()


def code_characters(source: str, start: int):
    """Yield C code characters, excluding comments and string/char literals."""
    position = start
    while position < len(source):
        if source.startswith("//", position):
            newline = source.find(chr(10), position + 2)
            position = len(source) if newline < 0 else newline + 1
            continue
        if source.startswith("/*", position):
            close = source.find("*/", position + 2)
            position = len(source) if close < 0 else close + 2
            continue
        character = source[position]
        if character in ('"', "'"):
            quote = character
            position += 1
            while position < len(source):
                if ord(source[position]) == 92:
                    position += 2
                elif source[position] == quote:
                    position += 1
                    break
                else:
                    position += 1
            continue
        yield position, character
        position += 1


def body(source: str, signature: str) -> str:
    # Ignore earlier call sites/prototypes and braces inside C comments/literals.
    start = source.find(signature)
    opening = -1
    while start >= 0:
        for position, character in code_characters(source, start + len(signature)):
            if character == ";":
                break
            if character == "{":
                opening = position
                break
        if opening >= 0:
            break
        start = source.find(signature, start + 1)
    if opening < 0:
        raise AssertionError(f"missing function body: {signature}")

    depth = 0
    for position, character in code_characters(source, opening):
        if character == "{":
            depth += 1
        elif character == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:position]
    raise AssertionError(f"unterminated function: {signature}")


# Keep the scanner honest about braces inside strings, character literals,
# line comments and block comments, with a declaration before the definition.
_parser_probe = r'''
int parser_probe(void);
int parser_probe(void) {
    const char *brace = "}";
    char open = '{';
    /* ignored: } { */
    // ignored: } {
    if (open == '{') { return brace[0] == '}' ? 1 : 0; }
    return 0;
}
'''
if "if (open == '{')" not in body(_parser_probe, "parser_probe(void)"):
    raise AssertionError("C-aware body parser regression")


def compact(source: str) -> str:
    return re.sub(r"\s+", " ", source).strip()


ras_credential_writer = compact(body(USB_SOURCE, "write_pppd_ras_credential("))
ras_conf_writer = compact(body(USB_SOURCE, "write_pppd_ras_conf("))
ras_launch = compact(body(USB_SOURCE, "launch_wan_modem_ras(int unit)"))
assert '#include "ppp_option_value_policy.h"' in USB_SOURCE
assert "ppp_option_value_escape(value, encoded, encoded_size)" in ras_credential_writer
assert "fprintf(fp, \"%s '%s'\\n\", option, encoded)" in ras_credential_writer
assert 'write_pppd_ras_credential(fp, "user", user, tmp, sizeof(tmp))' in ras_conf_writer
assert 'write_pppd_ras_credential(fp, "password", pass, tmp, sizeof(tmp))' in ras_conf_writer
assert "safe_pppd_line(user" not in ras_conf_writer
assert "safe_pppd_line(pass" not in ras_conf_writer
assert "goto invalid_credential;" in ras_conf_writer
assert '#include "wan_nvram_status_policy.h"' in USB_SOURCE
assert "rc_wan_nvram_read_int_status" in WAN_NVRAM_POLICY_SOURCE
assert "test_checked_integer_read_propagates_backend_errors" in WAN_NVRAM_POLICY_TEST
assert "test_transactional_snapshot_copies_modem_ras_credentials_before_cache_overwrite" in WAN_NVRAM_POLICY_TEST
ras_conf_compact = "".join(ras_conf_writer.split())
assert '#include <nvram_linux.h>' in USB_SOURCE
for key, destination in (
    ("modem_type", "modem_type_value"),
    ("modem_user", "user"),
    ("modem_pass", "pass"),
    ("modem_isp", "isp"),
):
    assert f'fields[' in ras_conf_writer and f'"{key}"' in ras_conf_writer
    assert f'fields[' in ras_conf_writer and f'destination = {destination};' in ras_conf_writer
assert "rc_wan_nvram_copy_snapshot(" in ras_conf_writer
assert "usb_modem_nvram_transaction_begin" in ras_conf_writer
assert "usb_modem_nvram_transaction_get" in ras_conf_writer
assert "usb_modem_nvram_transaction_end" in ras_conf_writer
assert "rc_wan_nvram_parse_int_value(modem_type_value,&modem_type)" in ras_conf_compact
assert ras_conf_compact.index("rc_wan_nvram_copy_snapshot(") < ras_conf_compact.index(
    "rc_wan_nvram_parse_int_value(modem_type_value,&modem_type)"
) < ras_conf_compact.index("ppp_options_file_open(call_path)")
assert ras_conf_compact.index("if(snapshot_status!=0)") < ras_conf_compact.index("ppp_options_file_open(call_path)")
assert ras_conf_compact.index("if(modem_type_status!=0)") < ras_conf_compact.index("ppp_options_file_open(call_path)")
assert ras_conf_compact.index("rc_wan_nvram_copy_snapshot(") < ras_conf_compact.index(
    "rc_wan_nvram_parse_int_value(modem_type_value,&modem_type)"
) < ras_conf_compact.index("ppp_options_file_open(call_path)")
assert ras_conf_compact.index("if(snapshot_status!=0)") < ras_conf_compact.index("ppp_options_file_open(call_path)")
assert ras_conf_compact.index("if(modem_type_status!=0)") < ras_conf_compact.index("ppp_options_file_open(call_path)")
assert "if (snapshot_status != 0)" in ras_conf_writer
assert "if (modem_type_status != 0)" in ras_conf_writer
assert ras_conf_compact.index("rc_wan_nvram_copy_snapshot(") < ras_conf_compact.index(
    "ppp_options_file_open(call_path)"
)
for key in ("modem_user", "modem_pass", "modem_isp"):
    assert f'nvram_safe_get("{key}")' not in ras_conf_compact
assert 'nvram_get_checked("modem_type"' not in ras_conf_compact
assert "test_transactional_snapshot_copies_modem_ras_credentials_before_cache_overwrite" in WAN_NVRAM_POLICY_TEST
assert "errno = 0; launch_status = eval(\"/usr/sbin/pppd\", \"call\", call_file);" in ras_launch
assert "if (!modem_wan_ras_peer_cleanup_required(launch_status)) return launch_status;" in ras_launch
assert "saved_errno = errno;" in ras_launch
assert "unlink(call_path)" in ras_launch
assert "errno = saved_errno; return launch_status;" in ras_launch
assert "return launch_status;" in ras_launch
assert "modem_wan_ras_peer_cleanup_required" in POLICY_SOURCE
assert "return launch_status != 0;" in POLICY_SOURCE
assert "test_ras_peer_file_cleanup_only_after_failed_launch" in WAN_TEST
assert 'ppp_option_value_escape("name\\tvalue"' in PPP_VALUE_POLICY_TEST
assert "ppp_option_escape_internal(value, out, out_size, 0)" in PPP_VALUE_POLICY_SOURCE

ndis = compact(body(USB_SOURCE, "launch_wan_usbnet(int unit)"))
migration = "migration_status = check_upnp_wanif_changed(ndis_ifname);"
identity_publish = "identity_status = modem_wan_publish_ndis_identity("
identity_gate = "if (identity_status != 0)"
interface_setup = 'interface_status = doSystem("ifconfig %s mtu %d up %s", ndis_ifname, ndis_mtu, "0.0.0.0");'
dhcp_launch = "dhcp_status = start_udhcpc_wan(ndis_ifname, unit, 0);"
assert migration in ndis and identity_publish in ndis
assert ndis.index(migration) < ndis.index(identity_publish) < ndis.index(identity_gate) < ndis.index(interface_setup) < ndis.index(dhcp_launch)
assert "return modem_wan_start_status(migration_status, error_status);" in ndis
assert "return modem_wan_start_status(identity_status, error_status);" in ndis
assert "modem_wan_publish_ndis_identity(" in ndis
assert "usb_modem_wan_identity_set_value, &unit, ndis_ifname," in ndis
assert "&identity_rollback_status);" in ndis
assert "USB modem WAN identity rollback incomplete" in ndis
assert "cannot publish NDIS protocol/interface identity" in ndis
firewall_capture = "firewall_status = start_firewall_ex();"
assert firewall_capture in ndis
assert "startup_status = modem_wan_start_status(migration_status, firewall_status);" in ndis
assert "return modem_wan_start_status(result_status, error_status);" in ndis
assert "USB modem WAN firewall startup incomplete" in ndis
assert 'set_wan_unit_value_int(unit, "err"' not in ndis
assert "set_wan_unit_error_status(unit, dhcp_status != 0);" in ndis

control = compact(body(USB_SOURCE, "ndis_control_network(char *ndis_ifname"))

control = compact(body(USB_SOURCE, "ndis_control_network(char *ndis_ifname"))
assert "*control_required = 0;" in control
for driver, operation in (
    ("qmi_wwan", "qmi_control_network(control_node_wdm, is_start)"),
    ("huawei_cdc_ncm", "ncm_control_network(control_node_wdm, is_start)"),
    ("cdc_ncm", "ncm_control_network(control_node_tty, is_start)"),
    ("sierra_net", "sierra_control_network(control_node_tty, is_start)"),
):
    call_pos = control.index("return " + operation + ";")
    branch_start = control.rfind(
        'if (is_usbnet_has_module(ndis_ifname, "' + driver + '"))',
        0, call_pos
    )
    assert branch_start >= 0 and "*control_required = 1;" in control[branch_start:call_pos]
assert "return mbim_control_network(control_node_wdm, is_start);" in control

assert interface_setup in ndis, "NDIS interface MTU/UP command status must be captured"
interface_failure_pos = ndis.index("if (interface_status != 0)")
firewall_start_pos = ndis.index(firewall_capture)
control_start_pos = ndis.index("control_status = ndis_control_network(ndis_ifname, modem_devnum, 1")
control_failure_pos = ndis.index("if (modem_ndis_control_failed(control_required, control_status))", control_start_pos)
dhcp_start_pos = ndis.index(dhcp_launch)
assert ndis.index(identity_gate) < interface_failure_pos < firewall_start_pos
assert interface_failure_pos < firewall_start_pos < control_start_pos < control_failure_pos < dhcp_start_pos
assert ndis.index("startup_status = modem_wan_start_status(migration_status, firewall_status);") < control_start_pos
interface_failure = ndis[interface_failure_pos:firewall_start_pos]
assert "set_wan_unit_error_status(unit, 1);" in interface_failure
assert "identity_rollback_status = modem_wan_clear_ndis_identity(" in interface_failure
assert "usb_modem_wan_identity_set_value, &unit);" in interface_failure
assert "if (identity_rollback_status != 0)" in interface_failure
assert "USB modem WAN interface identity rollback failed after ifconfig error" in interface_failure
assert "return modem_wan_start_status(result_status, error_status);" in interface_failure
assert "logmessage(LOGNAME" in interface_failure
assert "modem_wan_clear_ndis_identity" in POLICY_SOURCE
assert "test_ndis_identity_clear_attempts_protocol_after_ifname_failure" in WAN_TEST
assert "test_ndis_identity_clear_reports_second_field_failure" in WAN_TEST
assert "test_ndis_identity_clear_removes_both_fields();" in WAN_TEST

control_failure = ndis[control_failure_pos:dhcp_start_pos]
assert "set_wan_unit_error_status(unit, 1);" in control_failure
assert "USB modem control-session startup failed" in control_failure
assert "return modem_wan_start_status(result_status, error_status);" in control_failure
assert "if (control_required)" in ndis[control_failure_pos:dhcp_start_pos]
assert "modem_ndis_control_failed(control_required, control_status)" in ndis
assert "modem_wan_publish_ndis_identity" in POLICY_SOURCE
assert "test_ndis_identity_failure_clears_both_fields_and_preserves_primary_error" in WAN_TEST
assert "test_ndis_identity_failure_clears_both_fields_and_preserves_primary_error();" in WAN_TEST
assert "int set_wan_unit_value_status(int unit" in HEADER
assert "int set_wan_unit_value_int_status(int unit" in HEADER
assert "int set_wan_unit_error_status(int unit" in HEADER
assert "if (unit < 0 || !param_name || !param_name[0] || !value)" in WAN_SOURCE
assert "set_wan_unit_error_status(unit, dhcp_status != 0);" in ndis

ndis_stop = compact(body(USB_SOURCE, "stop_wan_usbnet(void)"))
assert "ndis_control_network(ndis_ifname, modem_devnum, 0, &control_required)" in ndis_stop
assert "modem_ndis_control_failed(control_required, control_status)" in ndis_stop

for marker in (
    "test_ndis_firewall_failure_reaches_startup_result",
    "test_ndis_optional_control_path_does_not_block_dhcp",
    "test_ndis_required_control_failure_blocks_dhcp",
    "test_ndis_required_control_success_allows_dhcp",
):
    assert marker in WAN_TEST
assert "int modem_ndis_control_failed(int control_required" not in POLICY_SOURCE
assert "return control_required && control_status != 0;" in POLICY_SOURCE
assert "test_first_wan_error_survives_usb_modem_and_later_wan_failures();" in WAN_TEST

vinet_setup = compact(body(WAN_SOURCE, "config_vinet_wan(void)"))
parent_ipv6_call = vinet_setup.index("ret = control_if_ipv6(ifname_wan_cpu, 0);")
parent_ipv6_guard = vinet_setup.rfind("if (get_ipv6_type() != IPV6_DISABLED)", 0, parent_ipv6_call)
parent_ipv6_end = vinet_setup.index("ret = create_vlan_iface(", parent_ipv6_call)
parent_ipv6 = vinet_setup[parent_ipv6_guard:parent_ipv6_end]
assert parent_ipv6_guard >= 0
assert re.search(r"ret\s*=\s*control_if_ipv6\(ifname_wan_cpu,\s*0\)\s*;", parent_ipv6)
assert re.search(r"if\s*\(ret\s*!=\s*0\)\s*\{[^}]*return\s+ret\s*;", parent_ipv6), (
    "WAN VLAN creation must stop when required IPv6 disable on the parent fails"
)

vinet_publish = vinet_setup.index('ret = set_wan_unit_value_status(0, "ifname", vinet_ifname);')
vinet_enable_call = vinet_setup.index("ret = control_if_ipv6(vinet_ifname, 1);")
vinet_enable_guard = vinet_setup.rfind("if (get_ipv6_type() != IPV6_DISABLED)", 0, vinet_enable_call)
vinet_enable = vinet_setup[vinet_enable_guard:vinet_publish]
assert vinet_enable_guard >= 0
assert re.search(r"if\s*\(ret\s*!=\s*0\)\s*\{[^}]*return\s+ret\s*;", vinet_enable), (
    "WAN interface publication must wait for successful IPv6 enable"
)
vinet_publish_gate = vinet_setup.index("if (ret != 0)", vinet_publish)
vinet_publish_failure = vinet_setup[vinet_publish_gate:vinet_setup.index("return 0;", vinet_publish_gate)]
assert "return ret;" in vinet_publish_failure, (
    "WAN startup must fail closed when active interface identity publication fails"
)

wisp_setup = compact(body(WAN_SOURCE, "config_apcli_wisp(void)"))
wisp_publish = wisp_setup.index('publication_status = set_wan_unit_value_status(0, "ifname", wisp_ifname);')
wisp_enable_call = wisp_setup.index("int ipv6_status = control_if_ipv6(wisp_ifname, 1);")
wisp_enable_guard = wisp_setup.rfind("if (get_ipv6_type() != IPV6_DISABLED)", 0, wisp_enable_call)
wisp_enable = wisp_setup[wisp_enable_guard:wisp_publish]
assert wisp_enable_guard >= 0
assert re.search(r"if\s*\(ipv6_status\s*!=\s*0\)\s*\{[^}]*return\s+ipv6_status\s*;", wisp_enable), (
    "AP-client WAN publication must wait for successful IPv6 enable"
)
wisp_publish_gate = wisp_setup.index("if (publication_status != 0)", wisp_publish)
wisp_publish_failure = wisp_setup[wisp_publish_gate:wisp_setup.index("return 0;", wisp_publish_gate)]
assert "return publication_status;" in wisp_publish_failure, (
    "AP-client WISP startup must fail closed when active interface identity publication fails"
)

wan_start = compact(body(WAN_SOURCE, "start_wan(void)"))
assert "modem_start_status" not in wan_start
assert "return wan_start_status;" in wan_start

callback_failure_start = wan_start.index("if (create_cb_links() != 0)")
callback_failure_end = wan_start.index("vlan_result = config_vinet_wan();", callback_failure_start)
callback_failure = wan_start[callback_failure_start:callback_failure_end]
assert "callback_status = errno ? errno : EIO;" in callback_failure
assert "wan_start_status = rc_first_service_start_status( wan_start_status, callback_status);" in callback_failure
assert "return wan_start_status;" in callback_failure
assert "return errno ? errno : EIO;" not in callback_failure

for setup_call, next_call, diagnostic in (
    ("vlan_result = config_vinet_wan();", "vlan_result = config_apcli_wisp();",
     "WAN VLAN configuration failed"),
    ("vlan_result = config_apcli_wisp();", "vlan_result = launch_viptv_wan();",
     "AP-client WISP migration failed"),
):
    setup_start = wan_start.index(setup_call)
    setup_end = wan_start.index(next_call, setup_start + len(setup_call))
    setup_failure = wan_start[setup_start:setup_end]
    aggregate = "wan_start_status = rc_first_service_start_status( wan_start_status, vlan_result);"
    assert aggregate in setup_failure, diagnostic + " must be merged before abort"
    assert setup_failure.index(aggregate) < setup_failure.index("cleanup_status = remove_cb_links();")
    assert "return wan_start_status;" in setup_failure
    assert "return vlan_result;" not in setup_failure

assert "test_early_wan_abort_preserves_first_error();" in WAN_TEST
assert "wan_up_status = wan_up(wan_ifname, unit, 1);" in wan_start
assert re.search(r"wan_start_status\s*=\s*rc_first_service_start_status\s*\(\s*wan_start_status\s*,\s*wan_up_status\s*\)\s*;", wan_start)
interface_failure_start = wan_start.index("if (interface_status != 0)")
interface_failure_end = wan_start.index("/* Perform AP-client reconnect and retain driver command failures. */", interface_failure_start)
interface_failure = wan_start[interface_failure_start:interface_failure_end]
interface_error_record = re.search(
    r"wan_start_status\s*=\s*rc_first_service_start_status\s*\(\s*wan_start_status\s*,\s*interface_status\s*\)\s*;",
    interface_failure,
)
assert interface_error_record, "physical WAN IFUP failure must reach start_wan() status"
assert interface_failure.index(interface_error_record.group(0)) < interface_failure.index(
    "interface_cleanup_status = ifconfig("
), "best-effort interface cleanup must not replace the original IFUP failure"
assert "continue;" in interface_failure
assert "return wan_start_status;" in wan_start
assert re.search(r"wan_start_status\s*=\s*rc_first_service_start_status\s*\(\s*wan_start_status\s*,\s*ipv6_status\s*\)\s*;", wan_start)
assert "vlan_result = launch_wan_usbnet(unit);" in wan_start
assert "vlan_result = launch_wan_modem_ras(unit);" in wan_start

iptv_call = "vlan_result = launch_viptv_wan();"
assert iptv_call in wan_start
iptv_call_pos = wan_start.index(iptv_call)
iptv_error_pos = wan_start.index("if (vlan_result != 0)", iptv_call_pos)
iptv_log_pos = wan_start.index("IPTV VLAN or service setup incomplete", iptv_error_pos)
assert re.search(
    r"wan_start_status\s*=\s*rc_first_service_start_status\s*\(\s*wan_start_status\s*,\s*vlan_result\s*\)\s*;",
    wan_start[iptv_error_pos:iptv_log_pos],
), "configured IPTV startup failure must be returned without aborting WAN core startup"

passthrough_call = "int passthrough_status = set_passthrough_pppoe(1);"
assert passthrough_call in wan_start
passthrough_pos = wan_start.index(passthrough_call)
passthrough_error_pos = wan_start.index("if (passthrough_status != 0)", passthrough_pos)
passthrough_status_pos = wan_start.index("wan_start_status = rc_first_service_start_status(", passthrough_error_pos)
passthrough_status_end = wan_start.index(";", passthrough_status_pos)
assert "wan_start_status, passthrough_status" in wan_start[passthrough_status_pos:passthrough_status_end]
assert passthrough_status_pos < wan_start.rindex("return wan_start_status;")

wait_ppp = compact(body(WAN_SOURCE, "wait_ppp_up(char *ppp_ifname, int unit)"))
assert "failure_status = (interface_status < 0 && errno != 0) ?" in wait_ppp
assert "return failure_status;" in wait_ppp
assert "socket_status = errno ? errno : EIO;" in wait_ppp
assert "return socket_status;" in wait_ppp
assert "return route_status ? route_status : EIO;" in wait_ppp
ping_command = 'eval("ping", "-c1", ppp_gate);'
assert ping_command in wait_ppp, "PPP peer probe must use argv, not a formatted shell command"
assert 'doSystem("ping -c1 %s", ppp_gate)' not in wait_ppp
assert "return 0;" in wait_ppp[wait_ppp.index(ping_command):]
ppp_branch_start = wan_start.index("char *proto_desc, *ppp_ifname = IFNAME_PPP;")
ipoe_branch_start = wan_start.index("else if (wan_proto == IPV4_WAN_PROTO_IPOE_DHCP", ppp_branch_start)
ppp_branch = wan_start[ppp_branch_start:ipoe_branch_start]
for operation in (
    "vlan_result = check_upnp_wanif_changed(ppp_ifname);",
    "vlan_result = launch_wan_pppd(unit, wan_proto);",
    "vlan_result = wait_ppp_up(ppp_ifname, unit);",
):
    operation_pos = ppp_branch.index(operation)
    next_continue = ppp_branch.index("continue;", operation_pos)
    failure_path = ppp_branch[operation_pos:next_continue]
    assert "wan_start_status = rc_first_service_start_status" in failure_path, (
        operation + " failures must reach the WAN startup result"
    )
wanx = compact(body(WAN_SOURCE, "launch_wanx(char *man_ifname, int unit, int wait_dhcpc, int use_zcip)"))
static_man_validation = "if (!dhcp_lease_ipv4_is_valid(man_addr) || !dhcp_lease_netmask_is_valid(man_mask))"
assert static_man_validation in wanx, "static MAN must reject malformed IPv4 address/netmask"
man_invalid_pos = wanx.index(static_man_validation)
man_subnet_pos = wanx.index("else if (is_same_subnet2(man_addr, lan_addr, man_mask, lan_mask))")
man_mtu_pos = wanx.index("mtu_status = set_interface_mtu(man_ifname, man_mtu);")
man_address_pos = wanx.index("address_status = ifconfig(man_ifname, IFUP, man_addr, man_mask);")
assert man_invalid_pos < man_subnet_pos < man_mtu_pos < man_address_pos
man_invalid_path = wanx[man_invalid_pos:man_subnet_pos]
assert "man_err = 1;" in man_invalid_path
assert "launch_status = EINVAL;" in man_invalid_path
assert "invalid static WAN IPv4 address or netmask" in man_invalid_path
assert 'logmessage(LOGNAME, "MAN interface %s MTU setup incomplete (status=%d)"' in wanx
assert "if (mtu_status != 0)" in wanx
wanx_error_pos = ppp_branch.index("if (wanx_status != 0)")
wanx_abort_pos = ppp_branch.index("if (!i_pppoe && wanx_status != 0)")
wanx_capture = re.search(
    r"wan_start_status\s*=\s*rc_first_service_start_status\s*\(\s*wan_start_status\s*,\s*wanx_status\s*\)\s*;",
    ppp_branch[wanx_error_pos:wanx_abort_pos],
)
assert wanx_capture, "configured PPPoE MAN failures must reach the WAN startup result"
assert "WAN physical interface setup incomplete" in ppp_branch[wanx_error_pos:wanx_abort_pos]
assert "test_manual_pppoe_physical_setup_failure_is_reported_without_blocking_pppd" in WAN_TEST
assert ppp_branch.index("vlan_result = launch_wan_pppd(unit, wan_proto);", wanx_abort_pos) > wanx_abort_pos

ipoe_end = wan_start.index("return wan_start_status;", ipoe_branch_start)
ipoe_branch = wan_start[ipoe_branch_start:ipoe_end]
upnp_reload_call = "wan_upnp_status = update_upnp();"
assert upnp_reload_call in ipoe_branch
upnp_reload_pos = ipoe_branch.index(upnp_reload_call)
assert ipoe_branch.index("wan_up_status = wan_up(wan_ifname, unit, 1);") < upnp_reload_pos
upnp_failure_pos = ipoe_branch.index("if (wan_upnp_status != 0)", upnp_reload_pos)
upnp_log_pos = ipoe_branch.index("UPnP update incomplete after WAN startup", upnp_failure_pos)
upnp_status_match = re.search(
    r"wan_start_status\s*=\s*rc_first_service_start_status\s*\(\s*wan_start_status\s*,\s*wan_upnp_status\s*\)\s*;",
    ipoe_branch[upnp_failure_pos:upnp_log_pos],
)
assert upnp_status_match, "static IPoE UPnP reload failure must reach the WAN start result"
assert upnp_failure_pos < upnp_failure_pos + upnp_status_match.start() < upnp_log_pos
migration_pos = ipoe_branch.index("vlan_result = check_upnp_wanif_changed(wan_ifname);")
migration_end = ipoe_branch.index("continue;", migration_pos)
assert "wan_start_status = rc_first_service_start_status" in ipoe_branch[migration_pos:migration_end]
static_config_start = ipoe_branch.index("if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC)")
static_config_end = ipoe_branch.index('set_wan_unit_error_status(unit, wan_err);', static_config_start)
static_config = compact(ipoe_branch[static_config_start:static_config_end])
snapshot_call = static_config.index("snapshot_status = wan_static_ipv4_snapshot(")
assert "char wan_addr[16], wan_mask[16], lan_addr[16], lan_mask[16];" in static_config
assert "nvram_safe_get(" not in static_config and "get_wan_unit_value(" not in static_config
assert "if (snapshot_status != 0)" in static_config
assert static_config.index("wan_start_status, snapshot_status);") < static_config.index("if (!dhcp_lease_ipv4_is_valid(wan_addr)")
invalid_config = "if (!dhcp_lease_ipv4_is_valid(wan_addr) || !dhcp_lease_netmask_is_valid(wan_mask))"
assert invalid_config in static_config and snapshot_call < static_config.index(invalid_config), (
    "static WAN IPoE must take a checked coherent NVRAM snapshot before validating or configuring addresses"
)
snapshot_start = WAN_SOURCE.index("wan_static_ipv4_snapshot(")
snapshot_signature = WAN_SOURCE[snapshot_start:WAN_SOURCE.index("{", snapshot_start)]
assert "const char *wan_addr_suffix" in snapshot_signature
assert "const char *wan_mask_suffix" in snapshot_signature
snapshot_helper = "".join(body(WAN_SOURCE, "wan_static_ipv4_snapshot(").split())
assert '"wan%d_%s",unit,wan_addr_suffix' in snapshot_helper
assert '"wan%d_%s",unit,wan_mask_suffix' in snapshot_helper
launch_wanx = "".join(body(WAN_SOURCE, "launch_wanx(").split())
man_snapshot_call = 'snapshot_status=wan_static_ipv4_snapshot(unit,"man_ipaddr","man_netmask",'
assert man_snapshot_call in launch_wanx
assert "charman_addr_storage[16],man_mask_storage[16],lan_addr[16],lan_mask[16];" in launch_wanx
assert "if(snapshot_status!=0)" in launch_wanx
assert launch_wanx.index(man_snapshot_call) < launch_wanx.index("if(snapshot_status!=0)") < launch_wanx.index("if(!is_valid_ipv4(man_addr))")
assert "get_wan_unit_value(unit,\"man_ipaddr\")" not in launch_wanx
assert "get_wan_unit_value(unit,\"man_netmask\")" not in launch_wanx
assert "nvram_safe_get(\"lan_ipaddr\")" not in launch_wanx
assert "nvram_safe_get(\"lan_netmask\")" not in launch_wanx
invalid_config_start = static_config.index(invalid_config)
subnet_conflict_start = static_config.index("else if (is_same_subnet2(wan_addr, lan_addr, wan_mask, lan_mask))")
assert invalid_config_start < subnet_conflict_start
invalid_config_path = static_config[invalid_config_start:subnet_conflict_start]
assert "wan_err = 1;" in invalid_config_path
assert re.search(
    r"wan_start_status\s*=\s*rc_first_service_start_status\s*\(\s*wan_start_status\s*,\s*EINVAL\s*\)\s*;",
    invalid_config_path,
), "invalid static WAN config must not be reported as successful startup"
assert "mtu_status = set_interface_mtu(wan_ifname, wan_mtu);" in static_config
assert 'logmessage(LOGNAME, "static WAN interface %s MTU setup failed (status=%d); skipping address and route setup"' in static_config
assert static_config.index("mtu_status = set_interface_mtu(wan_ifname, wan_mtu);") > subnet_conflict_start
assert static_config.index("mtu_status = set_interface_mtu(wan_ifname, wan_mtu);") < static_config.index("address_status = ifconfig(wan_ifname, IFUP, wan_addr, wan_mask);")

wan_dhcp_bound = compact(body(WAN_SOURCE, "udhcpc_bound(char *wan_ifname, int is_renew_mode)"))
assert "mtu_status = set_interface_mtu(wan_ifname, dhcp_mtu);" in wan_dhcp_bound
assert 'logmessage(log_prefix, "%s (%s) MTU setup incomplete (status=%d)"' in wan_dhcp_bound
wan_mtu_failure = wan_dhcp_bound[wan_dhcp_bound.index("if (mtu_status != 0)"):wan_dhcp_bound.index("wan_gw = nvram_safe_get", wan_dhcp_bound.index("if (mtu_status != 0)"))]
assert "i_err =" not in wan_mtu_failure and "route_setup_status =" not in wan_mtu_failure and "return " not in wan_mtu_failure

iptv_dhcp_bound = compact(body(WAN_SOURCE, "udhcpc_viptv_bound(char *man_ifname, int is_renew_mode)"))
assert "int interface_status, rollback_status, route_setup_status, mtu_status;" in iptv_dhcp_bound
assert "mtu_status = set_interface_mtu(man_ifname, dhcp_mtu);" in iptv_dhcp_bound
assert 'logmessage(log_prefix, "%s (%s) MTU setup incomplete (status=%d)"' in iptv_dhcp_bound
iptv_mtu_failure = iptv_dhcp_bound[iptv_dhcp_bound.index("if (mtu_status != 0)"):iptv_dhcp_bound.index("logmessage(log_prefix,", iptv_dhcp_bound.index("if (mtu_status != 0)"))]
assert "i_err =" not in iptv_mtu_failure and "route_setup_status =" not in iptv_mtu_failure and "return " not in iptv_mtu_failure

address_pos = ipoe_branch.index("if (address_status != 0)")
address_end = ipoe_branch.index("/* Rebuild required firewall policy before EAPOL, DHCP or route activation. */", address_pos)
assert "wan_start_status = rc_first_service_start_status" in ipoe_branch[address_pos:address_end]
auth_pos = ipoe_branch.index("if (auth_status != 0)")
auth_end = ipoe_branch.index("/* We are done configuration */", auth_pos)
assert "wan_start_status = rc_first_service_start_status" in ipoe_branch[auth_pos:auth_end]
dhcp_pos = ipoe_branch.index("if (dhcp_status != 0)")
dhcp_end = ipoe_branch.index('set_wan_unit_error_status(unit, wan_err);', dhcp_pos)
assert "wan_start_status = rc_first_service_start_status" in ipoe_branch[dhcp_pos:dhcp_end]

aggregate_pattern = r"wan_start_status\s*=\s*rc_first_service_start_status\s*\(\s*wan_start_status\s*,\s*vlan_result\s*\)\s*;"
aggregate_matches = list(re.finditer(aggregate_pattern, wan_start))
# Checked NVRAM failures now also feed first-error aggregation for modem
# transport, both PPPoE manual-mode branches, idle-time and WAN interface setup.
assert len(aggregate_matches) == 39

# Each checked protocol/interface pair publication failure retains both
# results: the failed NVRAM write and the physical-WAN recovery attempt.
normalized_wan_start = " ".join(wan_start.split())
for identity_marker, diagnostic in (
    ('vlan_result = rc_wan_nvram_publish_runtime_identity( set_wan_unit_value_status_callback, &unit, "RAS Modem", ppp_ifname);',
     "cannot publish active WAN interface"),
    ('vlan_result = rc_wan_nvram_publish_runtime_identity( set_wan_unit_value_status_callback, &unit, proto_desc, ppp_ifname);',
     "cannot publish active WAN interface"),
    ('vlan_result = rc_wan_nvram_publish_runtime_identity( set_wan_unit_value_status_callback, &unit, "IPoE", wan_ifname);',
     "cannot publish active WAN interface"),
):
    identity_at = normalized_wan_start.index(identity_marker)
    failure_at = normalized_wan_start.index(
        "if (vlan_result != 0)", identity_at + len(identity_marker)
    )
    restore_at = normalized_wan_start.index(
        "vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);",
        failure_at,
    )
    continue_at = normalized_wan_start.index("continue;", restore_at)
    failure_path = normalized_wan_start[failure_at:continue_at]
    assert len(re.findall(aggregate_pattern, failure_path)) == 2
    assert diagnostic in failure_path
    assert identity_at < failure_at < restore_at < continue_at


# The RAS firewall rebuild, failed-rebuild recovery, and PPP launch recovery
# each contribute status.
ras_launch_start = wan_start.index("char *ppp_ifname = IFNAME_RAS;")
ras_launch_end = wan_start.index("vlan_result = launch_wan_modem_ras(unit);", ras_launch_start)
ras_branch = wan_start[ras_launch_start:ras_launch_end]
ras_firewall_start = ras_branch.index("vlan_result = start_firewall_ex();")
assert re.search(aggregate_pattern, ras_branch[ras_firewall_start:]), "RAS firewall failure must reach modem startup status"
ras_migration = "vlan_result = check_upnp_wanif_changed(ppp_ifname);"
assert ras_migration in wan_start
ras_migration_start = wan_start.index(ras_migration)
assert re.search(aggregate_pattern, wan_start[ras_migration_start:]), "RAS migration failure must be retained"
ras_launch_start = wan_start.index("char *ppp_ifname = IFNAME_RAS;")
ras_launch_end = wan_start.index("vlan_result = launch_wan_modem_ras(unit);", ras_launch_start)
ras_branch = wan_start[ras_launch_start:ras_launch_end]
ras_wanx_pattern = r"wan_start_status\s*=\s*rc_first_service_start_status\s*\(\s*wan_start_status\s*,\s*wanx_status\s*\)\s*;"
ras_wanx_match = re.search(ras_wanx_pattern, ras_branch)
assert ras_wanx_match, "RAS physical WAN setup failure must reach modem startup status"
assert ras_branch.index("if (wanx_status != 0)") < ras_wanx_match.start()
ras_launch_call = "vlan_result = launch_wan_modem_ras(unit);"
ras_launch_pos = wan_start.index(ras_launch_call)
ras_launch_failure_pos = wan_start.index("if (vlan_result != 0)", ras_launch_pos)
ras_restore_call = "vlan_result = restore_wan_after_firewall_failure(unit, wan_ifname);"
ras_restore_pos = wan_start.index(ras_restore_call, ras_launch_failure_pos)
assert re.search(aggregate_pattern, wan_start[ras_launch_pos:ras_launch_failure_pos]), (
    "RAS PPP launch failure must be recorded before rollback status replaces vlan_result"
)
assert ras_restore_pos > ras_launch_failure_pos
assert re.search(aggregate_pattern, wan_start[ras_restore_pos:]), (
    "RAS rollback/recovery result must also reach WAN startup status"
)
assert wan_start.index("WAN recovery after USB modem RAS launch failure incomplete", ras_restore_pos) > ras_restore_pos
eapol_start = wan_start.index("if (auth_status != 0)")
eapol_end = wan_start.index("/* We are done configuration */", eapol_start)
assert "modem_start_status" not in wan_start
assert wan_start.index("set_passthrough_pppoe(1)") < wan_start.rindex("return wan_start_status;")

l2tp_start_merge = compact(
    body(WAN_SOURCE, "merge_l2tp_server_start_status(int wan_status)")
)
assert 'nvram_match("l2tp_srv_t", "1")' in l2tp_start_merge
assert "xl2tp_status = safe_start_xl2tpd();" in l2tp_start_merge
assert "wan_status == 0 && xl2tp_status < 0" in l2tp_start_merge
assert "return xl2tp_status;" in l2tp_start_merge
assert "return wan_status;" in l2tp_start_merge

# PPP control commands are fixed argv vectors; avoid an unnecessary shell layer.
assert not re.search(r"\b(?:system|doSystem)\s*\(", PPP_SOURCE), "PPP source must not call shell execution APIs"
rpl2tp_start = compact(body(PPP_SOURCE, "start_rpl2tp(int unit)"))
assert 'status = eval("/usr/sbin/l2tp-control", "start-session 0.0.0.0");' in rpl2tp_start
assert "if (status != 0)" in rpl2tp_start
ppp_launch = compact(body(PPP_SOURCE, "launch_wan_pppd_locked(int unit, int wan_proto)"))
assert 'launch_status = eval("/usr/bin/mwan.sh", "--rc-lifecycle-locked");' in ppp_launch
assert "if (launch_status != 0)" in ppp_launch

route_flush = compact(body(NET_SOURCE, "clear_if_route4(char *ifname)"))
assert 'return eval("ip", "route", "flush", "dev", ifname, "scope", "global")' in route_flush
assert "doSystem" not in route_flush
assert "int clear_if_route4(char *ifname);" in HEADER
route_policy = compact(ROUTE_FLUSH_POLICY_SOURCE)
assert "rc_first_service_start_status(first_status, status)" in route_policy
assert "if (!ifnames[i] || !ifnames[i][0])" in route_policy
assert "test_all_interfaces_are_attempted_and_first_error_is_preserved" in ROUTE_FLUSH_POLICY_TEST
assert "test_invalid_interface_does_not_skip_other_cleanup" in ROUTE_FLUSH_POLICY_TEST
full_restart_routes = compact(body(WAN_SOURCE, "full_restart_wan(void)"))
route_apply_pos = full_restart_routes.index("route_flush_status = rc_wan_route_flush_and_cache_apply(")
route_apply_end = full_restart_routes.index("flush_route_caches);", route_apply_pos)
route_gate_pos = full_restart_routes.index("if (route_flush_status != 0)")
assert route_apply_pos < route_apply_end < route_gate_pos < full_restart_routes.index("phy_clear_mac_table()")
assert "route_ifnames, 2, clear_if_route4, flush_route_caches);" in full_restart_routes
assert "interface routes or the global route cache could not be flushed" in full_restart_routes
route_restore_pos = full_restart_routes.index(
    "status = rc_wan_restore_lan_routes_after_failed_step(", route_gate_pos)
route_restore_check = full_restart_routes.index(
    "if (static_lan_route_status != 0)", route_restore_pos)
route_restore_return = full_restart_routes.index("return status;", route_restore_check)
assert route_gate_pos < route_restore_pos < route_restore_check < route_restore_return < full_restart_routes.index("phy_clear_mac_table()")
assert "static LAN route restoration also failed after interface route flush error" in full_restart_routes[route_restore_check:route_restore_return]
assert "test_failed_route_flush_restores_lan_routes_and_preserves_first_error" in ROUTE_FLUSH_POLICY_TEST
for call, diagnostic in (
    ("stop_wan()", "services did not stop (status=%d)"),
    ("disassembly_bridges_wan()", "WAN bridge teardown failed (status=%d)"),
    ("config_soft_bridge_lan()", "LAN bridge VLAN setup failed (status=%d)"),
):
    capture = full_restart_routes.index(f"status = {call};")
    gate = full_restart_routes.index("if (status != 0)", capture)
    log = full_restart_routes.index(diagnostic, gate)
    returned = full_restart_routes.index("return status;", log)
    assert capture < gate < log < returned, call
cache_policy = compact(body(ROUTE_FLUSH_POLICY_SOURCE, "rc_wan_route_flush_and_cache_apply("))
assert "rc_wan_route_flush_apply(ifnames, count, flush)" in cache_policy
assert "flush_cache ? flush_cache() : EINVAL" in cache_policy
assert "rc_first_service_start_status(first_status, cache_status)" in cache_policy
assert "test_global_cache_flush_is_attempted_and_errors_are_preserved" in ROUTE_FLUSH_POLICY_TEST
# LAN IPv6 reload preserves clear/add errors and avoids shell interpolation.
reload_policy = compact(LAN_ADDR6_RELOAD_POLICY_SOURCE)
assert "if (!clear_address || (has_address && !add_address)) return EINVAL;" in reload_policy
assert "status = clear_address();" in reload_policy
assert "if (status != 0 || !has_address) return status;" in reload_policy
assert "return add_address(context);" in reload_policy
assert "test_clear_failure_prevents_new_address_publication" in LAN_ADDR6_RELOAD_POLICY_TEST
assert "test_add_failure_is_returned_after_successful_clear" in LAN_ADDR6_RELOAD_POLICY_TEST
reload_lan = compact(body(NET_LAN6_SOURCE, "reload_lan_addr6(void)"))
assert "return rc_lan_addr6_reload_apply(lan_addr6 && lan_addr6[0]," in reload_lan
clear_reload = compact(body(NET_LAN6_SOURCE, "clear_lan_addr6_for_reload(void)"))
assert "return clear_if_addr6(IFNAME_BR);" in clear_reload
add_reload = compact(body(NET_LAN6_SOURCE, "add_lan_addr6_for_reload(void *context)"))
assert 'return eval("ip", "-6", "addr", "add", lan_addr6, "dev", IFNAME_BR);' in add_reload
assert "doSystem(" not in add_reload
assert "int reload_lan_addr6(void);" in HEADER
assert "void reload_lan_addr6(void);" not in HEADER
lan_start = compact(body(NET_LAN_SOURCE, "start_lan(int is_ap_mode, int do_wait)"))
assert "operation_status = reload_lan_addr6();" in NET_LAN_SOURCE
assert "start_status = rc_first_service_start_status(start_status, operation_status);" in lan_start
assert "LAN IPv6 address reload failed (status=%d)" in lan_start
ipv6_restart = compact(body(NET6_SOURCE, "full_restart_ipv6(int ipv6_type_old)"))
assert "operation_status = reload_lan_addr6();" in ipv6_restart
assert "restart_status = rc_first_service_start_status(restart_status, operation_status);" in ipv6_restart
assert "IPv6 enable could not reload LAN address (status=%d)" in ipv6_restart

# Global IPv6 sysctl writes retain first failure while trying every operation.
global_conf_policy = compact(IPV6_GLOBAL_CONF_POLICY_SOURCE)
assert "if (operation_count == 0) return 0;" in global_conf_policy
assert "if (!operations || !write_operation) return EINVAL;" in global_conf_policy
assert "if (first_status == 0 && status != 0) first_status = status;" in global_conf_policy
assert "test_all_attempted_first_error" in IPV6_GLOBAL_CONF_POLICY_TEST
global_writer = compact(body(NET6_SOURCE, "write_ipv6_conf_operation(void *context, const char *ifname"))
assert 'strcmp(ifname, "all") != 0' in global_writer and 'strcmp(ifname, "default") != 0' in global_writer
assert "!is_interface_exist(ifname)" in global_writer
assert 'return set_interface_conf_int("ipv6", ifname, param, value);' in global_writer
global_conf = compact(body(NET6_SOURCE, "control_if_ipv6_all(int enable)"))
assert "rc_ipv6_conf_apply(enable_ops" in global_conf and "rc_ipv6_conf_apply(disable_ops" in global_conf
assert "int write_status = fput_int(path, 900);" in global_conf and "snprintf(path, sizeof(path)" in global_conf
assert "int control_if_ipv6_all(int enable);" in HEADER and "void control_if_ipv6_all(int enable);" not in HEADER
assert NET6_SOURCE.count("operation_status = control_if_ipv6_all(") == 2
assert "IPv6 disable could not apply global sysctls (status=%d)" in NET6_SOURCE
assert "IPv6 enable could not apply global sysctls (status=%d)" in NET6_SOURCE

# Dynamic WAN IPv6 addresses are passed directly as argv, not through a shell.
assert 'doSystem("ip -6 addr add %s dev %s"' not in NET_WAN6_SOURCE
assert NET_WAN6_SOURCE.count('eval("ip", "-6", "addr", "add",') == 3
assert 'inet_ntop(AF_INET6, &addr6, addr6s, INET6_ADDRSTRLEN)' in NET_WAN6_SOURCE
assert 'inet_ntop(AF_INET6, &lan_addr6, lan_addr6s, INET6_ADDRSTRLEN)' in NET_WAN6_SOURCE
assert 'format_ipv6_literal(wan_addr6, addr6_text,' in NET_WAN6_SOURCE

# PPP probe gateways and custom WAN hook arguments must remain argv elements.
ppp_wait = body(WAN_SOURCE, "wait_ppp_up(char *ppp_ifname, int unit)")
assert "if (is_valid_ipv4(ppp_gate))" in ppp_wait
assert 'eval("ping", "-c1", ppp_gate);' in ppp_wait
assert 'doSystem("ping -c1 %s", ppp_gate)' not in ppp_wait

wan_up_internal = body(WAN_SOURCE, "wan_up_internal(char *wan_ifname, int unit, int is_static)")
assert 'eval(script_postw, "up", wan_ifname, wan_addr);' in wan_up_internal
assert 'doSystem("%s %s %s %s", script_postw, "up"' not in wan_up_internal
wan_down_internal = body(WAN_SOURCE, "wan_down_internal(char *wan_ifname, int unit, int is_static)")
assert 'eval(script_postw, "down", wan_ifname, wan_addr);' in wan_down_internal
assert 'doSystem("%s %s %s %s", script_postw, "down"' not in wan_down_internal

# Every WAN IPv6 tunnel and route command must use exec argv rather than a
# shell-formatted command, and every setup mutation must retain status gating.
sit_start = body(NET_WAN6_SOURCE, "int start_sit_tunnel(int ipv6_type")
native_start = body(NET_WAN6_SOURCE, "int wan6_up(char *wan_ifname, int unit)")
wan6_down = body(NET_WAN6_SOURCE, "int wan6_down(char *wan_ifname, int unit)")
stop_sit = body(NET_WAN6_SOURCE, "int stop_sit_tunnel(void)")
assert "doSystem(" not in NET_WAN6_SOURCE
for marker in (
    'WAN6_SIT_CHECK(eval("ip", "tunnel", "del", IFNAME_SIT)',
    'WAN6_SIT_CHECK(eval("ip", "tunnel", "add", IFNAME_SIT, "mode", "sit"',
    'WAN6_SIT_CHECK(eval("ip", "tunnel", "6rd", "dev", IFNAME_SIT',
    'WAN6_SIT_CHECK(eval("ip", "link", "set", "mtu", sit_mtu_text',
    'WAN6_SIT_CHECK(eval("ip", "-6", "route", "add", "default", "dev", IFNAME_SIT, "metric", "2048")',
    'WAN6_SIT_CHECK(eval("ip", "-6", "route", "add", "default", "via", wan_gate6, "dev", IFNAME_SIT, "metric", "1")',
):
    assert marker in sit_start, f"missing status-gated SIT argv command: {marker}"
for marker in (
    'WAN6_NATIVE_CHECK(eval("ip", "-6", "route", "add", gate6_text, "dev", wan_ifname)',
    'WAN6_NATIVE_CHECK(eval("ip", "-6", "route", "add", "default", "via", gate6_text, "metric", "1")',
    'WAN6_NATIVE_CHECK(eval("ip", "-6", "route", "add", "default", "dev", wan_ifname, "metric", "2048")',
):
    assert marker in native_start, f"missing status-gated native argv command: {marker}"
assert 'char sit_mtu_text[12], sit_ttl_text[4];' in NET_WAN6_SOURCE
assert 'snprintf(sit_mtu_text, sizeof(sit_mtu_text), "%d", sit_mtu)' in sit_start
assert 'snprintf(sit_ttl_text, sizeof(sit_ttl_text), "%d", sit_ttl)' in sit_start
assert sit_start.index("snprintf(sit_mtu_text") < sit_start.index("WAN6_SIT_CHECK(")
assert sit_start.index("snprintf(sit_ttl_text") < sit_start.index("WAN6_SIT_CHECK(")
assert 'cleanup_status = eval("ip", "link", "set", "dev", IFNAME_SIT, "down");' in sit_start
assert 'operation_status = eval("ip", "link", "set", "dev", IFNAME_SIT, "down");' in wan6_down
assert 'int clear_if_neigh6(char *ifname);' in HEADER
native_failed = native_start[native_start.index("native_failed:"):]
assert "if (routes_touched || addresses_touched)" in native_failed
assert "operation_status = clear_if_neigh6(wan_ifname);" in native_failed
native_route_cleanup = native_failed.index("operation_status = clear_if_route6(wan_ifname);")
native_neigh_cleanup = native_failed.index("operation_status = clear_if_neigh6(wan_ifname);")
native_addr_cleanup = native_failed.index("operation_status = clear_if_addr6(wan_ifname);")
assert native_route_cleanup < native_neigh_cleanup < native_addr_cleanup
rollback_steps = (
    "if (radv_touched)",
    "if (autoconf_touched)",
    "if (privacy_touched)",
    "if (routes_touched)",
    "if (routes_touched || addresses_touched)",
    "if (addresses_touched)",
)
native_rollback_positions = [native_failed.index(step) for step in rollback_steps]
native_identity_gate = native_failed.index("if (identity_publish_failed)")
native_identity_clear = native_failed.index("wan6_clear_native_identity_after_rollback(", native_identity_gate)
assert native_rollback_positions == sorted(native_rollback_positions)
assert native_rollback_positions[-1] < native_identity_gate < native_identity_clear
assert "operation_status = clear_if_neigh6(wan6_ifname);" in wan6_down
wan_route_cleanup = wan6_down.index("operation_status = clear_if_route6(wan6_ifname);")
wan_neigh_cleanup = wan6_down.index("operation_status = clear_if_neigh6(wan6_ifname);")
wan_addr_cleanup = wan6_down.index("operation_status = clear_if_addr6(wan6_ifname);")
assert wan_route_cleanup < wan_neigh_cleanup < wan_addr_cleanup
# The tenth aggregate preserves checked IPv6 runtime-metadata clear failures.
assert wan6_down.count("status = route_status_accumulate(status, operation_status);") == 10
assert 'return eval("ip", "tunnel", "del", IFNAME_SIT);' in stop_sit

for signature in (
    "full_restart_wan(void)",
    "try_wan_reconnect(int try_use_modem, long pause_in_seconds)",
    "deferred_wan_connect(void)",
):
    function = compact(body(WAN_SOURCE, signature))
    assert "status = start_wan();" in function
    assert "return merge_l2tp_server_start_status(status);" in function

wan_stop = compact(body(WAN_SOURCE, "stop_wan_locked(void)"))
assert "stop_status = wan_down(man_ifname, unit, 1);" in wan_stop
assert "static WAN route teardown incomplete" in wan_stop
assert "int modem_stop_status = stop_wan_usbnet();" in wan_stop
assert "rc_first_service_start_status(stop_status, modem_stop_status)" in wan_stop
assert wan_stop.index("stop_status = wan_down(man_ifname, unit, 1);") < wan_stop.index("int modem_stop_status = stop_wan_usbnet();")

for signature, call in (
    ("manual_wan_reconnect(void)", "try_wan_reconnect(1, 0)"),
    ("auto_wan_reconnect(void)", "try_wan_reconnect(1, 0)"),
    ("auto_wan_reconnect_pause(void)", "try_wan_reconnect(1, pause_seconds)"),
):
    assert f"return {call};" in compact(body(WAN_SOURCE, signature))

pause_reconnect = "".join(compact(body(WAN_SOURCE, "auto_wan_reconnect_pause(void)")).split())
pause_read = 'status=get_nvram_int_checked_status("di_recon_pause",&pause_seconds);'
pause_gate = "if(status!=0){"
pause_error = "".join("aborting automatic WAN reconnect because delay configuration is unavailable".split())
# pause_reconnect has all whitespace removed above, including inside literals.
pause_return = "returnstatus;"
pause_start = "returntry_wan_reconnect(1,pause_seconds);"
pause_positions = (
    pause_reconnect.index(pause_read),
    pause_reconnect.index(pause_gate),
    pause_reconnect.index(pause_error),
    pause_reconnect.index(pause_return, pause_reconnect.index(pause_gate)),
    pause_reconnect.index(pause_start),
)
assert pause_positions == tuple(sorted(pause_positions))
assert 'nvram_get_int("di_recon_pause")' not in pause_reconnect

init = compact(body(RC_SOURCE, "init_router(void)"))
assert "wan_start_status = start_wan();" in init
assert re.search(
    r"boot_services_status\s*=\s*rc_first_service_start_status\s*\(\s*boot_services_status\s*,\s*wan_start_status\s*\)\s*;",
    init,
)

for call in (
    "full_restart_wan()",
    "deferred_wan_connect()",
    "auto_wan_reconnect()",
    "auto_wan_reconnect_pause()",
    "manual_wan_reconnect()",
):
    notify_pattern = (
        r"rc_notify_merge_action_result\s*\(\s*action_result\s*,\s*"
        + re.escape(call)
        + r"\s*\)"
    )
    assert re.search(notify_pattern, RC_SOURCE), f"RC notification must preserve {call} status"

for declaration in (
    "int start_wan(void);",
    "int full_restart_wan(void);",
    "int try_wan_reconnect(int try_use_modem, long pause_in_seconds);",
    "int deferred_wan_connect(void);",
    "int auto_wan_reconnect(void);",
    "int auto_wan_reconnect_pause(void);",
    "int manual_wan_reconnect(void);",
):
    assert declaration in HEADER


deferred_hotplug = compact(body(STORAGE_SOURCE, "on_deferred_hotplug_dev(void)"))
modem_start = compact(body(SERVICES_USB_SOURCE, "try_start_usb_modem_to_wan(void)"))
assert "return try_wan_reconnect(1, 0);" in modem_start
assert "return;" not in modem_start
modem_start_tokens = "".join(modem_start.split())
assert "modem_config_status=get_modem_wan_start_config_status(&modem_prio,&modem_devnum)" in modem_start_tokens
assert "if(modem_config_status!=0)" in modem_start_tokens
assert "returnmodem_config_status;" in modem_start_tokens
assert modem_start_tokens.index("modem_config_status=get_modem_wan_start_config_status(") < modem_start_tokens.index("if(modem_config_status!=0)") < modem_start_tokens.index("returnmodem_config_status;") < modem_start_tokens.index("if(modem_prio<1)")
assert modem_start_tokens.index("if(modem_prio<1)") < modem_start_tokens.index("get_usb_modem_wan_status(")
assert 'nvram_get_int("modem_prio")' not in modem_start
assert "!get_modem_devnum()" not in modem_start

modem_snapshot = compact(body(USB_SOURCE, "usb_modem_read_start_config_snapshot("))
modem_field_reader = compact(body(USB_SOURCE, "usb_modem_read_start_config_field("))
for key in ("modem_prio", "modem_rule", "modem_type"):
    assert '"' + key + '"' in modem_snapshot
for marker in ("usb_modem_nvram_transaction_begin", "usb_modem_nvram_transaction_end", "usb_modem_read_start_config_field("):
    assert marker in modem_snapshot
assert "usb_modem_nvram_transaction_get" in modem_field_reader
assert "rc_wan_nvram_read_status(" in modem_field_reader
assert "rc_wan_nvram_copy_text(" in modem_field_reader

modem_config = compact(body(USB_SOURCE, "get_modem_wan_start_config_status("))
for key in ("config.modem_prio", "config.modem_rule", "config.modem_type"):
    assert key in modem_config
assert "usb_modem_read_start_config_snapshot(&config)" in modem_config
assert "rc_wan_nvram_parse_int_value(" in modem_config
assert "is_ready_modem_ndis(" in modem_config and "is_ready_modem_ras(" in modem_config

notification_config = compact(body(USB_SOURCE, "get_modem_hotplug_notification_status("))
assert "usb_modem_read_start_config_snapshot(&config)" in notification_config
for key in ("config.modem_prio_present", "config.modem_rule_present", "config.modem_type_present"):
    assert key in notification_config
assert "modem_wan_hotplug_notification_enabled(" in notification_config

modem_wrapper = "".join(compact(body(USB_SOURCE, "get_modem_devnum(void)")).split())
assert "get_modem_wan_start_config_status(&modem_priority,&modem_devnum)" in modem_wrapper
assert "cannotreadUSBmodemWANconfiguration" in modem_wrapper
assert 'nvram_get_int("modem_rule")' not in modem_wrapper
assert 'nvram_get_int("modem_type")' not in modem_wrapper

manual_node_reader = "".join(compact(body(USB_SOURCE, "usb_modem_read_manual_node_index_status(")).split())
assert "usb_modem_nvram_transaction_get(&transaction,\"modem_node\",&value)" in manual_node_reader
assert "rc_wan_nvram_read_int_status(" in manual_node_reader
assert "usb_modem_nvram_transaction_end(&transaction)" in manual_node_reader
assert "modem_node_manual_index_status(" in manual_node_reader
manual_node_selector = "".join(compact(body(USB_SOURCE, "get_modem_node_by_usb_identity(")).split())
assert "usb_modem_read_manual_node_index_status(&manual_index)" in manual_node_selector
assert "if(status!=0)" in manual_node_selector and "return-2;" in manual_node_selector
assert 'nvram_get_int("modem_node")' not in USB_SOURCE
assert "modem_node_manual_index_status" in MODEM_NODE_POLICY_SOURCE
for marker in (
    "test_manual_node_index_status_is_fail_closed",
    "test_manual_node_index_status_is_fail_closed();",
):
    assert marker in MODEM_NODE_TEST_SOURCE
ras_node_selector = "".join(compact(body(USB_SOURCE, "get_modem_node_ras(")).split())
assert "usb_modem_read_manual_node_index_status(&manual_index)" in ras_node_selector
assert "*selection_status_out=status;" in ras_node_selector
assert 'get_modem_node("ttyACM",0,manual_index,devnum_out)' in ras_node_selector
ras_readiness = "".join(compact(body(USB_SOURCE, "is_ready_modem_ras(")).split())
assert "selection_status_out" in ras_readiness
assert "get_modem_node_ras(node_name,devnum_out,selection_status_out)" in ras_readiness
ndis_control = "".join(compact(body(USB_SOURCE, "ndis_control_network(")).split())
assert "manual_node_status=errno>0?errno:EIO;" in ndis_control
assert "returnmanual_node_status;" in ndis_control
assert "cannotreadmanualNDIScontrol-nodeselection" in ndis_control

ndis_hotplug = compact(body(HOTPLUG_SOURCE, "int mdev_net_main("))
tty_hotplug = compact(body(HOTPLUG_SOURCE, "int mdev_tty_main("))
for hotplug_body, transport in ((ndis_hotplug, 1), (tty_hotplug, 0)):
    hotplug_tokens = "".join(hotplug_body.split())
    policy_call = f"get_modem_hotplug_notification_status({transport},&notify_modem)"
    assert policy_call in hotplug_tokens
    assert "if(modem_hotplug_policy_status!=0)" in hotplug_tokens
    assert "notify_modem=1;" in hotplug_tokens
    assert "if(notify_modem)" in hotplug_tokens
    assert hotplug_tokens.index(policy_call) < hotplug_tokens.index("if(modem_hotplug_policy_status!=0)") < hotplug_tokens.index("notify_modem=1;") < hotplug_tokens.index("if(notify_modem)")
assert 'nvram_invmatch("modem_prio"' not in HOTPLUG_SOURCE
assert 'nvram_match("modem_rule"' not in HOTPLUG_SOURCE

deferred_code = "".join(deferred_hotplug.split())
printer_unplug_read = 'storage_read_integer_status("usb_unplug_lp",0,&printer_unplug_value)'
printer_hotplug_read = 'storage_read_integer_status("usb_hotplug_lp",0,&printer_hotplug_value)'
modem_unplug_read = 'storage_read_integer_status("usb_unplug_md",0,&modem_unplug_value)'
modem_hotplug_read = 'storage_read_integer_status("usb_hotplug_md",0,&modem_hotplug_value)'
assert printer_unplug_read in deferred_code
assert printer_hotplug_read in deferred_code
assert modem_unplug_read in deferred_code
assert modem_hotplug_read in deferred_code
assert deferred_code.count("usb_hotplug_event_pair_status(") == 2
for marker in ("usb_unplug_lp", "usb_hotplug_lp", "usb_unplug_md", "usb_hotplug_md"):
    assert f'nvram_match("{marker}","1")' not in deferred_code

printer_pair = deferred_code.index("usb_hotplug_event_pair_status(")
modem_pair = deferred_code.index("usb_hotplug_event_pair_status(", printer_pair + 1)
assert deferred_code.index(printer_unplug_read) < deferred_code.index(printer_hotplug_read) < printer_pair
assert deferred_code.index(modem_unplug_read) < deferred_code.index(modem_hotplug_read) < modem_pair
assert "cannot read deferred printer event markers; preserving pending events for retry" in deferred_hotplug
assert "cannot read deferred modem event markers; preserving pending events for retry" in deferred_hotplug

modem_rule_read = 'storage_read_integer_status("modem_rule",0,&modem_rule_value)'
modem_rule_policy = 'modem_wan_unplug_event_status(modem_rule_status,modem_rule_value,&unplug_modem)'
unplug_ack = 'nvram_set_int_temp("usb_unplug_md",0)'
assert modem_rule_read in deferred_code
assert modem_rule_policy in deferred_code
rule_read_pos = deferred_code.index(modem_rule_read, modem_pair)
rule_policy_pos = deferred_code.index(modem_rule_policy, rule_read_pos)
rule_success_pos = deferred_code.index("if(modem_rule_status==0)", rule_policy_pos)
unplug_ack_pos = deferred_code.index(unplug_ack, rule_success_pos)
assert modem_pair < rule_read_pos < rule_policy_pos < rule_success_pos < unplug_ack_pos
assert 'nvram_get_int("modem_rule")' not in deferred_hotplug
assert "unplug_modem=1;" in deferred_code
assert "modem_unplug_status=modem_rule_status;" in deferred_code
assert "if(unplug_modem)" in deferred_code
assert "modem_unplug_status=try_wan_reconnect(1,0)" in deferred_code
assert "if(modem_rule_status==0)" in deferred_code
assert "modem_plug_status=try_start_usb_modem_to_wan();" in deferred_code
assert "MODEM_WAN_HOTPLUG_RETRY_UNPLUG" in deferred_hotplug
assert "MODEM_WAN_HOTPLUG_RETRY_PLUG" in deferred_hotplug
assert 'nvram_set_int_temp("usb_unplug_md",1)' in deferred_code
assert 'nvram_set_int_temp("usb_hotplug_md",1)' in deferred_code
assert "modem_wan_hotplug_retry_mask(" in deferred_code
assert "alarm(5);" in deferred_hotplug

usb_event_policy = (RC / "usb_hotplug_event_policy.h").read_text()
usb_event_test = (RC / "tests/usb_hotplug_event_policy.c").read_text()
assert "usb_hotplug_event_pair_status" in usb_event_policy
assert "test_each_failed_marker_read_preserves_the_pair" in usb_event_test

modem_test = (RC / "tests/modem_wan_start_policy.c").read_text()
assert "test_modem_unplug_rule_read_fails_closed" in modem_test
assert "test_modem_unplug_rule_read_fails_closed();" in modem_test
assert "modem_wan_hotplug_notification_enabled" in POLICY_SOURCE
for marker in (
    "test_hotplug_notification_preserves_ndis_and_ras_legacy_gates",
    "test_hotplug_notification_missing_any_legacy_match_key_stays_disabled",
    "test_hotplug_notification_rejects_invalid_and_unreadable_inputs",
):
    assert marker in modem_test and marker + "();" in modem_test
assert "modem_plug_status = try_start_usb_modem_to_wan();" in deferred_hotplug
assert "MODEM_WAN_HOTPLUG_RETRY_UNPLUG" in deferred_hotplug
assert "MODEM_WAN_HOTPLUG_RETRY_PLUG" in deferred_hotplug
assert 'nvram_set_int_temp("usb_unplug_md", 1)' in deferred_hotplug
assert 'nvram_set_int_temp("usb_hotplug_md", 1)' in deferred_hotplug
assert "if (modem_retry_mask != 0)" in deferred_hotplug
assert "alarm(5);" in deferred_hotplug
assert "test_modem_hotplug_retry_mask_preserves_failed_and_coalesced_events" in (
    RC / "tests/modem_wan_start_policy.c"
).read_text()

firewall_restart = compact(body(NET_SOURCE, "restart_firewall(void)"))
assert "int restart_firewall(void);" in HEADER
assert "upnp_status = update_upnp();" in firewall_restart
assert "if (upnp_status != 0)" in firewall_restart
assert "firewall_status = start_firewall_ex();" in firewall_restart
assert "return rc_first_service_start_status(firewall_status, upnp_status);" in firewall_restart

notify = compact(body(RC_SOURCE, "handle_notifications(void)"))
for marker in (
    "RCN_RESTART_IPTV",
    "RCN_RESTART_NETFILTER",
    "RCN_RESTART_FIREWALL",
    '"restart_firewall_wan"',
):
    marker_pos = notify.index(marker)
    branch_end = notify.find("else if", marker_pos + len(marker))
    branch = notify[marker_pos:branch_end if branch_end >= 0 else len(notify)]
    assert re.search(
        r"rc_notify_merge_action_result\s*\(\s*action_result\s*,\s*restart_firewall\(\)\s*\)\s*;",
        branch,
    ), marker + " must preserve firewall/UPnP refresh failure in RC notification"

cli_pos = RC_SOURCE.index('else if (!strcmp(base, "restart_firewall"))')
cli_end = RC_SOURCE.index('else if (!strcmp(base, "radio2_toggle"))', cli_pos)
assert "ret = restart_firewall();" in RC_SOURCE[cli_pos:cli_end]

sshd_restart = compact(body(SERVICES_SOURCE, "int restart_sshd(void)"))
assert "firewall_status = restart_firewall();" in sshd_restart
assert "if (start_status != 0)" in sshd_restart
assert "PROCESS_PRESENCE_UNKNOWN" in sshd_restart
assert "return firewall_status;" in sshd_restart
assert sshd_restart.index("if (start_status != 0)") < sshd_restart.index("return firewall_status;")
assert sshd_restart.index("PROCESS_PRESENCE_UNKNOWN") < sshd_restart.index("return firewall_status;")

httpd_start = compact(body(SERVICES_SOURCE, "start_httpd_locked(int restart_fw)"))
httpd_start_entry = compact(body(SERVICES_SOURCE, "start_httpd(int restart_fw)"))
assert "httpd_lifecycle_lock_acquire_status" in httpd_start_entry
assert "start_httpd_locked(restart_fw)" in httpd_start_entry
assert "httpd_lifecycle_lock_finish" in httpd_start_entry
assert "firewall_status = restart_firewall();" in httpd_start
assert "errno = 0;" in httpd_start
assert "firewall_errno = errno > 0 ?" in httpd_start
assert "httpd_firewall_failure_requires_stop(firewall_status, presence)" in httpd_start
assert "firewall_stop_status = stop_httpd_locked();" in httpd_start
assert "firewall_stop_state = httpd_process_presence(&firewall_stop_scan_error);" in httpd_start
assert "if (firewall_stop_state == PROCESS_PRESENCE_ABSENT)" in httpd_start
assert 'httpd_nvram_set_int_status("http_lanport", http_port, 0)' in httpd_start
assert 'httpd_nvram_set_int_status("https_lport", https_port, 0)' in httpd_start
assert "httpd_started_state_publish_status(0)" in httpd_start
assert "httpd_started_state_publish_status(1)" in httpd_start
assert 'nvram_set_int_temp("httpd_started"' not in httpd_start
assert httpd_start.index("httpd_process_presence(&scan_error)") < httpd_start.index('httpd_nvram_set_int_status("http_lanport", http_port, 0)')
assert httpd_start.index('httpd_started_state_publish_status(0)') < httpd_start.index("launch_status = _eval(httpd_argv, NULL, 0, NULL);")
assert "errno = firewall_errno;" in httpd_start
assert "return firewall_status;" in httpd_start
httpd_firewall_start = httpd_start.index("firewall_status = restart_firewall();")
httpd_firewall = httpd_start[httpd_firewall_start:]
assert "status = httpd_started_state_publish_status(1);" in httpd_firewall
assert "if (status != 0)" in httpd_firewall
assert "cannot publish HTTPD possibly-running state after firewall failure" in httpd_firewall
assert 'nvram_set_int_temp("httpd_started"' not in httpd_firewall
httpd_firewall_order = (
    httpd_firewall.index("firewall_status = restart_firewall();"),
    httpd_firewall.index("firewall_errno = errno > 0 ?"),
    httpd_firewall.index("httpd_firewall_failure_requires_stop(firewall_status, presence)"),
    httpd_firewall.index("firewall_stop_status = stop_httpd_locked();"),
    httpd_firewall.index("firewall_stop_state = httpd_process_presence(&firewall_stop_scan_error);"),
    httpd_firewall.index("if (firewall_stop_state == PROCESS_PRESENCE_ABSENT)"),
    httpd_firewall.index("status = httpd_started_state_publish_status(1);"),
    httpd_firewall.index("if (status != 0)"),
    httpd_firewall.index("errno = firewall_errno;"),
    httpd_firewall.index("return firewall_status;"),
)
assert httpd_firewall_order == tuple(sorted(httpd_firewall_order))
httpd_firewall_policy = compact(body(HTTPD_POLICY_SOURCE, "httpd_firewall_failure_requires_stop("))
assert "firewall_status != 0" in httpd_firewall_policy
assert "after_state != PROCESS_PRESENCE_ABSENT" in httpd_firewall_policy
assert "test_firewall_failure_requires_listener_shutdown" in HTTPD_POLICY_TEST_SOURCE
httpd_firewall_test = compact(body(HTTPD_POLICY_TEST_SOURCE, "test_firewall_failure_requires_listener_shutdown(void)"))
for state in ("PROCESS_PRESENCE_ABSENT", "PROCESS_PRESENCE_RUNNING",
              "PROCESS_PRESENCE_UNKNOWN", "(enum process_presence)99"):
    assert state in httpd_firewall_test

notify_policy = (RC / "rc_notify_batch_policy.h").read_text()
notify_test = (RC / "tests/rc_notify_batch_policy.c").read_text()
assert "operation_status != 0" in notify_policy
assert "rc_notify_merge_action_result(0, EBUSY) == -1" in notify_test
assert "rc_notify_merge_action_result(0, EIO) == -1" in notify_test


ftp_fw = compact(body(STORAGE_SOURCE, "control_ftp_fw(int is_run_before)"))
assert "return restart_firewall();" in ftp_fw
assert "int control_ftp_fw(int is_run_before);" in HEADER
ftp_restart = compact(body(STORAGE_SOURCE, "restart_ftpd(void)"))
assert "firewall_status = control_ftp_fw(is_run_before);" in ftp_restart
assert "rc_first_service_start_status(start_status, firewall_status)" in ftp_restart
for signature in ("restart_torrent(void)", "restart_aria(void)", "stop_stor_apps(void)", "start_stor_apps(void)"):
    service_body = compact(body(STORAGE_SOURCE, signature))
    assert "firewall_status = restart_firewall();" in service_body, signature
    assert "rc_first_service_start_status" in service_body
ipv6_restart = compact(body(NET6_SOURCE, "full_restart_ipv6(int ipv6_type_old)"))
assert "operation_status = restart_firewall();" in ipv6_restart
assert "rc_first_service_start_status(restart_status, operation_status)" in ipv6_restart
vpn_restart = compact(body(VPN_SERVER_SOURCE, "restart_vpn_server(void)"))
assert "firewall_status = restart_firewall();" in vpn_restart
assert "vpn_server_restart_first_error(restart_status, firewall_status)" in vpn_restart
upnp_restart = compact(body(SERVICES_EX_SOURCE, "restart_upnp(void)"))
assert "miniupnpd_lifecycle_lock_acquire()" in upnp_restart
assert "rc_upnp_restart_run(stop_upnp_locked, restart_firewall)" in upnp_restart
assert 'miniupnpd_lifecycle_lock_finish(lock_fd, ret, "restart")' in upnp_restart
assert upnp_restart.index("miniupnpd_lifecycle_lock_acquire()") < upnp_restart.index("rc_upnp_restart_run(stop_upnp_locked, restart_firewall)") < upnp_restart.index('miniupnpd_lifecycle_lock_finish(lock_fd, ret, "restart")')
assert "typedef int (*rc_upnp_firewall_restart_fn)(void);" in UPNP_POLICY_SOURCE
upnp_runner = compact(body(UPNP_POLICY_SOURCE, "rc_upnp_restart_run("))
assert "return restart_firewall();" in upnp_runner
assert "test_firewall_refresh_failure_is_propagated" in UPNP_TEST_SOURCE
for source_name, source in (("services_stor.c", STORAGE_SOURCE), ("net6.c", NET6_SOURCE), ("vpn_server.c", VPN_SERVER_SOURCE)):
    assert not any(line.strip() == "restart_firewall();" for line in source.splitlines()), (
        "unchecked firewall result remains in " + source_name
    )
print("WAN/storage/IPv6/VPN UPnP firewall result propagation: PASS")
print("WAN firewall UPnP refresh status propagation: PASS")
print("USB modem WAN startup status propagation: PASS")


# VPN client is also a direct restart_firewall() consumer. Preserve its first
# operation result and errno instead of silently discarding the firewall error.
vpn_client_restart = compact(body(VPN_CLIENT_SOURCE, "restart_vpn_client_locked(void)"))
assert "firewall_status = restart_firewall();" in vpn_client_restart
assert "if (start_status == 0 && firewall_status != 0)" in vpn_client_restart
assert "if (start_status != 0 && saved_errno != 0) errno = saved_errno;" in vpn_client_restart

vpn_server_restart = compact(body(VPN_SERVER_SOURCE, "restart_vpn_server(void)"))
assert "if (restart_status == 0) saved_errno = errno ? errno : EIO;" in vpn_server_restart

direct_firewall_references = sum(
    len(re.findall(r"\brestart_firewall\s*\(", source.read_text()))
    for source in RC.glob("*.c")
)
assert direct_firewall_references == 16, (
    "update the direct restart_firewall caller audit after a new call site: "
    + str(direct_firewall_references)
)
print("VPN client/server and complete direct firewall caller status contracts: PASS")

# Optional SFE and HW NAT transitions must use strict tri-state
# postcondition helpers; legacy Boolean probes can hide sysfs failures.
assert '#include "wireless_module_load_policy.h"' in NET_SOURCE
nat_reload = compact(body(NET_SOURCE, "reload_nat_modules(void)"))
assert "if (sfe_enable)" in nat_reload
assert 'module_status = rc_module_load_verified("fast_classifier", NULL, &module_ops);' in nat_reload
assert 'module_status = rc_module_unload_verified("fast_classifier", 1, &module_unload_ops);' in nat_reload
assert "if (sfe_loaded)" in nat_reload
assert "continuing without SFE" in nat_reload
assert 'is_module_loaded("fast_classifier")' not in nat_reload
assert 'rc_module_load_verified("hw_nat", hnat_param, &module_ops)' in nat_reload
assert 'rc_module_unload_verified("hw_nat", 0, &module_unload_ops)' in nat_reload
assert "if (!hwnat_allow)" in nat_reload
assert "if (hwnat_loaded == 0)" in nat_reload
assert "if (hwnat_loaded == 1)" in nat_reload
assert "hwnat_loaded = is_hwnat_loaded_status();" in nat_reload
assert 'is_hwnat_loaded();' not in nat_reload
assert 'if (hwnat_loaded && !hwnat_allow)' not in nat_reload
assert "int hwnat_loaded = 0;" in nat_reload
assert "hwnat_configure(hwnat_wan_vid, hwnat_allow);" in nat_reload
assert 'return is_module_loaded_status("hw_nat");' in NET_SOURCE

assert 'ftp_module_loaded = is_module_loaded_status("nf_conntrack_ftp");' in nat_reload
assert "if (ftp_module_loaded < 0)" in nat_reload
assert "module_status = -ftp_module_loaded;" in nat_reload
assert "if (ftp_reconfigure_allowed && ftp_module_loaded == 1 && loaded_ftp == 0)" in nat_reload
assert 'is_module_loaded("nf_conntrack_ftp")' not in nat_reload
assert 'module_smart_load("hw_nat", hnat_param);' not in nat_reload
assert 'module_smart_load("fast_classifier", NULL);' not in nat_reload
assert 'rc_module_unload_verified("nf_nat_pptp", 1, &module_unload_ops)' in nat_reload
assert 'rc_module_unload_verified("nf_nat_h323", 1, &module_unload_ops)' in nat_reload
assert 'rc_module_unload_verified("nf_nat_sip", 1, &module_unload_ops)' in nat_reload
assert 'rc_module_unload_verified("nf_nat_ftp", 0, &module_unload_ops)' in nat_reload
assert 'rc_module_unload_verified("nf_conntrack_ftp", 0, &module_unload_ops)' in nat_reload
assert "rc_module_unload_verified" in MODULE_POLICY_SOURCE
load_policy = compact(body(MODULE_POLICY_SOURCE, "rc_module_load_verified("))
assert "loaded = ops->is_loaded(module_name);" in load_policy
assert "if (loaded < 0)" in load_policy
assert "return -loaded;" in load_policy
assert "return loaded == 1 ? 0 : EIO;" in load_policy
module_loaders = (
    compact(body(NET_SOURCE, "reload_nat_modules(void)")),
    compact(body(RC_SOURCE, "load_boot_module(const char *module_name, const char *module_param)")),
    compact(body(RC_SOURCE, "load_wireless_module(const char *module_name)")),
    compact(body(NET_LAN_SOURCE, "init_bridge_load_module_verified(const char *module_name, const char *module_param)")),
)
for loader_body in module_loaders:
    assert "module_smart_load, is_module_loaded_status" in loader_body
    assert "module_smart_load, is_module_loaded)" not in loader_body
assert "test_load_probe_error_is_reported" in MODULE_POLICY_TEST
assert "test_required_module_set_probe_error_attempts_all_and_keeps_first_failure" in MODULE_POLICY_TEST
assert "is_module_loaded_status(const char *module_name)" in COMMON_EX_SOURCE
assert "opendir(\"/sys/module\")" in COMMON_EX_SOURCE
assert "return is_module_loaded_status(module_name) > 0;" in COMMON_EX_SOURCE
assert "if (errno == ENOENT)" in COMMON_EX_SOURCE
assert "int is_module_loaded_status(const char *module_name);" in HEADER
assert "test_unload_busy_module_is_not_reported_as_removed" in MODULE_POLICY_TEST
assert "test_unload_requirement_set_attempts_all_and_keeps_first_failure" in MODULE_POLICY_TEST
assert "test_unload_initial_probe_error_fails_closed" in MODULE_POLICY_TEST
assert "test_unload_postcondition_probe_error_is_reported" in MODULE_POLICY_TEST

# HW NAT must not be reported enabled unless the WAN VID sysfs write succeeds.
hwnat_config = compact(body(NET_SOURCE, "hwnat_configure(int wan_vid, int hwnat_allow)"))
assert "if (!hwnat_allow)" in hwnat_config
assert "module_loaded = is_hwnat_loaded_status();" in hwnat_config
assert "if (module_loaded < 0)" in hwnat_config
assert "status = -module_loaded;" in hwnat_config
assert "rc_hwnat_configure_policy(hwnat_allow, module_loaded == 1, wan_vid, hwnat_set_wan_vid)" in hwnat_config
assert "return status;" in hwnat_config
assert "module_param_set_int" not in hwnat_config
assert hwnat_config.index("if (!hwnat_allow)") < hwnat_config.index("module_loaded = is_hwnat_loaded_status()") < hwnat_config.index("rc_hwnat_configure_policy(")
assert "hwnat_set_wan_vid(int wan_vid)" in compact(NET_SOURCE)
assert "test_wan_vid_write_failure_is_reported" in HWNAT_POLICY_TEST
assert "test_missing_wan_vid_is_reported" in HWNAT_POLICY_TEST
assert "rc_sfe_configure_policy(sfe_enable, sfe_set_skip_to_bridge_ingress)" in nat_reload
assert "fput_int(\"/sys/fast_classifier/skip_to_bridge_ingress\", skip)" in NET_SOURCE
assert "doSystem(\"echo 0 > /sys/fast_classifier/skip_to_bridge_ingress\")" not in nat_reload
assert "doSystem(\"echo 1 > /sys/fast_classifier/skip_to_bridge_ingress\")" not in nat_reload
assert "test_failed_sfe_config_write_is_reported" in SFE_POLICY_TEST
assert "sfe_config_policy.h" in NET_SOURCE
assert "wan_vid, status" in hwnat_config
assert '#include "hwnat_config_policy.h"' in NET_SOURCE

# doSystem() returns system(3)'s encoded wait status. Normalize failures and
# still attempt independent 2.4 GHz, 5 GHz and IPv6 settings.
radio_policy = compact(body(NET_SOURCE, "hwnat_set_radio_registration(const char *ifname, int enabled)"))
ipv6_policy = compact(body(NET_SOURCE, "hwnat_set_ipv6_nat_debug(int enabled)"))
assert 'doSystem("iwpriv %s set hw_nat_register=%d", ifname, enabled)' in radio_policy
assert 'doSystem("echo 7 %d > /sys/kernel/debug/hnat/hnat_setting", enabled)' in ipv6_policy
assert "rc_hwnat_command_status_policy(command_status, saved_errno)" in compact(NET_SOURCE)
assert "WIFEXITED(wait_status)" in HWNAT_POLICY_SOURCE
assert "rc_hwnat_configure_radio_policy(" in nat_reload
assert "if (hwnat_loaded == 1)" in nat_reload
assert "hwnat_allow, 1, nvram_get_int(\"hw_nat_mode\")" in nat_reload
assert "is_hwnat_loaded();" not in nat_reload
assert "if (hwnat_loaded && !hwnat_allow)" not in nat_reload
assert "hwnat_set_radio_registration, hwnat_set_ipv6_nat_debug" in nat_reload
assert "test_system_wait_status_is_normalized" in HWNAT_POLICY_TEST
assert "test_radio_failures_do_not_skip_independent_writes" in HWNAT_POLICY_TEST
assert "test_disabled_or_unloaded_radio_policy_has_no_side_effects" in HWNAT_POLICY_TEST
print("Optional SFE, HW NAT WAN VID and radio/debugfs configuration result contracts: PASS")

# Configured NAT helper requirements must be post-load verified and their first
# failure must be merged into RC notification results.
reload_modules = compact(body(NET_SOURCE, "reload_nat_modules(void)"))
assert "rc_module_load_requirements_apply(pptp_modules, 2, &module_ops)" in reload_modules
assert "rc_module_load_requirements_apply(h323_modules, 2, &module_ops)" in reload_modules
assert "rc_module_load_requirements_apply(sip_modules, 2, &module_ops)" in reload_modules
assert "rc_module_load_requirements_apply(ftp_modules, 2, &module_ops)" in reload_modules
assert "Cannot read active nf_conntrack_ftp ports" in reload_modules
assert "still referenced" in reload_modules
assert "return reload_status;" in reload_modules
assert "int reload_nat_modules(void);" in HEADER
assert "".join(RC_SOURCE.split()).count("rc_notify_merge_action_result(action_result,reload_nat_modules())") == 2
assert RC_SOURCE.count("reload_nat_modules();") == 0

# WAN startup must retain NAT helper/accelerator errors without aborting the
# independent WAN path: all configured interfaces and services are still tried.
wan_start = compact(body(WAN_SOURCE, "start_wan(void)"))
nat_reload_call = "vlan_result = reload_nat_modules();"
assert nat_reload_call in wan_start
nat_reload_pos = wan_start.index(nat_reload_call)
nat_failure_pos = wan_start.index("if (vlan_result != 0)", nat_reload_pos)
nat_failure_end = wan_start.index("refresh_resolvconf_nonfatal(", nat_failure_pos)
nat_failure = wan_start[nat_failure_pos:nat_failure_end]
assert re.search(
    r"wan_start_status\s*=\s*rc_first_service_start_status\s*\(\s*wan_start_status\s*,\s*vlan_result\s*\)\s*;",
    nat_failure,
)
assert "continuing WAN startup" in nat_failure
assert not re.search(r"\b(return|continue)\b", nat_failure)
assert nat_reload_pos < wan_start.index("refresh_resolvconf_nonfatal(")
print("Configured NAT helper/accelerator errors reach WAN status without blocking WAN startup: PASS")

# Core firewall rebuilding must report validation, transaction and cleanup
# failures; restart must still attempt UPnP refresh and preserve the first error.
firewall_rebuild = compact(body(FIREWALL_SOURCE, "start_firewall_ex_locked(void)"))
assert "firewall_restore_transaction_prepare_reset(" in firewall_rebuild
assert "firewall_restore_transaction_status(&firewall_core_restore_transaction)" in firewall_rebuild
assert "firewall_restore_transaction_finish(" in firewall_rebuild
assert "firewall_restore_transaction_abort(" in firewall_rebuild
assert "return EBUSY;" in firewall_rebuild
assert "return EINVAL;" in firewall_rebuild
assert "return EIO;" in firewall_rebuild
assert "FIREWALL_ADBYBY_RESTORE_RECOVERY_FAILED" in firewall_rebuild
assert "adbyby_restore_result != FIREWALL_ADBYBY_RESTORE_OK" in firewall_rebuild
forwarding_disable = compact(body(FIREWALL_SOURCE, "firewall_disable_forwarding("))
assert "next_status = set_ipv4_forward(0);" in forwarding_disable
assert 'next_status = set_interface_conf_int("ipv6", "all", "forwarding", 0);' in forwarding_disable
assert forwarding_disable.count("rc_first_service_start_status") == 2
assert 'forward_status = firewall_disable_forwarding("Core firewall rollback");' in firewall_rebuild
assert 'forward_status = firewall_disable_forwarding("Adbyby firewall rollback");' in firewall_rebuild
assert not re.search(r"\breturn\s*;", firewall_rebuild)
firewall_entry = compact(body(FIREWALL_SOURCE, "start_firewall_ex(void)"))
assert "firewall_lifecycle_lock_acquire()" in firewall_entry
assert "status = start_firewall_ex_locked();" in firewall_entry
assert "firewall_lifecycle_lock_release(lock_fd)" in firewall_entry
assert firewall_entry.index("firewall_lifecycle_lock_acquire()") < firewall_entry.index("status = start_firewall_ex_locked();") < firewall_entry.index("firewall_lifecycle_lock_release(lock_fd)")
assert "if (lock_fd < 0)" in firewall_entry
assert "errno = saved_errno;" in firewall_entry
assert "int start_firewall_ex(void);" in HEADER

firewall_restart = compact(body(NET_SOURCE, "restart_firewall(void)"))
assert "firewall_status = start_firewall_ex();" in firewall_restart
assert "saved_errno = errno ? errno : firewall_status;" in firewall_restart
assert "upnp_status = update_upnp();" in firewall_restart
assert firewall_restart.index("firewall_status = start_firewall_ex();") < firewall_restart.index("upnp_status = update_upnp();")
assert "return rc_first_service_start_status(firewall_status, upnp_status);" in firewall_restart
assert "errno = saved_errno;" in firewall_restart
print("Core firewall restore, rollback, cleanup and UPnP first-error propagation: PASS")

retained_log = compact(body(FIREWALL_SOURCE, "firewall_log_retained_restore_snapshots("))
assert "tx->cleanup_path_count" in retained_log
assert "tx->cleanup_paths[i]" in retained_log
assert "tx->snapshots[i].path" in retained_log
assert "firewall_core_restore_transaction.cleanup_failed" in firewall_rebuild
assert "partial cleanup paths retained" in firewall_rebuild
print("Firewall recovery and partial-cleanup path diagnostics: PASS")

hook_remove = compact(body(FIREWALL_HOOK_SOURCE, "firewall_hook_file_remove_with("))
assert "unlink_fn(staged_path)" in hook_remove
assert "errno == ENOENT" in hook_remove
assert "return -1;" in hook_remove
assert "staged_path[0] = '\\0';" in hook_remove
assert "test_unlink_once" in FIREWALL_HOOK_TEST
assert "firewall_hook_file_remove_with(staged,test_unlink_once)==-1" in FIREWALL_HOOK_TEST
command_errno = compact(body(FIREWALL_COMMAND_STATUS_SOURCE, "firewall_command_status_to_errno("))
assert "return EIO;" in command_errno
assert "return system_errno;" in command_errno
assert "firewall_command_status_to_errno(-1, EACCES) == EACCES" in FIREWALL_COMMAND_STATUS_TEST
assert '#include "service_start_status.h"' in FIREWALL_SOURCE
hook_dispatch = compact(body(FIREWALL_SOURCE, "start_firewall_ex_locked(void)"))
assert len(re.findall(r"status\s*=\s*rc_first_service_start_status\(status,\s*hook_error\);", hook_dispatch)) >= 9
for hook in ("Shadowsocks firewall hook", "Persistent post-firewall hook", "Optware iptables update hook"):
    assert hook_dispatch.count(hook) >= 3
assert re.search(r"errno\s*!=\s*ENOENT", hook_dispatch)
assert "firewall_log_hook_cleanup_failure" in hook_dispatch
print("Optional firewall hook failure and staged-file cleanup propagation: PASS")
