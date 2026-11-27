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
pair_transaction = function_body(helper, "dnsmasq_file_publish_pair_with_ops(")
set_section = helper[helper.index(
    "/* Publish up to three related outputs as one rollback-capable transaction."):]
set_publisher = function_body(set_section, "dnsmasq_file_publish_set_with_ops(")
pair_compact = " ".join(pair_publisher.split())
pair_transaction_compact = " ".join(pair_transaction.split())
set_compact = " ".join(set_publisher.split())
assert "dnsmasq_file_publish_pair_with_ops(" in pair_publisher
assert "dnsmasq_file_publish_internal" in pair_publisher
assert "destinations[0] = first_destination" in pair_transaction
assert "destinations[1] = second_destination" in pair_transaction
assert "backup_paths[0] = first_backup_path" in pair_transaction
assert "backup_paths[1] = second_backup_path" in pair_transaction
assert "staged_files, staged_paths, destinations, backup_paths, 2, mode" in pair_transaction_compact
assert "dnsmasq_file_backup_create(destinations[i], backup_paths[i])" in set_publisher
assert "publish_fn(staged_fp, staged_paths[i], destinations[i], mode," in set_publisher
failed_publish = set_compact.index("if (publish_fn(")
failed_rename_accounting = set_compact.index("if (rename_completed) published++;", failed_publish)
failed_rollback = set_compact.index("goto rollback;", failed_rename_accounting)
rollback_loop = set_compact.index("while (published > 0)")
rollback_decrement = set_compact.index("published--;", rollback_loop)
rollback_restore = set_compact.index(
    "dnsmasq_file_backup_restore(destinations[published], backup_paths[published])",
    rollback_decrement,
)
assert failed_publish < failed_rename_accounting < failed_rollback < rollback_loop
assert rollback_loop < rollback_decrement < rollback_restore
assert "dnsmasq_file_publish_set_with_ops(" in helper


assert "test_xupnpd_pair_preserves_modes_and_rolls_back" in test
assert "unlink(second_stage)" in test
assert "DNSMASQ_FILE_PRESERVE_MODE" in test
assert "not-committed-common\\n" in test and "rollback-common\\n" in test
assert "not-committed-runtime\\n" in test and "rollback-runtime\\n" in test
assert "(st.st_mode & 0777) == 0600" in test
assert "(st.st_mode & 0777) == 0640" in test
print("XUPnP generated configuration pair transaction contract: PASS")
