#!/usr/bin/env python3
from pathlib import Path

common = Path("trunk/user/rc/src/common_ex.c").read_text()
policy = Path("trunk/user/rc/src/router_mode_nvram_policy.h").read_text()
regression = Path("trunk/user/rc/src/tests/router_mode_nvram_policy.c").read_text()
header = Path("trunk/user/rc/src/rc.h").read_text()
wan = Path("trunk/user/rc/src/net_wan.c").read_text()
rc = Path("trunk/user/rc/src/rc.c").read_text()
workflow = Path(".github/workflows/router-mode-update.yml").read_text()


def body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for position in range(brace, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:position]
    raise AssertionError("unterminated function: " + signature)


update = body(common, "update_router_mode(void)")
update_compact = " ".join(update.split())
assert "nvram_transaction_begin(&transaction)" in update
assert 'nvram_transaction_get(&transaction, "sw_mode"' in update
assert "rc_router_mode_update_needed(api_result, saved_errno" in update
assert update.index('nvram_transaction_get(&transaction, "sw_mode"') < update.index('nvram_transaction_get(&transaction, "wan_nat_x"')
assert "rc_router_mode_update_target(api_result, saved_errno" in update
assert update.index('nvram_transaction_get(&transaction, "wan_nat_x"') < update.index("rc_router_mode_update_target(api_result, saved_errno")
assert 'nvram_transaction_set(&transaction, "sw_mode"' in update
assert update.index('nvram_transaction_set(&transaction, "sw_mode"') < update.index("nvram_transaction_end(&transaction)")
assert "nvram_transaction_end(&transaction)" in update
assert "rc_router_mode_nvram_write_status(api_result, saved_errno)" in update_compact
assert 'nvram_get_int("sw_mode")' not in update
assert 'nvram_get_int("wan_nat_x")' not in update
assert 'nvram_set_int("sw_mode"' not in update
assert "if (status == 0 && update_needed)" in update
assert "status = rc_router_mode_nvram_write_status(end_result, end_errno)" in update

needed = body(policy, "rc_router_mode_update_needed(")
target = body(policy, "rc_router_mode_update_target(")
assert "rc_router_mode_nvram_read_status" in needed
assert "rc_router_mode_parse_integer" in needed
assert "mode != 3" in needed
assert "rc_router_mode_nvram_read_status" in target
assert "rc_router_mode_parse_integer" in target
assert "nat_enabled == 0 ? 4 : 1" in target

for marker in (
    "test_router_mode_update_detection_preserves_ap_mode",
    "test_router_mode_update_target_preserves_nat_mapping",
    "test_router_mode_update_write_status",
    "rc_router_mode_update_needed(-1, EACCES, NULL",
    'rc_router_mode_update_target(1, 0, "1junk"',
):
    assert marker in regression, marker
assert "int update_router_mode(void);" in header

full_wan = body(wan, "full_restart_wan(void)")
assert full_wan.index("if (get_ap_mode())") < full_wan.index("update_router_mode()")
assert full_wan.index("update_router_mode()") < full_wan.index("stop_wan()")
assert full_wan.count("update_router_mode()") == 1

branch_start = rc.index("else if (strcmp(entry->d_name, RCN_RESTART_NETFILTER) == 0)")
branch_end = rc.index("else if (strcmp(entry->d_name, RCN_RESTART_FIREWALL) == 0)", branch_start)
branch = rc[branch_start:branch_end]
assert "router_mode_status = update_router_mode()" in branch
assert branch.index("if (router_mode_status == 0)") < branch.index("reload_nat_modules()")
assert branch.index("if (router_mode_status == 0)") < branch.index("restart_firewall()")
assert "rc_notify_merge_action_result(action_result, -1)" in branch
assert "flush_conntrack_table(NULL)" in branch

paths = (
    ".github/workflows/router-mode-update.yml",
    "scripts/check_router_mode_update.py",
    "trunk/user/rc/src/common_ex.c",
    "trunk/user/rc/src/router_mode_nvram_policy.h",
    "trunk/user/rc/src/tests/router_mode_nvram_policy.c",
    "trunk/user/rc/src/rc.h",
    "trunk/user/rc/src/rc.c",
    "trunk/user/rc/src/net_wan.c",
)
push = workflow.split("  pull_request:", 1)[0]
pr = workflow.split("  workflow_dispatch:", 1)[0].split("  pull_request:", 1)[1]
for path in paths:
    assert push.count('"' + path + '"') == 1, path
    assert pr.count('"' + path + '"') == 1, path

print("Router-mode NVRAM update failure gate: PASS")
