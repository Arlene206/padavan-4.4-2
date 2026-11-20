#!/usr/bin/env python3
"""Guard MiniUPnP netfilter rule builders against failed component allocations."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/netfilter/iptcrdr.c").read_text()
POLICY = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/netfilter/iptcrdr_allocation_policy.h").read_text()


def function_body(signature):
    position = SOURCE.index(signature)
    while True:
        brace = SOURCE.find("{", position)
        semicolon = SOURCE.find(";", position)
        if brace < 0:
            raise AssertionError("function definition missing: " + signature)
        if semicolon >= 0 and semicolon < brace:
            position = SOURCE.find(signature, position + len(signature))
            if position < 0:
                raise AssertionError("function definition missing: " + signature)
            continue
        depth = 0
        for index in range(brace, len(SOURCE)):
            if SOURCE[index] == "{":
                depth += 1
            elif SOURCE[index] == "}":
                depth -= 1
                if depth == 0:
                    return SOURCE[position:index + 1]
        raise AssertionError("unterminated function: " + signature)


assert '#include "iptcrdr_allocation_policy.h"' in SOURCE
assert "free(target);" in POLICY and "free(match);" in POLICY and "free(entry);" in POLICY
assert "errno = saved_errno;" in POLICY

factories = (
    ("get_tcp_match(unsigned short dport, unsigned short sport)", "match"),
    ("get_udp_match(unsigned short dport, unsigned short sport)", "match"),
    ("get_dnat_target(const char * daddr, unsigned short dport)", "target"),
    ("get_snat_target(const char * saddr, unsigned short sport)", "target"),
    ("get_dscp_target(unsigned char dscp)", "target"),
    ("get_masquerade_target(unsigned short port)", "target"),
    ("get_accept_target(void)", "target"),
)
for signature, pointer in factories:
    body = function_body(signature)
    allocation = body.index(pointer + " = calloc(1, size);")
    guard = body.index("if (!" + pointer + ")", allocation)
    null_return = body.index("return NULL;", guard)
    dereference = body.index(pointer + "->", allocation)
    assert allocation < guard < null_return < dereference, signature
    assert "errno = saved_errno;" in body[guard:null_return]


builders = (
    ("addnatrule(int proto, unsigned short eport,", "addnatrule", "get_dnat_target", "DNAT target"),
    ("addmasqueraderule(int proto,", "addmasqueraderule", "get_masquerade_target", "MASQUERADE target"),
    ("addpeernatrule(int proto,", "addpeernatrule", "get_snat_target", "SNAT target"),
    ("addpeerdscprule(int proto, unsigned char dscp,", "addpeerdscprule", "get_dscp_target", "DSCP target"),
    ("add_filter_rule(int proto,", "add_filter_rule", "get_accept_target", "ACCEPT target"),
)
for signature, caller, target_factory, target_name in builders:
    body = function_body(signature)

    entry_guard = body.index("if (!e)")
    entry_cleanup = body.index(
        'miniupnpd_iptcrdr_allocation_failure(NULL, NULL, NULL, "' + caller + '", "rule entry")'
    )
    first_entry_use = body.index("e->ip.proto")
    assert entry_guard < entry_cleanup < first_entry_use, caller + " entry allocation"

    match_guard = body.index("if (!match)")
    match_cleanup = body.index(
        'miniupnpd_iptcrdr_allocation_failure(e, NULL, NULL, "' + caller + '", "protocol match")'
    )
    match_deref = body.index("match->")
    assert match_guard < match_cleanup < match_deref, caller + " match allocation"

    target_call = body.index("target = " + target_factory + "(")
    target_guard = body.index("if (!target)", target_call)
    target_cleanup = body.index(
        'miniupnpd_iptcrdr_allocation_failure(e, match, NULL, "' + caller + '", "' + target_name + '")',
        target_call,
    )
    target_deref = body.index("target->", target_call)
    assert target_call < target_guard < target_cleanup < target_deref, caller + " target allocation"

    realloc_call = body.index("tmp = realloc(", target_call)
    realloc_guard = body.index("if (!tmp)", realloc_call)
    realloc_cleanup = body.index(
        'miniupnpd_iptcrdr_allocation_failure(e, match, target, "' + caller + '", "rule buffer")',
        realloc_guard,
    )
    assert realloc_call < realloc_guard < realloc_cleanup < body.index("e = tmp;", realloc_guard), caller + " realloc failure"

print("MiniUPnP iptables allocation source contract: PASS")
