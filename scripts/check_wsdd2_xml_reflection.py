#!/usr/bin/env python3
"""Ensure WSD responses do not reflect raw request MessageID markup."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/wsdd2/src/wsd.c").read_text()
POLICY = (ROOT / "trunk/user/wsdd2/src/wsd_xml_text_policy.h").read_text()
WORKFLOW = (ROOT / ".github/workflows/wsdd2-xml-reflection-regression.yml").read_text()
XY_WORKFLOW = (ROOT / ".github/workflows/xy-c1-firmware-validation.yml").read_text()

assert '#include "wsd_xml_text_policy.h"' in SOURCE
start = SOURCE.index("static int wsd_send_soap_msg(")
end = SOURCE.index("static int wsd_send_hello(", start)
sender = SOURCE[start:end]
assert "escaped_relates = wsd_xml_escape_text(relates)" in sender
assert sender.index("wsd_xml_escape_text(relates)") < sender.index("asprintf(&soap_relates")
assert "free(escaped_relates);" in sender
assert '"<wsa:RelatesTo>%s</wsa:RelatesTo>"' in sender
assert "if (ch < 0x20 && ch != '\\t' && ch != '\\n' && ch != '\\r')" in POLICY
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
