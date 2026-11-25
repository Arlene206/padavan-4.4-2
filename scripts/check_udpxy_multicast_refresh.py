#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(".")
BASE = ROOT / "trunk/user/udpxy/src"
netop = (BASE / "netop.c").read_text()
netop_h = (BASE / "netop.h").read_text()
udpxy = (BASE / "udpxy.c").read_text()
policy = (BASE / "udpxy_multicast_refresh_policy.h").read_text()
test = (BASE / "test/udpxy_multicast_refresh_policy.c").read_text()
workflow = (ROOT / ".github/workflows/udpxy-multicast-refresh-stability.yml").read_text()
xy_workflow = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()


def body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:index]
    raise AssertionError("unterminated function: " + signature)


renew = body(netop, "renew_multicast( int msockfd")
check_refresh = body(udpxy, "check_mcast_refresh( int msockfd")
policy_renew = body(policy, "udpxy_renew_multicast_with(")
retry_anchor = body(policy, "udpxy_mcast_refresh_anchor(")

assert '"udpxy_multicast_refresh_policy.h"' in netop
assert '"udpxy_multicast_refresh_policy.h"' in udpxy
assert "udpxy_renew_multicast_with( renew_multicast_operation" in renew
assert "set_multicast( msockfd, mifaddr, s_in_addr, opname )" in netop
assert "const char* opname" in netop
assert "const char* opname" in netop_h

assert 'update(msockfd, mifaddr, s_in_addr, "DROP", context)' in policy_renew
assert 'return update(msockfd, mifaddr, s_in_addr, "ADD", context)' in policy_renew
assert "if (0 == refresh_status || interval <= (time_t)1)" in retry_anchor
assert "return now - interval + (time_t)1" in retry_anchor

assert "int refresh_rc" in check_refresh
assert "(time_t)-1 == now" in check_refresh
assert "refresh_rc = renew_multicast( msockfd, mifaddr, s_in_addr)" in check_refresh
assert "if( 0 != refresh_rc )" in check_refresh
assert "udpxy_mcast_refresh_anchor(" in check_refresh
assert "(void) renew_multicast" not in check_refresh
assert "*last_tm = now;" not in check_refresh

for token in (
    "test_drop_failure_still_adds",
    "test_add_failure_is_reported_after_drop_success",
    "test_retry_recovers_after_partial_renewal",
    "test_retry_deadline",
):
    assert token in test, token

expected_paths = (
    ".github/workflows/udpxy-multicast-refresh-stability.yml",
    "trunk/user/udpxy/src/netop.c",
    "trunk/user/udpxy/src/netop.h",
    "trunk/user/udpxy/src/udpxy.c",
    "trunk/user/udpxy/src/udpxy_multicast_refresh_policy.h",
    "trunk/user/udpxy/src/test/udpxy_multicast_refresh_policy.c",
    "scripts/check_udpxy_multicast_refresh.py",
)
push, pr = workflow.split("  pull_request:", 1)[0], workflow.split("permissions:", 1)[0].split("  pull_request:", 1)[1]
for path in expected_paths:
    quoted = '"' + path + '"'
    assert push.count(quoted) == 1, ("push path missing/duplicated", path)
    assert pr.count(quoted) == 1, ("pull_request path missing/duplicated", path)
assert xy_workflow.count('"trunk/user/udpxy/**"') == 1

print("UDPXY multicast renewal and retry source contract: PASS")
