#!/usr/bin/env python3
"""Require NAT restoration when a committed NAT deletion is followed by filter failure."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/netfilter/iptcrdr.c").read_text()
POLICY = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/netfilter/iptcrdr_update_policy.h").read_text()
CLEANUP_POLICY = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/miniupnpd_redirect_cleanup_policy.h").read_text()
UPNP_REDIRECT = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/upnpredirect.c").read_text()
IPT_HEADER = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/netfilter/iptcrdr.h").read_text()


def function_body(source, signature):
    position = source.index(signature)
    while True:
        brace = source.find("{", position)
        semicolon = source.find(";", position)
        assert brace >= 0, "function definition missing: " + signature
        if semicolon >= 0 and semicolon < brace:
            position = source.find(signature, position + len(signature))
            assert position >= 0, "function definition missing: " + signature
            continue
        depth = 0
        for index in range(brace, len(source)):
            if source[index] == "{":
                depth += 1
            elif source[index] == "}":
                depth -= 1
                if depth == 0:
                    return " ".join(source[position:index + 1].split())
        raise AssertionError("unterminated function: " + signature)


helper = function_body(
    SOURCE,
    "insert_rule_and_commit(unsigned int index, const struct ipt_entry *entry,"
)
delete_helper = function_body(
    SOURCE,
    "delete_rule_and_commit(unsigned int index, IPTC_HANDLE h,"
)
for cleanup_helper in (helper, delete_helper):
    assert "miniupnpd_restore_errno_on_failure(" in cleanup_helper
    assert "failure_errno = errno" in cleanup_helper
    assert cleanup_helper.index("iptc_free(") < cleanup_helper.rindex(
        "miniupnpd_restore_errno_on_failure("
    )
assert "return miniupnpd_restore_errno_on_failure(-1, failure_errno);" in helper
delete = function_body(
    SOURCE,
    "delete_redirect_and_filter_rules_ex(unsigned short eport, int proto, int *primary_removed)"
)
lookup = function_body(SOURCE, "get_redirect_rule_by_index(")
assert "nat_scan_completed" not in lookup
assert delete.count("nat_scan_completed = 1;") == 1
scan_guard = delete.index("if(!iptc_is_chain(miniupnpd_nat_chain, h))")
scan_loop = delete.index("for(e = iptc_first_rule(miniupnpd_nat_chain", scan_guard)
scan_complete = delete.index("nat_scan_completed = 1;", scan_loop)
scan_loop_close = delete.index("} nat_scan_completed = 1;", scan_loop)
peer_cleanup = delete.index("/*delete PEER rule*/", scan_loop)
assert scan_guard < scan_loop < scan_loop_close < scan_complete < peer_cleanup
assert "r = -1;" in delete[scan_guard:scan_complete]
legacy_delete = function_body(
    SOURCE,
    "delete_redirect_and_filter_rules(unsigned short eport, int proto)"
)
delete_wrapper = function_body(
    UPNP_REDIRECT,
    "_upnp_delete_redir(unsigned short eport, int proto)"
)
assert "primary_failure_errno" in delete
assert "secondary_failure_errno" in delete
assert "primary_failure_errno = miniupnpd_record_first_failure_errno(primary_failure_errno, ENOENT)" in delete
assert "miniupnpd_select_cleanup_failure_errno(" in delete
assert "primary_failure_errno = miniupnpd_record_first_failure_errno(primary_failure_errno," in delete
assert "secondary_failure_errno = miniupnpd_record_first_failure_errno(secondary_failure_errno," in delete
errno_capture_assignments = [
    statement.strip() for statement in delete.split(";")
    if re.search(r"\\b(?:primary_failure_errno|secondary_failure_errno)\\s*=", statement)
    and not re.search(r"\\bint\\s+(?:primary_failure_errno|secondary_failure_errno)\\s*=", statement)
]
assert errno_capture_assignments
assert all(
    "miniupnpd_record_first_failure_errno(" in line
    for line in errno_capture_assignments
)
assert "miniupnpd_restore_errno_on_failure(cleanup_status, failure_errno)" in delete
assert "return miniupnpd_restore_errno_on_failure(-1, failure_errno)" in delete
assert "secondary_failure_errno = miniupnpd_record_first_failure_errno(secondary_failure_errno, errno)" in delete

policy = " ".join(POLICY.split())

assert helper.index('h = iptc_init(table)') < helper.index("iptc_insert_entry(chain, entry, index, h)")
assert "iptc_insert_entry(chain, entry, index, &h)" in helper
assert helper.index("iptc_insert_entry(") < helper.index("iptc_commit(")
assert helper.count("iptc_free(") == 2

snapshot = delete.index("nat_backup = malloc(nat_entry_len)")
copy = delete.index("memcpy(nat_backup, e, nat_entry_len)", snapshot)
nat_delete = delete.index("delete_rule_and_commit(index, h, miniupnpd_nat_chain")
assert delete.index("nat_index = i") < snapshot < copy < nat_delete
assert "if (nat_backup_failed) return miniupnpd_restore_errno_on_failure(r, primary_failure_errno);" in delete

nat_status = delete.index("nat_delete_status = r", nat_delete)
filter_open = delete.index('h = iptc_init("filter")', nat_status)
filter_failure = delete.index("r = -1;", filter_open)
rollback_gate = delete.index(
    "miniupnpd_nat_delete_rollback_required(nat_delete_status, r)", filter_failure
)
restore = delete.index(
    'insert_rule_and_commit( nat_index, nat_backup, "nat", miniupnpd_nat_chain, "delete_redirect_and_filter_rules")',
    rollback_gate,
)
assert nat_delete < nat_status < filter_open < filter_failure < rollback_gate < restore
assert "miniupnpd_nat_delete_failure_result(r, rollback_status)" in delete[rollback_gate:]
assert "errno = failure_errno;" in delete[rollback_gate:]
assert delete.index("free(nat_backup)", rollback_gate) > restore
assert "return nat_status == 0 && filter_status != 0;" in policy
assert "return filter_status != 0 ? filter_status : rollback_status;" in policy

# Persistent lease metadata follows primary DNAT/filter state, not peer cleanup.
cleanup_policy = " ".join(CLEANUP_POLICY.split())
assert "miniupnpd_primary_redirect_removed(" in cleanup_policy
assert "nat_scan_completed" in delete
assert "*primary_removed = 0" in delete
assert "filter_delete_status = r" in delete
assert "nat_rollback_attempted = 1" in delete
assert "miniupnpd_primary_redirect_removed(" in delete
assert "miniupnpd_finalize_redirect_cleanup(" in delete
assert "del_redirect_desc" in delete
assert "delete_redirect_and_filter_rules_ex(eport, proto, NULL)" in legacy_delete
assert "delete_redirect_and_filter_rules_ex(unsigned short eport, int proto," in IPT_HEADER
assert "delete_redirect_and_filter_rules_ex(eport, proto, &primary_removed)" in delete_wrapper
assert "if (primary_removed)" in delete_wrapper
assert 'if (r == 0)' not in delete_wrapper

print("MiniUPnP NAT/filter delete compensation and lease-state contract: PASS")
