#!/usr/bin/env python3
"""Guard atomic MiniUPnP lease persistence and mapping rollback contracts."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/upnpredirect.c").read_text()
POLICY = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/miniupnpd_lease_file_policy.h").read_text()
TEST = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/tests/miniupnpd_lease_file_policy_test.c").read_text()


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


lease_definition = "lease_file_remove(unsigned short eport, int proto)"
lease_definition_at = SOURCE.index(lease_definition)
assert SOURCE[:lease_definition_at].endswith("static int\n")
assert "static int\nstatic int\nlease_file_rewrite_entry" not in SOURCE

lease_add = function_body(SOURCE, "lease_file_add(unsigned short eport,")
remove = function_body(SOURCE, lease_definition)
rewrite_entry = function_body(
    SOURCE,
    "lease_file_rewrite_entry(unsigned short eport, int proto,"
)
replace = function_body(
    SOURCE,
    "lease_file_replace(unsigned short eport, const char *iaddr,"
)
delete = function_body(SOURCE, "_upnp_delete_redir(unsigned short eport, int proto)")
redirect_internal = function_body(SOURCE, "upnp_redirect_internal(const char * rhost")
update = function_body(SOURCE, "upnp_redirect(const char * rhost")

# Appending a new mapping is rolled back to its original EOF on a partial write.
assert "miniupnpd_lease_append_transaction(" in lease_add
assert "O_WRONLY | O_APPEND | O_CREAT" in lease_add
assert "fwrite(" not in lease_add
for marker in ("ops->truncate(fd, original_size)", "ops->sync(fd)",
               "errno = saved_errno"):
    assert marker in POLICY, marker
for marker in ("test_lease_append_rolls_back_partial_write",
               "partial_then_enospc_write", "rollback_failed == 0"):
    assert marker in TEST, marker

# Shared rewrite removes every exact matching key and preserves other keys.
assert "lease_file_rewrite_entry(eport, proto, NULL, 0)" in remove
assert "lease_file_rewrite_entry(eport, proto, record" in replace
assert "miniupnpd_lease_file_rewrite(" in rewrite_entry
assert "miniupnpd_lease_record_key_matches(" in POLICY
for marker in ("mkstemp(tmpfilename)", "fchmod(tmp", "fclose(fd)",
               "fflush(fdt)", "fsync(fileno(fdt))",
               "rename(tmpfilename, lease_file)", "tmp_created"):
    assert marker in rewrite_entry, marker
assert "errno == ENOENT" in rewrite_entry
assert "ferror(source)" in POLICY
assert "miniupnpd_lease_file_rewrite(FILE *source, FILE *destination," in POLICY

for marker in ("test_exact_record_key_matching", "test_atomic_rewrite_replaces_all_matching_keys",
               "test_atomic_rewrite_remove_preserves_other_keys",
               "test_atomic_rewrite_creates_missing_file_content",
               "test_atomic_rewrite_rejects_invalid_arguments"):
    assert marker in TEST, marker

# New-rule persistence precedes filter publication and compensates firewall on failure.
assert "lease_file_add(eport, iaddr, iport, proto, desc, timestamp) < 0" in redirect_internal
assert redirect_internal.index("lease_file_add(") < redirect_internal.index("add_filter_rule2(")
assert "_upnp_delete_redir(eport, proto)" in redirect_internal

# Existing-rule update replaces its lease in one rename and rolls back live state on error.
assert "desc_old, sizeof(desc_old)" in update
assert "timestamp_old = timestamp" in update
assert update.index("update_portmapping(ext_if_name, eport, proto, iport") < update.index(
    "lease_file_replace(eport, iaddr, iport, proto, desc, timestamp)"
)
assert "lease_file_remove(eport, proto)" not in update
assert "lease_file_add(eport, iaddr, iport," not in update
assert "update_portmapping(" in update and "iport_old" in update
assert "update_portmapping_desc_timestamp(" in update
assert "saved_errno" in update and "errno = saved_errno" in update

# Delete lease cleanup remains checked; its cross-table status semantics are a separate task.
assert "lease_file_remove(eport, proto) < 0" in delete
assert "lease_status != 0" in delete
if "upnp_event_var_change_notify" in delete:
    assert delete.index("lease_file_remove(eport, proto) < 0") < delete.index(
        "upnp_event_var_change_notify"
    )

print("MiniUPnP atomic lease persistence and mapping rollback source contract: PASS")
