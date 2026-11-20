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


add = function_body(SOURCE, "lease_file_add(unsigned short eport,")
remove = function_body(SOURCE, "lease_file_remove(unsigned short eport, int proto)")
delete = function_body(SOURCE, "_upnp_delete_redir(unsigned short eport, int proto)")
add = function_body(SOURCE, "upnp_redirect_internal(const char * rhost")
update = function_body(SOURCE, "upnp_redirect(const char * rhost")

assert "miniupnpd_lease_record_key_matches(" in remove
assert "miniupnpd_lease_append_transaction(" in add
assert "O_WRONLY | O_APPEND | O_CREAT" in add
assert "fwrite(" not in add
for marker in (
    "ops->truncate(fd, original_size)",
    "ops->sync(fd)",
    "errno = saved_errno",
):
    assert marker in POLICY, marker
for marker in (
    "test_lease_append_rolls_back_partial_write",
    "partial_then_enospc_write",
    "rollback_failed == 0",
):
    assert marker in TEST, marker
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

rewrite_entry = function_body(
    SOURCE,
    "lease_file_rewrite_entry(unsigned short eport, int proto,"
)
replace = function_body(
    SOURCE,
    "lease_file_replace(unsigned short eport, const char *iaddr,"
)
assert "miniupnpd_lease_file_rewrite(" in rewrite_entry
for marker in ("mkstemp(tmpfilename)", "fchmod(tmp", "fflush(fdt)",
               "fsync(fileno(fdt))", "rename(tmpfilename, lease_file)"):
    assert marker in rewrite_entry, marker
assert "miniupnpd_lease_file_rewrite(FILE *source, FILE *destination," in POLICY
for marker in ("test_atomic_rewrite_replaces_all_matching_keys",
               "test_atomic_rewrite_remove_preserves_other_keys",
               "test_atomic_rewrite_creates_missing_file_content"):
    assert marker in TEST, marker
assert "lease_file_replace(eport, iaddr, iport, proto, desc, timestamp)" in update
assert "desc_old, sizeof(desc_old)" in update
assert "timestamp_old = timestamp" in update
assert update.index("update_portmapping(ext_if_name, eport, proto, iport") < update.index(
    "lease_file_replace(eport, iaddr, iport, proto, desc, timestamp)"
)
assert "update_portmapping(\n\t\t\t\t\t\text_if_name, eport, proto, iport_old" in update
assert "update_portmapping_desc_timestamp(" in update
assert "lease_file_remove(eport, proto)" not in update
assert "lease_file_add(eport, iaddr, iport," not in update

print("MiniUPnP lease-file deletion and persistence failure contract: PASS")
