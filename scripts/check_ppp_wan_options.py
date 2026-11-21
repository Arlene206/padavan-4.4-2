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

launch = body(net_ppp, "launch_wan_pppd_locked(int unit, int wan_proto)")
preflight = launch.index("ppp_wan_option_values_read(")
generation = launch.index("ppp_wan_session_generation_change(&callback_generation)")
options_open = launch.index("ppp_options_file_open(options)")
stop = launch.find("kill_services(svcs, 5, 1)")
require(preflight < generation < options_open,
        "PPP options must be checked before session invalidation and file open")
require(stop < 0 or preflight < stop,
        "L2TP daemon stop must follow checked option reads")
require("get_wan_unit_value_int(" not in launch,
        "PPP builder still uses error-collapsing integer accessor")
for field in ("auth_type","mtu","mru","pppoe_idletime","pppoe_demand",
              "pppoe_txonly_x","dnsenable_x","mppe_mode","lcp_echo","adaptive_lcp"):
    require("option_values." + field in launch, "unchecked PPP option: " + field)

policy_body = body(policy, "ppp_wan_option_values_read(int is_pppoe")
for token in ('PPP_WAN_READ_OPTION("ppp_auth", auth_type)',
              'PPP_WAN_READ_OPTION("dnsenable_x", dnsenable_x)',
              'PPP_WAN_READ_OPTION("ppp_mppe", mppe_mode)',
              'PPP_WAN_READ_OPTION("ppp_lcp", lcp_echo)',
              'PPP_WAN_READ_OPTION("ppp_alcp", adaptive_lcp)',
              "memset(values, 0, sizeof(*values))"):
    require(token in policy_body, "PPP option policy missing " + token)

routes = body(net_ppp, "preset_wan_ppp_routes(char *ppp_ifname, int unit)")
primary = routes.index('get_wan_unit_value_int_status(unit, "primary", &primary)')
default_route = routes.index('route_add(ppp_ifname, 1, "0.0.0.0"')
static_routes = routes.index("add_static_wan_routes(ppp_ifname)")
require(primary < default_route < static_routes,
        "PPP default route must use checked primary value and still restore static routes")
require("status = route_status_accumulate(status, route_status)" in routes,
        "PPP primary read error must be reported")

for name in ("test_pppoe_options","test_pptp_and_l2tp_options",
             "test_missing_values_keep_zero_defaults",
             "test_failures_stop_and_clear_snapshot",
             "test_invalid_arguments_fail_closed"):
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
print("PPP NVRAM config read and default-route fail-closed contracts: PASS")
