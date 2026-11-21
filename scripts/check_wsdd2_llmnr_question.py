#!/usr/bin/env python3
"""Enforce bounded LLMNR question parsing in wsdd2."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/wsdd2/src/llmnr.c").read_text()
POLICY = (ROOT / "trunk/user/wsdd2/src/llmnr_question_policy.h").read_text()
WORKFLOW = (ROOT / ".github/workflows/wsdd2-llmnr-question-regression.yml").read_text()
XY_WORKFLOW = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()


def compact(text):
    return re.sub(r"\\s+", " ", text).strip()


assert '#include "llmnr_question_policy.h"' in SOURCE
assert "if (inlen < 17)" in SOURCE
assert "llmnr_parse_question(in, inlen, in_name, sizeof(in_name)," in compact(SOURCE)
start = SOURCE.index("static int llmnr_send_response(")
end = SOURCE.index("int llmnr_init(", start)
parser = SOURCE[start:end]
for forbidden in ("in_name_p", "in_label", "realloc(in_name", "strcat(in_name", "free(in_name)"):
    assert forbidden not in parser, forbidden

policy = compact(POLICY)
for token in (
    "packet_len < 17",
    "label_len > 63",
    "(size_t)(end - cursor) < label_len",
    "wire_len >= 255",
    "(size_t)(end - cursor) < 4",
    "separator_len + label_len >= name_capacity - name_len",
):
    assert token in policy, token

for path in (
    "trunk/user/wsdd2/src/llmnr.c",
    "trunk/user/wsdd2/src/llmnr_question_policy.h",
    "trunk/user/wsdd2/src/tests/llmnr_question_policy_test.c",
    "scripts/test_wsdd2_llmnr_question.sh",
    "scripts/check_wsdd2_llmnr_question.py",
):
    assert path in WORKFLOW, path
assert 'trunk/user/wsdd2/**' in XY_WORKFLOW
print("wsdd2 bounded LLMNR question source contract: PASS")
