#!/usr/bin/env python3
"""Require rollback of the NAT rule after a later filter update fails."""

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
                    return source[position:index + 1]
        raise AssertionError("unterminated function: " + signature)


body = " ".join(function_body(
    SOURCE,
    "update_portmapping(const char * ifname, unsigned short eport, int proto,"
).split())
policy = " ".join(POLICY.split())

nat_commit = body.index('update_rule_and_commit("nat", miniupnpd_nat_chain, nat_index, new_e)')
filter_open = body.index('iptc_init("filter")')
assert nat_commit < filter_open
assert "nat_index = index;" in body
assert "nat_status = r;" in body

filter_init_failure = body.index("goto rollback_nat;", filter_open)
assert filter_init_failure < body.index("i = 0;", filter_open)

chain_check = body.index("if (!iptc_is_chain(miniupnpd_forward_chain, h))", filter_open)
chain_failure = body.index("r = -1;", chain_check)
filter_table_close = body.index("iptc_free", chain_failure)
assert body.index("goto rollback_nat;", filter_table_close) > filter_table_close

not_found = body.index("if (!found || r < 0)", filter_table_close)
assert "goto rollback_nat;" in body[not_found:not_found + 100]

filter_commit = body.index('update_rule_and_commit("filter", miniupnpd_forward_chain, index, filter_e)')
rollback_check = body.index(
    "if (miniupnpd_portmapping_rollback_required(nat_status, r))", filter_commit
)
assert "goto rollback_nat;" in body[rollback_check:rollback_check + 120]

rollback = body.index("rollback_nat:")
rollback_gate = body.index("miniupnpd_portmapping_rollback_required(nat_status, r)", rollback)
restore = body.index("htons(old_iport)", rollback_gate)
rollback_commit = body.index("update_rule_and_commit(", rollback)
rollback_arguments = body.index(
    '"nat", miniupnpd_nat_chain, nat_index, new_e)', rollback_commit
)
assert rollback_gate < restore < rollback_commit < rollback_arguments
assert "miniupnpd_portmapping_failure_result(r, rollback_status)" in body[rollback:]
assert "errno = failure_errno;" in body[rollback:]
assert "free(filter_e);" in body[rollback:]
assert "free(new_e);" in body[rollback:]

assert "return first_status == 0 && second_status != 0;" in policy
assert "return update_status != 0 ? update_status : rollback_status;" in policy
print("MiniUPnP NAT/filter update rollback source contract: PASS")
