#!/usr/bin/env python3
"""Verify initial physical WAN MTU failures reach start_wan() status."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
WAN = (RC / "net_wan.c").read_text()
POLICY = (RC / "wan_mtu_setup_policy.h").read_text()
TEST = (RC / "tests/service_start_status.c").read_text()
DHCP_TEST = (RC / "tests/dhcp_wan_interface_policy.c").read_text()
WAN_WORKFLOW = (ROOT / ".github/workflows/wan-route-status.yml").read_text()
PROCESS_WORKFLOW = (ROOT / ".github/workflows/process-lifecycle-regression.yml").read_text()


def body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    quote = ""
    index = opening
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "*":
                state = "block"
                index += 2
                continue
            if char == "/" and nxt == "/":
                state = "line"
                index += 2
                continue
            if char in ("'", '"'):
                quote = char
                state = "quote"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[opening + 1:index]
        elif state == "quote":
            if char == chr(92):
                index += 2
                continue
            if char == quote:
                state = "code"
        elif state == "line" and char == chr(10):
            state = "code"
        elif state == "block" and char == "*" and nxt == "/":
            state = "code"
            index += 2
            continue
        index += 1
    raise AssertionError("unterminated function: " + signature)


def require(condition, message):
    if not condition:
        raise SystemExit("WAN initial MTU status contract: FAIL: " + message)


def code_characters(text, start):
    """Yield C code characters while ignoring comments and literals."""
    position = start
    while position < len(text):
        if text.startswith("//", position):
            newline = text.find(chr(10), position + 2)
            position = len(text) if newline < 0 else newline + 1
            continue
        if text.startswith("/*", position):
            close = text.find("*/", position + 2)
            position = len(text) if close < 0 else close + 2
            continue
        character = text[position]
        if character in ('"', "'"):
            quote = character
            position += 1
            while position < len(text):
                if text[position] == chr(92):
                    position += 2
                elif text[position] == quote:
                    position += 1
                    break
                else:
                    position += 1
            continue
        yield position, character
        position += 1


def body_at_column_zero_terminator(text, signature):
    """Extract C bodies despite mutually exclusive preprocessor brace branches."""
    search_from = 0
    opening = -1
    while True:
        start = text.find(signature, search_from)
        if start < 0:
            break
        for position, character in code_characters(text, start + len(signature)):
            if character == ";":
                break
            if character == "{":
                opening = position
                break
        if opening >= 0:
            break
        search_from = start + len(signature)
    if opening < 0:
        raise AssertionError("missing function body: " + signature)

    for position, character in code_characters(text, opening):
        if character != "}":
            continue
        line_start = text.rfind(chr(10), 0, position) + 1
        if line_start == position:
            return text[opening + 1:position]
    raise AssertionError("unterminated function: " + signature)


_conditional_probe = r'''
int conditional_probe(int value) {
#if defined(LEFT_BRANCH)
    if (value) {
#else
    if (!value) {
#endif
        value++;
    }
    return value;
}
'''
_conditional_body = body_at_column_zero_terminator(
    _conditional_probe, "conditional_probe(int value)"
)
for marker in ("if (value)", "if (!value)", "return value;"):
    require(marker in _conditional_body,
            "preprocessor-aware function extraction regression: " + marker)


start_wan = body_at_column_zero_terminator(WAN, "start_wan(void)")
require("int unit, wan_proto, wan_mtu;" in start_wan,
        "configured WAN MTU must be available to both initial and static setup")
proto = start_wan.index("wan_proto = get_wan_proto(unit);")
read_mtu = start_wan.index("get_wan_unit_value_int_status(", proto)
read_mtu_argument = start_wan.index('unit, "mtu", &wan_mtu);', read_mtu)
read_failure = start_wan.index("if (nvram_status != 0)", read_mtu_argument)
read_failure_merge = start_wan.index(
    "wan_start_status = rc_first_service_start_status(", read_failure
)
read_failure_log = start_wan.index("cannot read configured MTU", read_failure_merge)
read_abort = start_wan.index("continue;", read_failure_log)
prepare = start_wan.index("if (wan_default_mtu_should_be_set(", read_abort)
default_set = start_wan.index("mtu_status = set_interface_mtu(wan_ifname, 1500);", prepare)
failure = start_wan.index("if (mtu_status != 0)", default_set)
aggregate = start_wan.index(
    "wan_start_status = rc_first_service_start_status(", failure
)
diagnostic = start_wan.index("WAN interface %s default MTU setup incomplete", aggregate)
ifup = start_wan.index(
    'interface_status = ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);',
    diagnostic,
)
require(
    proto < read_mtu < read_mtu_argument < read_failure < read_failure_merge
    < read_failure_log < read_abort < prepare < default_set < failure
    < aggregate < diagnostic < ifup,
    "checked MTU read must abort only the affected unit on error before default setup; "
    "default write errors are aggregated/logged before interface bring-up",
)
assert 'get_wan_unit_value_int(unit, "mtu")' not in start_wan
default_block = start_wan[prepare:ifup]
require(
    "wan_proto == IPV4_WAN_PROTO_IPOE_STATIC, wan_mtu" in default_block,
    "default MTU suppression must apply only to static IPoE configured MTU",
)
require("continue;" not in default_block,
        "default MTU failure must not skip required physical interface bring-up")

static_branch = start_wan.index(
    "if (wan_proto == IPV4_WAN_PROTO_IPOE_STATIC)", ifup
)
static_end = start_wan.index(
    "/* Rebuild required firewall policy before EAPOL, DHCP or route activation. */", static_branch
)
static = start_wan[static_branch:static_end]
custom_set = static.index("mtu_status = set_interface_mtu(wan_ifname, wan_mtu);")
custom_failure = static.index("wan_start_status = rc_first_service_start_status(", custom_set)
address_gate = static.index("if (!wan_err)", custom_failure)
address_set = static.index(
    "address_status = ifconfig(wan_ifname, IFUP, wan_addr, wan_mask);",
    address_gate,
)
require(
    "if (wan_mtu >= 1300 && wan_mtu < 1500)" in static
    and custom_set < custom_failure < address_gate < address_set,
    "static custom MTU must still be applied and its failure must block address publication",
)
require("int wan_mtu =" not in static,
        "static path must use the same configured MTU value as the initial decision")
require(
    "return wan_start_status;" in start_wan,
    "start_wan must return the accumulated physical MTU failure",
)

policy_body = body(POLICY, "wan_default_mtu_should_be_set(")
require(
    "is_static_ipoe && configured_mtu >= 1300 && configured_mtu < 1500" in policy_body,
    "only configured static-IPoE MTUs from 1300 through 1499 suppress default MTU",
)
for token in (
    "wan_default_mtu_should_be_set(1, 1299)",
    "wan_default_mtu_should_be_set(1, 1300)",
    "wan_default_mtu_should_be_set(1, 1499)",
    "wan_default_mtu_should_be_set(1, 1500)",
    "wan_default_mtu_should_be_set(0, 1400)",
    "physical_ifup_attempts == 1",
    "protocol_start_attempts == 1",
    "assert(status == -1)",
):
    require(token in TEST, "service startup regression missing " + token)
require(
    "test_wan_default_mtu_respects_static_custom_override();" in TEST
    and "test_wan_default_mtu_failure_is_reported_while_startup_continues();" in TEST,
    "both WAN MTU policy regressions must run from main()",
)

for path in (
    "scripts/check_wan_initial_mtu_status.py",
    "trunk/user/rc/src/wan_mtu_setup_policy.h",
    "trunk/user/rc/src/tests/service_start_status.c",
):
    require(
        WAN_WORKFLOW.count('"' + path + '"') >= 2,
        "WAN push and pull_request triggers must include " + path,
    )
require(
    "python3 scripts/check_wan_initial_mtu_status.py" in WAN_WORKFLOW,
    "WAN regression job must run the initial MTU contract checker",
)
for path in (
    "trunk/user/rc/src/wan_mtu_setup_policy.h",
    "trunk/user/rc/src/tests/service_start_status.c",
):
    require(
        PROCESS_WORKFLOW.count('"' + path + '"') >= 2,
        "Process push and pull_request triggers must include " + path,
    )
require(
    "Compile and run RC boot service status aggregation regression" in PROCESS_WORKFLOW
    and 'trunk/user/rc/src/tests/service_start_status.c' in PROCESS_WORKFLOW,
    "Process CI must compile and run the regression with the new policy header",
)


# Static MAN/physical WAN custom MTU setup is part of launch_wanx()'s result.
launch_wanx = body(WAN, "launch_wanx(char *man_ifname, int unit, int wait_dhcpc, int use_zcip)")
man_static = launch_wanx.index("if (man_mtu >= 1300 && man_mtu < 1500)")
man_set = launch_wanx.index("mtu_status = set_interface_mtu(man_ifname, man_mtu);", man_static)
man_guard = launch_wanx.index("if (mtu_status != 0)", man_set)
man_merge = launch_wanx.index(
    "launch_status = route_status_accumulate(", man_guard
)
man_merge_arg = launch_wanx.index("launch_status, mtu_status", man_merge)
man_log = launch_wanx.index(
    "MAN interface %s MTU setup incomplete (status=%d)", man_merge_arg
)
man_address = launch_wanx.index(
    "address_status = ifconfig(man_ifname, IFUP, man_addr, man_mask);", man_log
)
man_route = launch_wanx.index(
    "route_status = man_up_internal(man_ifname, unit, 1, &gateway_route_status);",
    man_address,
)
require(
    man_static < man_set < man_guard < man_merge < man_merge_arg < man_log < man_address < man_route,
    "MAN custom MTU failure must be accumulated/logged before address and route setup",
)
require(
    "return launch_status;" in launch_wanx,
    "launch_wanx must return the accumulated MAN MTU setup result",
)
require("test_man_custom_mtu_failure_is_reported_and_setup_continues();" in TEST,
        "MAN custom MTU first-error regression must run from main()")
for token in (
    "address_attempts == 1",
    "route_attempts == 1",
    "assert(status == -1)",
):
    require(token in TEST, "MAN MTU failure-continuation coverage missing " + token)


# DHCP-advertised MTU is optional: reapply changes on renewal, but keep
# ioctl failure diagnostic-only. Only actual route errors drive route_err retry.
wan_bound = body(WAN, "udhcpc_bound(char *wan_ifname, int is_renew_mode)")
require("mtu_status = 0;" in wan_bound,
        "WAN DHCP MTU status must be initialized when the option is absent")
wan_mtu_condition = wan_bound.index("if (wan_dhcp_mtu_is_supported(dhcp_mtu))")
wan_mtu_set = wan_bound.index(
    "mtu_status = set_interface_mtu(wan_ifname, dhcp_mtu);", wan_mtu_condition
)
wan_mtu_log = wan_bound.index("MTU setup incomplete (status=%d)", wan_mtu_set)
wan_route_call = wan_bound.index(
    "route_setup_status = wan_up_internal(wan_ifname, unit, 0);", wan_mtu_log
)
wan_route_err = wan_bound.index(
    'wan_nvram_int_temp_status(\n\t\tstrcat_r(prefix, "route_err", tmp), route_setup_status);',
    wan_route_call,
)
wan_return = wan_bound.index("return callback_status;", wan_route_err)
require(
    'wan_nvram_int_temp_status(\n\t\tstrcat_r(prefix, "err", tmp), i_err);' in wan_bound,
    "WAN DHCP lease error marker must use checked NVRAM publication",
)
require(
    "callback_status = route_status_accumulate(callback_status, publish_status);" in wan_bound,
    "WAN DHCP callback must preserve the first NVRAM publication failure",
)
require(
    wan_mtu_condition < wan_mtu_set < wan_mtu_log < wan_route_call
    < wan_route_err < wan_return,
    "WAN DHCP MTU remains diagnostic-only before independent route status publication",
)
require("mtu_status, route_setup_status" not in wan_bound,
        "optional WAN MTU failure must not be merged into route retry status")
require(
    "!is_renew_mode" not in wan_bound[wan_mtu_condition - 80:wan_mtu_set],
    "changed DHCP MTU must also be applied during renew callbacks",
)

wan_renew = body(WAN, "udhcpc_renew(char *wan_ifname)")
wan_renew_compact = "".join(wan_renew.split())
for key, target in (
    ("err", "lease_error"),
    ("route_err", "route_error"),
    ("mtu_err", "mtu_error"),
):
    require(
        'get_nvram_int_checked_status(strcat_r(prefix,"' + key + '",tmp),&' +
        target + ');' in wan_renew_compact,
        "WAN renewal must check previous DHCP retry state: " + key,
    )
wan_renew_retry = wan_renew_compact.index("dhcp_wan_bound_retry_decide(")
wan_retry_error = wan_renew_compact.index("if(nvram_status!=0)", wan_renew_retry)
wan_retry_gate = wan_renew_compact.index("if(retry_required)", wan_retry_error)
wan_retry_bound = wan_renew_compact.index("returnudhcpc_bound(wan_ifname,1);", wan_retry_gate)
wan_renew_mtu = wan_renew_compact.index('value=getenv("mtu");', wan_retry_bound)
wan_mtu_change_bound = wan_renew_compact.index(
    "returnudhcpc_bound(wan_ifname,1);", wan_renew_mtu
)
require(
    wan_renew_retry < wan_retry_error < wan_retry_gate < wan_retry_bound <
    wan_renew_mtu < wan_mtu_change_bound,
    "WAN renewal must fail closed on retry-state read errors, retry incomplete lease state, and re-enter bound after DHCP MTU changes",
)
require("cannot verify previous DHCP retry state" in wan_renew and
        "get_nvram_int_checked_status(strcat_r(prefix,\"mtu\",tmp),&stored_mtu);" in wan_renew_compact,
        "WAN renewal must use checked stored-MTU reads")
assert "nvram_get_int(" not in wan_renew
assert "dhcp_wan_bound_retry_required(" not in wan_renew

viptv_bound = body(WAN, "udhcpc_viptv_bound(char *man_ifname, int is_renew_mode)")
require("mtu_status = 0;" in viptv_bound,
        "IPTV DHCP MTU status must be initialized when the option is absent")
tv_mtu_condition = viptv_bound.index("if (wan_dhcp_mtu_is_supported(dhcp_mtu))")
tv_mtu_set = viptv_bound.index(
    "mtu_status = set_interface_mtu(man_ifname, dhcp_mtu);", tv_mtu_condition
)
tv_mtu_log = viptv_bound.index("MTU setup incomplete (status=%d)", tv_mtu_set)
tv_routes = viptv_bound.index("route_result = add_dhcp_routes(", tv_mtu_log)
tv_route_err = viptv_bound.index(
    'wan_nvram_int_temp_status("manv_route_err", route_setup_status);',
    tv_routes,
)
tv_return = viptv_bound.index("return callback_status;", tv_route_err)
require(
    'wan_nvram_int_temp_status("manv_err", i_err);' in viptv_bound,
    "IPTV DHCP lease error marker must use checked NVRAM publication",
)
require(
    "callback_status = route_status_accumulate(route_setup_status, multicast_status);" in viptv_bound,
    "IPTV DHCP callback must preserve multicast status separately from route retry state",
)
require(
    "callback_status = route_status_accumulate(callback_status, publish_status);" in viptv_bound,
    "IPTV DHCP callback must preserve the first NVRAM publication failure",
)
require(
    tv_mtu_condition < tv_mtu_set < tv_mtu_log < tv_routes < tv_route_err < tv_return,
    "IPTV DHCP MTU remains diagnostic-only while route and multicast status are returned",
)
require("route_status_accumulate(route_setup_status, mtu_status)" not in viptv_bound,
        "optional IPTV MTU failure must not be merged into route retry status")
require(
    "!is_renew_mode" not in viptv_bound[tv_mtu_condition - 80:tv_mtu_set],
    "changed IPTV DHCP MTU must also be applied during renew callbacks",
)

viptv_renew = body(WAN, "udhcpc_viptv_renew(char *man_ifname)")
tv_renew_compact = "".join(viptv_renew.split())
for key, target in (
    ("err", "lease_error"),
    ("route_err", "route_error"),
    ("mtu_err", "mtu_error"),
):
    require(
        'get_nvram_int_checked_status(strcat_r(prefix,"' + key + '",tmp),&' +
        target + ');' in tv_renew_compact,
        "IPTV renewal must check previous DHCP retry state: " + key,
    )
tv_retry = tv_renew_compact.index("dhcp_wan_bound_retry_decide(")
tv_retry_error = tv_renew_compact.index("if(nvram_status!=0)", tv_retry)
tv_retry_gate = tv_renew_compact.index("if(retry_required)", tv_retry_error)
tv_retry_bound = tv_renew_compact.index("returnudhcpc_viptv_bound(man_ifname,1);", tv_retry_gate)
tv_renew_mtu = tv_renew_compact.index('value=getenv("mtu");', tv_retry_bound)
tv_mtu_change_bound = tv_renew_compact.index(
    "returnudhcpc_viptv_bound(man_ifname,1);", tv_renew_mtu
)
require(
    tv_retry < tv_retry_error < tv_retry_gate < tv_retry_bound <
    tv_renew_mtu < tv_mtu_change_bound,
    "IPTV renewal must fail closed on retry-state read errors, retry incomplete lease state, and re-enter bound after DHCP MTU changes",
)
require("cannot verify previous DHCP retry state" in viptv_renew and
        'get_nvram_int_checked_status("manv_mtu",&stored_mtu);' in tv_renew_compact,
        "IPTV renewal must use checked stored-MTU reads")
assert "nvram_get_int(" not in viptv_renew
assert "dhcp_wan_bound_retry_required(" not in viptv_renew

dhcp_mtu_policy = body(POLICY, "wan_dhcp_mtu_is_supported(")
require(
    "dhcp_mtu >= 576 && dhcp_mtu <= 1500" in dhcp_mtu_policy,
    "DHCP MTU helper must match parser-supported values 576..1500",
)
require(
    "wan_dhcp_mtu_is_supported(dhcp_mtu)" in wan_bound
    and "wan_dhcp_mtu_is_supported(dhcp_mtu)" in viptv_bound,
    "WAN and IPTV must apply every supported DHCP MTU",
)
for callback in (wan_bound, viptv_bound):
    require(
        "dhcp_lease_decimal_is_valid(value, 576U, 1500U, &parsed_value)" in callback,
        "DHCP parser and setter ranges must remain aligned",
    )
require(
    all(token in DHCP_TEST for token in (
        "wan_dhcp_mtu_is_supported(575)",
        "wan_dhcp_mtu_is_supported(576)",
        "wan_dhcp_mtu_is_supported(1299)",
        "wan_dhcp_mtu_is_supported(1300)",
        "wan_dhcp_mtu_is_supported(1499)",
        "wan_dhcp_mtu_is_supported(1500)",
        "wan_dhcp_mtu_is_supported(1501)",
    )),
    "DHCP MTU regression must cover all boundary cases",
)
require(
    "test_dhcp_optional_mtu_failure_does_not_request_route_retry();" in DHCP_TEST
    and "assert(!dhcp_wan_route_setup_retry_required(route_setup_status));" in DHCP_TEST,
    "DHCP interface regression must preserve diagnostic-only optional MTU semantics",
)
for path in ("trunk/user/rc/src/tests/dhcp_wan_interface_policy.c",):
    require(
        WAN_WORKFLOW.count('"' + path + '"') >= 2,
        "WAN push and pull_request filters must include " + path,
    )

print("WAN required and optional MTU policy: PASS")
