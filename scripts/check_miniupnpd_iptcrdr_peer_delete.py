#!/usr/bin/env python3
"""Require peer SNAT restoration when a later mangle/DSCP deletion fails."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/netfilter/iptcrdr.c").read_text()
POLICY = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/netfilter/iptcrdr_update_policy.h").read_text()


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
delete = function_body(
    SOURCE,
    "delete_redirect_and_filter_rules(unsigned short eport, int proto)"
)
policy = " ".join(POLICY.split())

peer_snapshot = delete.index("peer_backup = malloc(peer_entry_len)")
peer_copy = delete.index("memcpy(peer_backup, e, peer_entry_len)", peer_snapshot)
peer_delete = delete.index("delete_rule_and_commit(index, h, miniupnpd_nat_postrouting_chain")
assert delete.index("peer_index = i") < peer_snapshot < peer_copy < peer_delete
assert "peer_delete_status = r2;" in delete
mangle_open = delete.index('h = iptc_init("mangle")', peer_delete)
rollback_gate = delete.index(
    "miniupnpd_peer_delete_rollback_required(peer_delete_status, r2)", mangle_open
)
restore = delete.index(
    'insert_rule_and_commit( peer_index, peer_backup, "nat", miniupnpd_nat_postrouting_chain, "delete_redirect_and_filter_rules")',
    rollback_gate,
)
assert peer_delete < mangle_open < rollback_gate < restore
assert "miniupnpd_peer_delete_failure_result(r2, rollback_status)" in delete[rollback_gate:]
assert "errno = failure_errno;" in delete[rollback_gate:]
assert delete.index("free(peer_backup)", rollback_gate) > restore

assert "iptc_insert_entry(chain, entry, index, h)" in helper
assert "iptc_insert_entry(chain, entry, index, &h)" in helper
assert "peer_backup_failed" not in delete
assert "return peer_status == 0 && dscp_status != 0;" in policy
assert "return dscp_status != 0 ? dscp_status : rollback_status;" in policy
print("MiniUPnP peer SNAT/DSCP compensation source contract: PASS")
