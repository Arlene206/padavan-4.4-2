#!/usr/bin/env python3
"""Ensure WSD responses do not reflect raw request MessageID markup."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/wsdd2/src/wsd.c").read_text()
POLICY = (ROOT / "trunk/user/wsdd2/src/wsd_xml_text_policy.h").read_text()
REQUEST_POLICY = (ROOT / "trunk/user/wsdd2/src/wsd_xml_request_policy.h").read_text()
REQUEST_TEST = (ROOT / "trunk/user/wsdd2/src/tests/wsd_xml_request_policy_test.c").read_text()
WORKFLOW = (ROOT / ".github/workflows/wsdd2-xml-reflection-regression.yml").read_text()
XY_WORKFLOW = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()

assert '#include "wsd_xml_text_policy.h"' in SOURCE
assert '#include "wsd_xml_request_policy.h"' in SOURCE
assert "wsd_tag_find(xml, length, action_tag, &action_len)" in SOURCE
assert "wsd_tag_find(xml, length, msgid_tag, &msgid_len)" in SOURCE
assert "strstr(xml, tag[i][0])" not in SOURCE
assert "wsd_xml_find_simple_text(xml, length, tag, tag_count, len)" in SOURCE
assert "Embedded NUL in XML body" in SOURCE
assert "len = (ssize_t)strlen(buf);" in SOURCE
assert "wsd_xml_find_sequence" in REQUEST_POLICY
assert '"<!--"' in REQUEST_POLICY
assert '"<![CDATA["' in REQUEST_POLICY
assert '"?>"' in REQUEST_POLICY
assert "memchr(xml," in REQUEST_POLICY
for token in (
    "test_ignores_tag_lookalikes_in_comments_and_cdata",
    "test_ignores_tag_lookalikes_in_quoted_attributes_and_processing_instructions",
    "test_rejects_duplicate_empty_and_nested_elements",
    "test_bounded_input_and_malformed_markup",
):
    assert token in REQUEST_TEST
start = SOURCE.index("static int wsd_send_soap_msg(")
end = SOURCE.index("static int wsd_send_hello(", start)
sender = SOURCE[start:end]
assert "escaped_relates = wsd_xml_escape_text(relates)" in sender
assert sender.index("wsd_xml_escape_text(relates)") < sender.index("asprintf(&soap_relates")
assert "free(escaped_relates);" in sender
assert '"<wsa:RelatesTo>%s</wsa:RelatesTo>"' in sender
assert "wsd_xml_text_is_valid_xml10_utf8(" in POLICY
assert "errno = EILSEQ;" in POLICY
assert "errno = EINVAL;" in POLICY
assert "first >= 0xc2 && first <= 0xdf" in POLICY
assert "first >= 0xe0 && first <= 0xef" in POLICY
assert "first >= 0xf0 && first <= 0xf4" in POLICY
assert "codepoint < 0x800" in POLICY
assert "codepoint >= 0xd800 && codepoint <= 0xdfff" in POLICY
assert "codepoint > 0x10ffff" in POLICY
assert "codepoint >= 0xe000 && codepoint <= 0xfffd" in POLICY
assert "if (wsd_xml_text_is_valid_xml10_utf8(" in POLICY
assert POLICY.index("if (wsd_xml_text_is_valid_xml10_utf8(") < POLICY.index("escaped = malloc(capacity)")
for marker in (
    'case \'&\':',
    'case \'<\':',
    'case \'>\':',
    'case \'"\':',
    "case '\\'':",
    '"&amp;"',
    '"&lt;"',
    '"&gt;"',
    '"&quot;"',
    '"&apos;"',
):
    assert marker in POLICY, marker
assert "wsd_xml_text_policy_test.c" in WORKFLOW
assert "wsd_xml_request_policy.h" in WORKFLOW
assert "wsd_xml_request_policy_test.c" in WORKFLOW
assert 'trunk/user/wsdd2/**' in XY_WORKFLOW

get_start = SOURCE.index("static int wsd_send_get_response(")
get_end = SOURCE.index("\n/*\n * Validate HTTP POST", get_start)
get_response = SOURCE[get_start:get_end]
metadata_keys = ("vendor:", "vendorurl:", "model:", "modelurl:", "presentationurl:")
for index, key in enumerate(metadata_keys):
    assert f'raw_metadata[{index}] = get_getresp("{key}")' in get_response
assert "raw_metadata[5] = netbiosname;" in get_response
assert "raw_metadata[6] = workgroup;" in get_response
assert "escaped[i] = wsd_xml_escape_text(raw_metadata[i])" in get_response
assert get_response.index("wsd_xml_escape_text(raw_metadata[i])") < get_response.index("asprintf(&body")
assert "escaped[0]" in get_response and "escaped[6]" in get_response
assert "free(escaped[i]);" in get_response

print("wsdd2 WSD RelatesTo and metadata XML escaping source contract: PASS")
