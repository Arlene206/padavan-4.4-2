#!/usr/bin/env python3
from pathlib import Path

net = Path("trunk/user/rc/src/net.c").read_text()
policy = Path("trunk/user/rc/src/nat_helper_settings_policy.h").read_text()
test = Path("trunk/user/rc/src/tests/nat_helper_settings_policy.c").read_text()
workflow = Path(".github/workflows/wan-route-status.yml").read_text()


def body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:position]
    raise AssertionError("unterminated function: " + signature)


reload = body(net, "reload_nat_modules(void)")
compact = "".join(reload.split())
all_net = "".join(net.split())
assert '#include "nat_helper_settings_policy.h"' in net
for token in (
    "nvram_transaction_begin((structnvram_transaction*)context)",
    "nvram_transaction_get((structnvram_transaction*)context,param_name,value)",
    "nvram_transaction_end((structnvram_transaction*)context)",
):
    assert token in all_net, token

snapshot_call = (
    "rc_nat_helper_settings_read(is_ap_mode,net_nvram_transaction_begin,"
    "net_nvram_transaction_get,net_nvram_transaction_end,"
    "&nat_nvram_transaction,&nat_settings)"
)
assert snapshot_call in compact
assert "if(nat_settings_status!=0)" in compact
assert compact.index("if(nat_settings_status!=0)") < compact.index('rc_module_unload_verified("hw_nat"')
assert 'nvram_get_int("wan_nat_x")' not in reload
for key in ("nf_alg_ftp0", "nf_alg_ftp1", "nf_alg_pptp", "nf_alg_h323", "nf_alg_sip"):
    assert f'nvram_get_int("{key}")' not in reload, key
    assert f'nvram_match("{key}"' not in reload, key

for marker in (
    "rc_wan_nvram_copy_snapshot(fields, field_count",
    "rc_wan_nvram_parse_int_snapshot(integer_texts",
    'strcmp(values[3], "1") == 0',
    'strcmp(values[4], "1") == 0',
    'strcmp(values[5], "1") == 0',
):
    assert marker in policy, marker

for case in (
    "test_router_mode_reads_one_coherent_nat_helper_tuple",
    "test_missing_keys_keep_legacy_disabled_defaults_and_port_ranges",
    "test_each_snapshot_read_error_preserves_current_module_decisions",
    "test_transaction_close_error_preserves_current_module_decisions",
    "test_invalid_integer_and_oversized_value_do_not_publish_settings",
    "test_ap_mode_reads_only_wan_nat_selector",
    "NAT helper module setting snapshot: PASS",
):
    assert case in test, case

paths = (
    ".github/workflows/wan-route-status.yml",
    "scripts/check_nat_helper_settings.py",
    "trunk/user/rc/src/net.c",
    "trunk/user/rc/src/nat_helper_settings_policy.h",
    "trunk/user/rc/src/tests/nat_helper_settings_policy.c",
)
filters = workflow.split("jobs:", 1)[0]
push, pr = filters.split("  pull_request:", 1)
for path in paths:
    assert push.count('"' + path + '"') == 1, path
    assert pr.count('"' + path + '"') == 1, path

print("NAT helper reload checked-snapshot and failure-preservation contract: PASS")
