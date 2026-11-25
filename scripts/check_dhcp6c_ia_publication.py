#!/usr/bin/env python3
"""Check DHCPv6 IA publication retry and partial-prefix reconciliation contracts."""
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
BASE = "trunk/user/busybox/busybox-1.24.x/networking/udhcp/"


def read(path):
    return (ROOT / path).read_text()


def function_body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth, state, pos = 0, "code", brace
    while pos < len(source):
        ch = source[pos]
        nxt = source[pos + 1] if pos + 1 < len(source) else ""
        if state == "code":
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "/" and nxt == "*":
                state = "block_comment"
                pos += 1
            elif ch == "/" and nxt == "/":
                state = "line_comment"
                pos += 1
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return source[brace + 1:pos]
        elif state in ("string", "char"):
            if ch == "\\":
                pos += 1
            elif state == "string" and ch == '"':
                state = "code"
            elif state == "char" and ch == "'":
                state = "code"
        elif state == "block_comment":
            if ch == "*" and nxt == "/":
                state = "code"
                pos += 1
        elif state == "line_comment" and ch == "\n":
            state = "code"
        pos += 1
    raise AssertionError("unterminated function: " + signature)


ia = read(BASE + "dhcp6c_ia.c")
client = read(BASE + "dhcp6c.c")
header = read(BASE + "dhcp6c.h")
policy = read(BASE + "dhcp6c_ia_policy.h")
test = read(BASE + "tests/dhcp6c_ia_publication_policy.c")
workflow = read(".github/workflows/dhcp6c-ia-publication-stability.yml")

assert '#include "dhcp6c_ia_policy.h"' in ia
assert '#include "dhcp6c_ia_policy.h"' in client
assert "int update_ia(struct dhcp6_list *iahead" in ia
assert "int update_ia(struct dhcp6_list *iahead" in header
assert "dhcp6c_ia_pd_target_needs_apply" in policy
assert "dhcp6c_ia_first_error" in policy
assert "dhcp6c_ia_retry_required" in policy

installed = function_body(ia, "static int ifprefix_is_installed(struct iaprefix *sp, struct prefix_ifconf *pif)")
assert "TAILQ_FOREACH(ip, &sp->u.ifprefix_list, plink)" in installed
assert "ip->ifconf == pif" in installed

update_prefix = function_body(ia, "static int update_prefix6(struct ia *ia,")
assert "if (lv->lvtype == DHCP6_LISTVAL_PREFIX6)" in update_prefix
assert "ifprefix_is_installed(sp, pif)" in update_prefix
assert "dhcp6c_ia_pd_target_needs_apply(" in update_prefix
assert "if (add_ifprefix(sp, &pinfo, pif) != 0)" in update_prefix
assert "&& spcreate" not in update_prefix
assert "errno = EINVAL;" in update_prefix

update_ia = function_body(ia, "int update_ia(struct dhcp6_list *iahead")
assert "int ia_error = 0, update_status;" in update_ia
assert "dhcp6c_ia_first_error(ia_error" in update_ia
assert "first_error = dhcp6c_ia_first_error(first_error, ia_error);" in update_ia
assert "if (ia_error != 0)" in update_ia
assert update_ia.index("if (ia_error != 0)") < update_ia.index("if (ia->ctl == NULL || !pref_isvalid(ia->ctl))")
assert update_ia.index("if (ia_error != 0)") < update_ia.index("dhcp6_timer_set(ia->t1 * 1000ULL, ia->timer)")
assert "return first_error;" in update_ia

reply = function_body(client, "static int client6_recvreply(")
apply_begin = reply.index("if (state != DHCP6S_RELEASE)")
script_begin = reply.index("if (ifp->scriptpath != NULL")
apply_block = reply[apply_begin:script_begin]
assert "ia_status = update_ia(" in apply_block
assert "dhcp6c_ia_retry_required(ia_status)" in apply_block
assert "DHCP6C_SCRIPT_RETRY_DELAY_MS" in apply_block
assert "return -1;" in apply_block
assert "script_status = dhcp6_script(" in reply

assert "test_partial_pd_failure_is_retried_without_reapplying_successes" in test
assert "test_first_error_and_retry_gate" in test
assert "DHCPv6 IA delegated-prefix retry policy: PASS" in test

required_paths = (
    ".github/workflows/dhcp6c-ia-publication-stability.yml",
    "scripts/check_dhcp6c_ia_publication.py",
    BASE + "dhcp6c_ia.c",
    BASE + "dhcp6c.c",
    BASE + "dhcp6c.h",
    BASE + "dhcp6c_ia_policy.h",
    BASE + "tests/dhcp6c_ia_publication_policy.c",
)
filters = workflow.split("jobs:", 1)[0]
push, pull = filters.split("  pull_request:", 1)
for path in required_paths:
    quoted = '"' + path + '"'
    assert push.count(quoted) == 1, "push filter missing/duplicate: " + path
    assert pull.count(quoted) == 1, "pull_request filter missing/duplicate: " + path
print("DHCPv6 IA retry/publication source contracts: PASS")
