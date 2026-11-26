#!/usr/bin/env python3
"""Guard the XUPnP two-file config transaction against partial publication."""

import re
from pathlib import Path


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    quote = ""
    index = opening
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if state == "line-comment":
            if char == "\n":
                state = "code"
        elif state == "block-comment":
            if char == "*" and next_char == "/":
                state = "code"
                index += 1
        elif state == "quoted":
            if char == "\\":
                index += 1
            elif char == quote:
                state = "code"
        else:
            if char == "/" and next_char == "/":
                state = "line-comment"
                index += 1
            elif char == "/" and next_char == "*":
                state = "block-comment"
                index += 1
            elif char in ('"', "'"):
                quote = char
                state = "quoted"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[opening + 1:index]
        index += 1
    raise AssertionError(f"unterminated function: {signature}")


net = Path("trunk/user/rc/src/net.c").read_text()
helper = Path("trunk/user/rc/src/dnsmasq_file.h").read_text()
test = Path("trunk/user/rc/src/tests/dnsmasq_file.c").read_text()
body = function_body(net, "start_xupnpd(char *wan_ifname)")
compact = " ".join(body.split())

common_open = compact.index(
    "xupnpd_config_tempfile_open(common_path, common_stage, sizeof(common_stage))"
)
runtime_open = compact.index(
    "xupnpd_config_tempfile_open(runtime_path, runtime_stage, sizeof(runtime_stage))"
)
publish = compact.index("dnsmasq_file_publish_pair(")
assert common_open < runtime_open < publish
assert not re.search(r"\brename\s*\(", body)
assert "xupnpd_config_source_open(common_path)" in compact
assert "xupnpd_config_source_open(runtime_path)" in compact
assert compact.index("fsync(fileno(fp1))") < runtime_open
assert compact.index("fsync(fileno(fp3))") < publish
assert compact.index('"udpxy_enable_x"') < publish
assert compact.index('"xupnpd_udpxy"') < publish
assert "DNSMASQ_FILE_PRESERVE_MODE" in compact
assert "goto xupnpd_config_prepare_failed" in body
assert "dnsmasq_file_discard(common_stage)" in body
assert "dnsmasq_file_discard(runtime_stage)" in body
assert '#include "dnsmasq_file.h"' in net

assert "#define DNSMASQ_FILE_PRESERVE_MODE ((mode_t)-1)" in helper
publisher = function_body(helper, "dnsmasq_file_publish_internal(FILE *fp,")
assert "mode != DNSMASQ_FILE_PRESERVE_MODE" in publisher
pair_publisher = function_body(helper, "dnsmasq_file_publish_pair(FILE *first_fp,")
assert "dnsmasq_file_backup_create(first_destination, first_backup_path)" in pair_publisher
assert "dnsmasq_file_backup_create(second_destination, second_backup_path)" in pair_publisher
assert "dnsmasq_file_backup_restore(first_destination" in pair_publisher

assert "test_xupnpd_pair_preserves_modes_and_rolls_back" in test
assert "unlink(second_stage)" in test
assert "DNSMASQ_FILE_PRESERVE_MODE" in test
assert "not-committed-common\\n" in test and "rollback-common\\n" in test
assert "not-committed-runtime\\n" in test and "rollback-runtime\\n" in test
assert "(st.st_mode & 0777) == 0600" in test
assert "(st.st_mode & 0777) == 0640" in test
print("XUPnP generated configuration pair transaction contract: PASS")
