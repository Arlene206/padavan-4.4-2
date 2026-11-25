#!/usr/bin/env python3
"""Keep wsdd2 TCP message writes complete across short writes and EINTR."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/wsdd2/src/wsd.c").read_text()
POLICY = (ROOT / "trunk/user/wsdd2/src/wsd_stream_send_policy.h").read_text()
TEST = (ROOT / "trunk/user/wsdd2/src/tests/wsd_stream_send_policy_test.c").read_text()
SCRIPT = (ROOT / "scripts/test_wsdd2_stream_send.sh").read_text()
WORKFLOW = (ROOT / ".github/workflows/wsdd2-stream-send-regression.yml").read_text()
XY_WORKFLOW = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()


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


assert '#include "wsd_stream_send_policy.h"' in SOURCE
sender = function_body(SOURCE, "static int wsd_send_msg(")
assert "if (ep->type == SOCK_STREAM)" in sender
assert "return wsd_stream_send_all(fd, msg, msglen, send) != 0;" in sender
assert "send(fd, msg, msglen" not in sender
assert "sendto(fd, msg, msglen, MSG_NOSIGNAL" in sender

for token in (
    "typedef ssize_t (*wsd_stream_send_fn)",
    "errno == EINTR",
    "if (sent == 0)",
    "errno = EPIPE;",
    "if ((size_t)sent > remaining)",
    "errno = EIO;",
    "remaining -= (size_t)sent;",
):
    assert token in POLICY, token

for test_name in (
    "test_short_writes_and_eintr_send_every_byte",
    "test_permanent_error_preserves_successful_prefix_and_errno",
    "test_zero_length_send_fails_instead_of_spinning",
    "test_impossible_send_count_is_rejected",
    "test_invalid_arguments_are_rejected",
):
    assert test_name in TEST, test_name
assert "-std=gnu89 -Wall -Wextra -Werror" in SCRIPT
assert "-fsanitize=address,undefined" in SCRIPT
assert "wsd_stream_send_policy_test.c" in SCRIPT
assert "wsd_stream_send_policy_test.c" in WORKFLOW
for path in (
    ".github/workflows/wsdd2-stream-send-regression.yml",
    "trunk/user/wsdd2/src/wsd.c",
    "trunk/user/wsdd2/src/wsd_stream_send_policy.h",
    "trunk/user/wsdd2/src/tests/wsd_stream_send_policy_test.c",
    "scripts/test_wsdd2_stream_send.sh",
    "scripts/check_wsdd2_stream_send.py",
):
    assert path in WORKFLOW, path
assert 'trunk/user/wsdd2/**' in XY_WORKFLOW
print("wsdd2 TCP send-all source contract: PASS")
