#!/usr/bin/env python3
from pathlib import Path

net = Path("trunk/user/rc/src/net.c").read_text()
policy = Path("trunk/user/rc/src/nat_helper_settings_policy.h").read_text()
test = Path("trunk/user/rc/src/tests/nat_helper_settings_policy.c").read_text()
workflow = Path(".github/workflows/wan-route-status.yml").read_text()
offload_policy = Path("trunk/user/rc/src/nat_offload_settings_policy.h").read_text()
offload_test = Path("trunk/user/rc/src/tests/nat_offload_settings_policy.c").read_text()


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
    "rc_nat_helper_parse_boolean_value(const char *text, int *value)",
    "if (parsed[0] != 0 && parsed[0] != 1)",
    "rc_nat_helper_parse_boolean_value(values[3], &pptp_enabled)",
    "rc_nat_helper_parse_boolean_value(values[4], &h323_enabled)",
    "rc_nat_helper_parse_boolean_value(values[5], &sip_enabled)",
):
    assert marker in policy, marker

for case in (
    "test_router_mode_reads_one_coherent_nat_helper_tuple",
    "test_missing_keys_keep_legacy_disabled_defaults_and_port_ranges",
    "test_each_snapshot_read_error_preserves_current_module_decisions",
    "test_transaction_close_error_preserves_current_module_decisions",
    "test_invalid_integer_and_oversized_value_do_not_publish_settings",
    "test_boolean_selectors_reject_unknown_values_without_publishing",
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


offload = body(net, "reload_nat_modules(void)")
offload_compact = "".join(offload.split())
assert '#include "nat_offload_settings_policy.h"' in net
assert "rc_nat_offload_settings_read(" in offload_compact
assert offload_compact.index("if(hwnat_settings_status!=0)") < offload_compact.index('rc_module_unload_verified("hw_nat"')
assert "hwnat_allow=nat_offload_settings.hwnat_allowed;" in offload_compact
assert "hw_nat_mode=nat_offload_settings.hw_nat_mode;" in offload_compact
assert "ipv6_nat=nat_offload_settings.ipv6_nat;" in offload_compact
assert "sfe_enable=nat_offload_settings.sfe_enabled;" in offload_compact
for key in ("sw_mode", "hw_nat_mode", "ip6_lan_auto", "sfe_enable"):
    assert f'nvram_get_int("{key}")' not in offload
for marker in (
    '"sw_mode"',
    '"hw_nat_mode"',
    '"ip6_lan_auto"',
    '"sfe_enable"',
    "rc_wan_nvram_copy_snapshot(fields, count",
    "rc_wan_nvram_parse_int_snapshot(integer_texts, count, parsed)",
    "if (read_hwnat && parsed[ipv6_nat_index] != 0 &&",
    "parsed[ipv6_nat_index] != 1)",
):
    assert marker in offload_policy, marker
for case in (
    "test_hwnat_and_sfe_selectors_share_one_snapshot",
    "test_hwnat_enable_predicate_preserves_legacy_mode_rules",
    "test_missing_values_keep_legacy_zero_defaults",
    "test_every_snapshot_read_error_preserves_previous_settings",
    "test_transaction_failure_and_invalid_values_do_not_publish_settings",
    "test_invalid_ipv6_lan_auto_values_preserve_previous_settings",
    "test_selective_reads_and_no_feature_defaults",
    "NAT offload setting snapshot: PASS",
):
    assert case in offload_test, case

print("NAT helper reload checked-snapshot and failure-preservation contract: PASS")
