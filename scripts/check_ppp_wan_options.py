#!/usr/bin/env python3
"""Verify PPP option NVRAM reads fail closed before session/config side effects."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
rc_dir = ROOT / "trunk/user/rc/src"
net_ppp = (rc_dir / "net_ppp.c").read_text()
net_wan = (rc_dir / "net_wan.c").read_text()
rc_header = (rc_dir / "rc.h").read_text()
policy = (rc_dir / "ppp_wan_option_policy.h").read_text()
test = (rc_dir / "tests/ppp_wan_option_policy.c").read_text()
control_dir = ROOT / "trunk/user/rp-l2tp/rp-l2tp-0.5-rtn/handlers"
control_source = (control_dir / "cmd-control.c").read_text()
control_server_source = (control_dir / "cmd.c").read_text()
control_request_policy = (control_dir / "cmd-request-policy.h").read_text()
control_request_test = (control_dir / "tests/cmd-request-policy.c").read_text()
control_policy = (control_dir / "cmd-control-status.h").read_text()
control_test = (control_dir / "tests/cmd-control-status.c").read_text()
workflow = (ROOT / ".github/workflows/wan-route-status.yml").read_text()


def body(source, signature):
    start = source.find(signature)
    if start < 0:
        raise AssertionError("missing function: " + signature)
    opening = source.find("{", start + len(signature))
    if opening < 0:
        raise AssertionError("missing function body: " + signature)
    depth, index = 0, opening
    while index < len(source):
        if source.startswith("//", index):
            end = source.find(chr(10), index + 2)
            index = len(source) if end < 0 else end + 1
            continue
        if source.startswith("/*", index):
            end = source.find("*/", index + 2)
            index = len(source) if end < 0 else end + 2
            continue
        ch = source[index]
        if ch in ('"', "'"):
            quote = ch
            index += 1
            while index < len(source):
                if source[index] == "\\":
                    index += 2
                elif source[index] == quote:
                    index += 1
                    break
                else:
                    index += 1
            continue
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:index]
        index += 1
    raise AssertionError("unterminated function: " + signature)


def require(value, message):
    if not value:
        raise AssertionError(message)


checked = body(net_wan,
    "get_wan_unit_value_int_status(int unit, const char *param_name, int *value)")
require("nvram_get_checked(wanN_param, &stored_value)" in checked,
        "WAN PPP integer reader must use checked NVRAM access")
require("rc_wan_nvram_read_int_status(api_result, saved_errno" in checked,
        "WAN PPP integer parser must propagate read/parse failures")
require("int get_wan_unit_value_int_status" in rc_header,
        "checked integer accessor prototype missing")
require(not __import__("re").search(r"static\s+int\s+get_wan_unit_value_int_status", net_wan),
        "checked integer accessor must be link-visible to net_ppp.c")

reader = body(net_ppp,
    "ppp_wan_option_read_int(void *context, const char *param_name, int *value)")
require("get_wan_unit_value_int_status(*unit, param_name, value)" in reader and
        "errno = status" in reader, "PPP read callback must preserve status")

copy_reader = body(net_wan,
    "get_wan_unit_value_copy_status(int unit, const char *param_name,")
require("rc_wan_nvram_copy_value(wan_nvram_checked_get_callback, NULL" in copy_reader,
        "WAN string accessor must use checked NVRAM copies")
require("int get_wan_unit_value_copy_status" in rc_header,
        "checked WAN string accessor prototype missing")
text_reader = body(net_ppp,
    "ppp_wan_option_read_text(void *context, const char *param_name,")
require("get_wan_unit_value_copy_status(*unit, param_name" in text_reader and
        "errno = status" in text_reader, "PPP string callback must preserve status")
l2tp_daemon = body(net_ppp, "safe_start_xl2tpd_locked(void)")
l2tp_proto_read = "get_wan_unit_proto_checked_status(unit, &wan_proto)"
l2tp_error_gate = l2tp_daemon.find("if (wan_value_status != 0)")
l2tp_error_return = l2tp_daemon.find("return -1;", l2tp_error_gate)
l2tp_config_open = l2tp_daemon.find('fopen(l2tp_conf, "w")')
require(l2tp_proto_read in l2tp_daemon and
        "int unit, wan_proto;" in l2tp_daemon and
        l2tp_error_gate > l2tp_daemon.index(l2tp_proto_read) and
        l2tp_error_gate < l2tp_error_return < l2tp_config_open and
        "errno = saved_errno;" in l2tp_daemon[l2tp_error_gate:l2tp_error_return] and
        "get_wan_proto(unit)" not in l2tp_daemon,
        "xl2tpd must preserve protocol read errors before replacing its configuration")
require("int get_wan_unit_proto_checked_status(int unit, int *wan_proto);" in rc_header,
        "checked WAN protocol reader must be exported for PPP consumers")
peer_copy = body(net_ppp,
    "get_wan_ppp_peer_copy(int unit, char *peer, size_t peer_size)")
require('get_wan_unit_value_copy_status(unit, "ppp_peer"' in peer_copy and
        'get_wan_unit_value_copy_status(unit, "man_gateway"' in peer_copy,
        "PPP peer fallback must use checked local copies")

launch = body(net_ppp, "launch_wan_pppd_locked(int unit, int wan_proto,")
require("int auth_type, mtu, mru, mtu_max, mtu_def, mru_def, mru_max;" in launch,
        "PPP MTU default local must remain declared for all protocol branches")
preflight = launch.index("ppp_wan_option_values_read(")
text_preflight = launch.index("ppp_wan_option_text_values_read(")
generation = launch.index("ppp_wan_session_generation_change(&callback_generation)")
options_open = launch.index("ppp_options_file_open(options)")
stop = launch.find("kill_services(svcs, 5, 1)")
require(preflight < generation and text_preflight < generation < options_open,
        "PPP integer and string options must be checked before session changes and file open")
require(stop < 0 or (preflight < stop and text_preflight < stop),
        "L2TP daemon stop must follow checked option reads")
require("get_wan_unit_value_int(" not in launch and "get_wan_unit_value(" not in launch,
        "PPP builder still consumes unchecked NVRAM options")
for field in ("username", "password", "peer", "access_concentrator", "service", "user_options"):
    require("text_values." + field in launch, "PPP writer omits checked string option " + field)
for field in ("auth_type","mtu","mru","pppoe_idletime","pppoe_demand",
              "pppoe_txonly_x","dnsenable_x","mppe_mode","lcp_echo","adaptive_lcp"):
    require("option_values." + field in launch, "unchecked PPP option: " + field)


launch_mode_snapshot = body(net_ppp,
    "ppp_wan_launch_mode_snapshot_read(int wan_proto,")
require('"pppoemwan_enable"' in launch_mode_snapshot and
        "rc_wan_nvram_copy_snapshot(" in launch_mode_snapshot and
        "ppp_wan_launch_mode_resolve(" in launch_mode_snapshot,
        "PPP launch selectors must come from one checked NVRAM snapshot")
require('if (wan_proto == IPV4_WAN_PROTO_L2TP)' in launch_mode_snapshot and
        '"wan_l2tpd"' in launch_mode_snapshot and
        "field_count = 2" in launch_mode_snapshot,
        "RPL2TP selector must be read only for L2TP builds/protocols")
for signature, operation in (
    ("ppp_wan_launch_snapshot_transaction_begin(void *context)", "nvram_transaction_begin("),
    ("ppp_wan_launch_snapshot_transaction_get(void *context,", "nvram_transaction_get("),
    ("ppp_wan_launch_snapshot_transaction_end(void *context)", "nvram_transaction_end("),
):
    adapter = body(net_ppp, signature)
    require(operation in adapter, "PPP launch snapshot adapter missing " + operation)

mode_policy = body(policy, "ppp_wan_launch_mode_resolve(")
require('strcmp(multiwan_value, "1") == 0' in mode_policy and
        'strcmp(wan_l2tpd_value, "1") == 0' in mode_policy and
        "read_status != 0" in mode_policy,
        "PPP launch mode must preserve exact match defaults and fail on read errors")
require("launch_mode->multiwan_enabled" in launch and
        "launch_mode->use_rpl2tp" in launch and
        'nvram_match("pppoemwan_enable"' not in launch and
        'nvram_match("wan_l2tpd"' not in launch,
        "PPP lifecycle decisions must reuse the checked immutable launch snapshot")
wrapper = body(net_ppp, "launch_wan_pppd(int unit, int wan_proto)")
unit_check = wrapper.index("ppp_wan_launch_unit_status(unit)")
unit_error = wrapper.index("if (mode_status != 0)", unit_check)
unit_errno = wrapper.index("errno = mode_status;", unit_error)
snapshot_read = wrapper.index("ppp_wan_launch_mode_snapshot_read(wan_proto, &launch_mode)")
require(unit_check < unit_error < unit_errno < snapshot_read and
        "ppp_wan_launch_unit_status(int unit)" in policy,
        "invalid WAN unit must report EINVAL before any NVRAM snapshot")
snapshot_error = wrapper.index("if (mode_status != 0)", snapshot_read)
lock_acquire = wrapper.index("vpn_server_lifecycle_lock_acquire()")
require(snapshot_read < snapshot_error < lock_acquire and
        "launch_wan_pppd_locked(unit, wan_proto, &launch_mode)" in wrapper and
        "!launch_mode.multiwan_enabled" in wrapper,
        "PPP launch snapshot error must abort before lock/session effects and drive lock selection")
require("launch_mode->multiwan_enabled" in launch and launch.count("launch_mode->multiwan_enabled") >= 3,
        "WAN PPP multi-WAN stop/start/rollback must share one selector snapshot")

policy_body = body(policy, "ppp_wan_option_values_read(int is_pppoe")
for token in ('PPP_WAN_READ_OPTION("ppp_auth", auth_type)',
              'PPP_WAN_READ_OPTION("dnsenable_x", dnsenable_x)',
              'PPP_WAN_READ_OPTION("ppp_mppe", mppe_mode)',
              'PPP_WAN_READ_OPTION("ppp_lcp", lcp_echo)',
              'PPP_WAN_READ_OPTION("ppp_alcp", adaptive_lcp)',
              "memset(values, 0, sizeof(*values))"):
    require(token in policy_body, "PPP option policy missing " + token)

ipup_reader = body(net_ppp,
    "ppp_wan_ipup_get_nvram_value(void *opaque, const char *param_name,")
require("nvram_transaction_get(context->transaction, wan_param, value)" in ipup_reader,
        "PPP ip-up prior-state read must use the active NVRAM transaction")
require('snprintf(wan_param, sizeof(wan_param), "wan%d_%s"' in ipup_reader,
        "PPP ip-up checked getter must resolve unit-scoped keys")

ipup_publish = body(net_ppp,
    "ppp_wan_ipup_publish_address(void *opaque, const char *ipaddr,")
begin = ipup_publish.index("nvram_transaction_begin(&transaction)")
snapshot = ipup_publish.index("rc_wan_nvram_copy_value(")
publish = ipup_publish.index("dhcp_lease_nvram_publish(")
end_transaction = ipup_publish.index("nvram_transaction_end(&transaction)")
require(begin < snapshot < publish < end_transaction,
        "PPP ip-up must read and publish its prior/current tuple under one transaction")
require("get_wan_unit_value(" not in ipup_publish,
        "PPP ip-up prior tuple must not use cache-backed unchecked getters")
require("publish_attempted = 1" in ipup_publish and
        ipup_publish.index("publish_attempted = 1") < publish,
        "PPP ip-up recovery must know whether any writes were attempted")
require("if (!publish_attempted)" in ipup_publish,
        "a failed preflight read must not trigger an unnecessary restore transaction")

routes = body(net_ppp, "preset_wan_ppp_routes(char *ppp_ifname, int unit)")
primary = routes.index('get_wan_unit_value_int_status(unit, "primary", &primary)')
default_route = routes.index('route_add(ppp_ifname, 1, "0.0.0.0"')
static_routes = routes.index("add_static_wan_routes(ppp_ifname)")
require(primary < default_route < static_routes,
        "PPP default route must use checked primary value and still restore static routes")
require("status = route_status_accumulate(status, route_status)" in routes,
        "PPP primary read error must be reported")

text_policy = body(policy, "ppp_wan_option_text_values_read(int is_pppoe, int is_pptp")
require("(is_pppoe && is_pptp)" in text_policy,
        "PPP text snapshot must reject conflicting PPPoE/PPTP flags")
peer_gate = text_policy.index("if (is_pptp)")
peer_read = text_policy.index('PPP_WAN_READ_TEXT("ppp_peer", peer)', peer_gate)
pppoe_gate = text_policy.index("if (is_pppoe)")
pppoe_options = text_policy.index('PPP_WAN_READ_TEXT("pppoe_ac", access_concentrator)', pppoe_gate)
require(peer_gate < peer_read < pppoe_gate < pppoe_options,
        "peer and PPPoE-only text keys must be protocol-scoped")

for name in ("test_pppoe_options","test_pptp_and_l2tp_options",
             "test_missing_values_keep_zero_defaults",
             "test_failures_stop_and_clear_snapshot",
             "test_invalid_arguments_fail_closed",
             "test_pppoe_string_snapshot_reads_only_pppoe_fields",
             "test_l2tp_string_snapshot_skips_inactive_peer_and_pppoe_fields",
             "test_empty_peer_uses_checked_management_gateway_fallback",
             "test_text_backend_failure_discards_partial_values",
             "test_empty_text_defaults_and_oversize_failure",
             "test_text_snapshot_rejects_invalid_protocol_flags",
             "test_launch_unit_status_reports_invalid_index",
             "test_launch_mode_exact_nvram_match_compatibility",
             "test_launch_mode_protocol_and_build_gates",
             "test_launch_mode_snapshot_errors_fail_closed",
             "test_launch_mode_invalid_arguments_fail_closed"):
    require(name in test, "missing PPP option regression: " + name)

push, pr = workflow.split("  pull_request:", 1)
for path in ("trunk/user/rc/src/ppp_wan_option_policy.h",
             "trunk/user/rc/src/tests/ppp_wan_option_policy.c",
             "scripts/check_ppp_wan_options.py"):
    require(path in push and path in pr, "WAN workflow path filter misses " + path)
require("Compile and run PPP WAN option checked-read preflight regression" in workflow,
        "missing PPP policy compile/run step")
require("Check PPP option reads fail closed before session changes" in workflow,
        "missing PPP source contract step")

require('#include "cmd-control-status.h"' in control_source and
        "cmd_control_response_status(" in control_source and
        "response_prefix_length" in control_source,
        "l2tp-control must validate the daemon response before reporting command success")
require("SO_SNDTIMEO" in control_source and "SO_RCVTIMEO" in control_source and
        "CONTROL_IO_TIMEOUT_SECONDS" in control_source,
        "l2tp-control socket reads and writes must be bounded")
require("write_all(fd, cmd, strlen(cmd))" in control_source and
        'write_all(fd, "\\n", 1)' in control_source and
        "writev(" not in control_source,
        "l2tp-control command delivery must handle partial writes")
require("return EIO;" in control_policy and "return EPROTO;" in control_policy and
        "response[0] == 'O' && response[1] == 'K'" in control_policy and
        "response[0] == 'E'" in control_policy,
        "l2tp-control response parser must accept OK only and reject ERR/malformed replies")

require(control_source.index(r'write_all(STDOUT_FILENO, "\n", 1)') <
        control_source.index("response_status = cmd_control_response_status("),
        "l2tp-control must preserve its terminating newline for ERR replies")
server_start = body(control_server_source, "process_option(EventSelector *es,")
chmod_gate = server_start.find("if (chmod(sockname, 0600) < 0)")
listen_gate = server_start.find("if (listen(fd, 5) < 0)")
require(chmod_gate >= 0 and listen_gate > chmod_gate,
        "L2TP control socket permissions must be enforced before listen")
chmod_failure = server_start[chmod_gate:listen_gate]
require("l2tp_set_errmsg" in chmod_failure and "close(fd);" in chmod_failure and
        "(void) remove(sockname);" in chmod_failure and "return -1;" in chmod_failure and
        "errno = saved_errno;" in chmod_failure and
        "(void) chmod(sockname, 0600)" not in server_start,
        "L2TP command socket must fail closed and clean up when chmod fails")
for case in ("test_success_responses", "test_error_responses",
             "test_malformed_and_truncated_responses"):
    require(case in control_test and case + "()" in control_test,
            "missing l2tp-control response regression: " + case)
for path in ("trunk/user/rp-l2tp/rp-l2tp-0.5-rtn/handlers/cmd-control.c",
             "trunk/user/rp-l2tp/rp-l2tp-0.5-rtn/handlers/cmd-control-status.h",
             "trunk/user/rp-l2tp/rp-l2tp-0.5-rtn/handlers/tests/cmd-control-status.c"):
    require(path in push and path in pr,
            "WAN route workflow path filter misses " + path)
require("Compile and run rp-l2tp control response status regression" in workflow,
        "missing l2tp-control response host regression compile/run step")
require('#include "cmd-request-policy.h"' in control_server_source and
        "CMD_CONTROL_REQUEST_BUFFER_SIZE 512" in control_server_source,
        "L2TP control server must tie its request bound to the allocated buffer size")
require("EventTcp_ReadBuf(es, fd, CMD_CONTROL_REQUEST_BUFFER_SIZE" in control_server_source,
        "L2TP control server must use its explicit request buffer capacity")
control_handler = body(control_server_source,
    "static void\ncmd_handler(EventSelector *es,")
require("cmd_control_request_is_valid(" in control_handler and
        "flag == EVENT_TCP_FLAG_COMPLETE" in control_handler and
        "buf[len] = '\\0';" in control_handler and
        "buf[len - 1] = '\\0';" in control_handler,
        "L2TP control handler must validate framing before safe NUL termination")
require("buf[len+1]" not in control_handler and
        "len < CMD_CONTROL_REQUEST_BUFFER_SIZE" in control_handler,
        "L2TP control handler must not read/write beyond the fixed receive buffer")
require("static inline int\ncmd_control_request_is_valid(" in control_request_policy and
        "length < capacity" in control_request_policy and
        "has_newline" in control_request_policy,
        "L2TP request policy must reject over-capacity and unterminated input")
for case in ("test_valid_delimited_requests",
             "test_incomplete_requests_are_rejected",
             "test_empty_and_capacity_sized_requests_are_rejected"):
    require(case in control_request_test and case + "()" in control_request_test,
            "missing L2TP request framing regression: " + case)
for path in ("trunk/user/rp-l2tp/rp-l2tp-0.5-rtn/handlers/cmd-request-policy.h",
             "trunk/user/rp-l2tp/rp-l2tp-0.5-rtn/handlers/tests/cmd-request-policy.c"):
    require(path in push and path in pr,
            "WAN route workflow path filter misses " + path)
require("Compile and run rp-l2tp control request framing regression" in workflow,
        "missing l2tp-control request framing host regression compile/run step")
print("PPP NVRAM config and rp-l2tp command status contracts: PASS")
