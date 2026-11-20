#!/usr/bin/env python3
"""Guard exact lease-key deletion and fail-closed lease-file persistence cleanup."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/upnpredirect.c").read_text()
POLICY = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/miniupnpd_lease_file_policy.h").read_text()
TEST = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/tests/miniupnpd_lease_file_policy_test.c").read_text()

lease_definition = "lease_file_remove(unsigned short eport, int proto)"
lease_definition_at = SOURCE.index(lease_definition)
lease_declaration_prefix = SOURCE[:lease_definition_at]
assert lease_declaration_prefix.endswith("static int\n"), (
    "lease_file_remove must be declared static int"
)
assert not lease_declaration_prefix.endswith("static int\nstatic int\n"), (
    "lease_file_remove has a duplicated static int specifier"
)


def function_body(source, signature):
    position = source.index(signature)
    while True:
        brace = source.find("{", position)
        semicolon = source.find(";", position)
        assert brace >= 0, "function definition missing: " + signature
        if semicolon >= 0 and semicolon < brace:
            position = source.find(signature, position + len(signature))
            assert position >= 0, "function definition missing: " + signature
            continue
        depth = 0
        for index in range(brace, len(source)):
            if source[index] == "{":
                depth += 1
            elif source[index] == "}":
                depth -= 1
                if depth == 0:
                    return source[position:index + 1]
        raise AssertionError("unterminated function: " + signature)


remove = function_body(SOURCE, "lease_file_remove(unsigned short eport, int proto)")
delete = function_body(SOURCE, "_upnp_delete_redir(unsigned short eport, int proto)")
add = function_body(SOURCE, "upnp_redirect_internal(const char * rhost")
update = function_body(SOURCE, "upnp_redirect(const char * rhost")

assert "miniupnpd_lease_record_key_matches(" in remove
assert "errno == ENOENT" in remove
for marker in (
    "ferror(fd)",
    "fwrite(buf, 1, buf_size, fdt)",
    "fflush(fdt)",
    "fsync(fileno(fdt))",
    "fclose(fdt)",
    "fclose(fd)",
    "rename(tmpfilename, lease_file)",
    "status != 0",
    "errno = status",
):
    assert marker in remove, marker

assert "record[key_length] == ':'" in POLICY
assert "memcmp(record, key, key_length)" in POLICY
for marker in (
    "test_exact_record_key_matching",
    "longer_port",
    "different_protocol",
    "missing_separator",
):
    assert marker in TEST, marker

assert "lease_file_remove(eport, proto) < 0" in delete
assert "lease_status != 0" in delete
if "upnp_event_var_change_notify" in delete:
    assert delete.index("lease_file_remove(eport, proto) < 0") < delete.index(
        "upnp_event_var_change_notify"
    )

assert "lease_file_add(eport, iaddr, iport, proto, desc, timestamp) < 0" in add
assert add.index("lease_file_add(") < add.index("add_filter_rule2(")
assert "_upnp_delete_redir(eport, proto)" in add

assert "lease_file_remove(eport, proto)" in update
assert "lease_file_add(eport, iaddr, iport," in update
assert update.index("lease_file_remove(eport, proto)") < update.index(
    "lease_file_add(eport, iaddr, iport,"
)
assert "lease_status < 0" in update

print("MiniUPnP lease-file deletion and persistence failure contract: PASS")
