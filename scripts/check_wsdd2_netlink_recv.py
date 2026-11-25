#!/usr/bin/env python3
"""Ensure WSDD2 never silently consumes truncated netlink address events."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/wsdd2/src/wsdd2.c").read_text()
POLICY = (ROOT / "trunk/user/wsdd2/src/wsd_netlink_recv_policy.h").read_text()
TEST = (ROOT / "trunk/user/wsdd2/src/tests/wsd_netlink_recv_policy_test.c").read_text()
RUNNER = (ROOT / "scripts/test_wsdd2_netlink_recv.sh").read_text()
WORKFLOW = (ROOT / ".github/workflows/wsdd2-netlink-recv-regression.yml").read_text()
XY_WORKFLOW = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()


def compact(value):
    return re.sub(r"\s+", " ", value).strip()


def function_body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:pos]
    raise AssertionError("unterminated function: " + signature)


assert '#include "wsd_netlink_recv_policy.h"' in SOURCE
receiver = function_body(SOURCE, "static int netlink_recv(struct endpoint *ep)\\n{")
assert "recvmsg(ep->sock, &msg, 0)" in receiver
assert "wsd_netlink_datagram_truncated(msg.msg_flags)" in receiver
assert receiver.index("wsd_netlink_datagram_truncated(msg.msg_flags)") < receiver.index("for (nh = (struct nlmsghdr *)buf")
reject = receiver[receiver.index("wsd_netlink_datagram_truncated(msg.msg_flags)"):receiver.index("for (nh = (struct nlmsghdr *)buf")]
assert "ep->_errno = EMSGSIZE;" in reject
assert "return -1;" in reject
assert "MSG_TRUNC" in POLICY
assert "message_flags & MSG_TRUNC" in compact(POLICY)
for token in ("MSG_TRUNC", "MSG_DONTWAIT", "wsd_netlink_datagram_truncated"):
    assert token in TEST
assert "-std=gnu89 -Wall -Wextra -Werror" in RUNNER
assert "-fsanitize=address,undefined" in RUNNER
assert "wsd_netlink_recv_policy_test.c" in RUNNER
for path in (
    ".github/workflows/wsdd2-netlink-recv-regression.yml",
    "trunk/user/wsdd2/src/wsdd2.c",
    "trunk/user/wsdd2/src/wsd_netlink_recv_policy.h",
    "trunk/user/wsdd2/src/tests/wsd_netlink_recv_policy_test.c",
    "scripts/test_wsdd2_netlink_recv.sh",
    "scripts/check_wsdd2_netlink_recv.py",
):
    assert path in WORKFLOW, path
assert 'trunk/user/wsdd2/**' in XY_WORKFLOW
print("wsdd2 netlink truncation source contract: PASS")
