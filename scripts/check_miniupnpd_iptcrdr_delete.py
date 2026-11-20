#!/usr/bin/env python3
"""Require NAT restoration when a committed NAT deletion is followed by filter failure."""

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

assert helper.index('h = iptc_init(table)') < helper.index("iptc_insert_entry(chain, entry, index, h)")
assert "iptc_insert_entry(chain, entry, index, &h)" in helper
assert helper.index("iptc_insert_entry(") < helper.index("iptc_commit(")
assert helper.count("iptc_free(") == 2

snapshot = delete.index("nat_backup = malloc(nat_entry_len)")
copy = delete.index("memcpy(nat_backup, e, nat_entry_len)", snapshot)
nat_delete = delete.index("delete_rule_and_commit(index, h, miniupnpd_nat_chain")
assert delete.index("nat_index = i") < snapshot < copy < nat_delete
assert "if (nat_backup_failed) return r;" in delete

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

print("MiniUPnP NAT/filter delete compensation source contract: PASS")
