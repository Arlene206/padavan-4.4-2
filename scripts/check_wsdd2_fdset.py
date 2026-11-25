#!/usr/bin/env python3
"""Keep WSDD2 select registration inside fd_set's representable range."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/wsdd2/src/wsdd2.c").read_text()
POLICY = (ROOT / "trunk/user/wsdd2/src/wsd_select_fd_policy.h").read_text()
TEST = (ROOT / "trunk/user/wsdd2/src/tests/wsd_select_fd_policy_test.c").read_text()
RUNNER = (ROOT / "scripts/test_wsdd2_fdset.sh").read_text()
WORKFLOW = (ROOT / ".github/workflows/wsdd2-fdset-regression.yml").read_text()
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


assert '#include "wsd_select_fd_policy.h"' in SOURCE
assert "FD_SET(" not in SOURCE
assert SOURCE.count("wsd_select_fd_add(&fds, ep->sock, &nfds)") == 2
assert SOURCE.count("socket descriptor exceeds select capacity") == 2
main = function_body(SOURCE, "int main(")
assert main.count("wsd_select_fd_add(&fds, ep->sock, &nfds)") == 2
assert main.count("close_ep(ep);") >= 2
assert main.count("badep = ep;") >= 2

policy = compact(POLICY)
for token in (
    "fd < 0",
    "fd >= FD_SETSIZE",
    "errno = EMFILE",
    "FD_SET(fd, set)",
    "if (*maxfd < fd)",
):
    assert token in policy, token

for token in (
    "test_valid_fd_boundaries_are_registered",
    "test_out_of_range_fd_is_rejected_before_fd_set",
    "test_invalid_arguments_are_rejected",
    "FD_SETSIZE",
    "INT_MAX",
    "canary[i] == 0xa5",
):
    assert token in TEST, token
assert "-std=gnu89 -Wall -Wextra -Werror" in RUNNER
assert "-fsanitize=address,undefined" in RUNNER
assert "wsd_select_fd_policy_test.c" in RUNNER
for path in (
    ".github/workflows/wsdd2-fdset-regression.yml",
    "trunk/user/wsdd2/src/wsdd2.c",
    "trunk/user/wsdd2/src/wsd_select_fd_policy.h",
    "trunk/user/wsdd2/src/tests/wsd_select_fd_policy_test.c",
    "scripts/test_wsdd2_fdset.sh",
    "scripts/check_wsdd2_fdset.py",
):
    assert path in WORKFLOW, path
assert 'trunk/user/wsdd2/**' in XY_WORKFLOW
print("wsdd2 select descriptor range source contract: PASS")
