#!/usr/bin/env python3
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
helper = (root / "trunk/user/smartdns/smartdns_dnsmasq_config.sh").read_text()
smartdns = (root / "trunk/user/smartdns/smartdns.sh").read_text()
autostart = (root / "trunk/user/scripts/files/usr/bin/autostart.sh").read_text()
makefile = (root / "trunk/user/smartdns/Makefile").read_text()
test = (root / "trunk/user/smartdns/tests/dnsmasq_config_preservation.sh").read_text()
workflow = (root / ".github/workflows/dnsmasq-lifecycle.yml").read_text()
xy = (root / ".github/workflows/xy-c1-firmware-validation.yml").read_text()

def body(source, name):
    pattern = re.compile(r"(?m)^" + re.escape(name) + r"\s*\(\)\s*\{")
    matches = list(pattern.finditer(source))
    assert len(matches) == 1, "expected one function definition: " + name
    start = matches[0].start()
    body_start = matches[0].end()
    pos = body_start
    depth, quote, escaped = 1, None, False
    while pos < len(source):
        ch = source[pos]
        if quote:
            if escaped: escaped = False
            elif ch == "\\": escaped = True
            elif ch == quote: quote = None
        else:
            if ch in ("'", '"'): quote = ch
            elif ch == "{": depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0: return source[body_start:pos]
        pos += 1
    raise AssertionError("unterminated " + name)

normal = body(helper, "smartdns_dnsmasq_normalize_port")
remove = body(helper, "smartdns_dnsmasq_remove_exact_directives")
publish = body(helper, "smartdns_dnsmasq_publish_managed_block")
block = body(helper, "smartdns_dnsmasq_remove_managed_block")
saved = body(helper, "smartdns_dnsmasq_remove_saved_directives")
assert '[ "${#smartdns_port_value}" -le 5 ]' in normal
assert '[ "$smartdns_port_value" -le 65535 ]' in normal
for token in ('[ ! -L "$smartdns_config_file" ]',
              'mktemp "${smartdns_config_file}.tmp.XXXXXX"',
              'cp -p "$smartdns_config_file"', "cmp -s",
              'mv -f "$smartdns_tmp_file" "$smartdns_config_file"'):
    assert token in remove, token
assert 'sub(/^[[:space:]]*/, "", line)' in remove
assert 'sub(/[[:space:]]*$/, "", line)' in remove
assert 'END { print found ? 1 : 0 }' in remove
assert 'END { exit(found ? 0 : 1) }' not in remove
assert 'smartdns_scan_result=$(awk' in remove
assert 'END { print found ? 1 : 0 }' in publish
assert 'END { exit(found ? 1 : 0) }' not in publish
assert 'smartdns_scan_result=$(awk' in publish
assert 'smartdns_block_status=$(awk' in block
assert 'else if (!seen_begin) print "absent"' in block
assert 'exit 2' not in block
for token in ('mktemp "${smartdns_config_file}.tmp.XXXXXX"',
              'cp -p "$smartdns_config_file"',
              "smartdns-managed-dnsmasq-begin",
              "smartdns-managed-dnsmasq-end",
              'mv -f "$smartdns_tmp_file" "$smartdns_config_file"'):
    assert token in publish, "atomic publication missing: " + token
assert "for smartdns_directive do" in publish
assert '# smartdns-managed-dnsmasq-begin' in block
assert '# smartdns-managed-dnsmasq-end' in block
assert 'if (bad || inside || (seen_begin != seen_end)) print "invalid"' in block
assert 'exit 2' not in block
assert "managed-v1" in saved and "legacy" in saved and "smartdns_dnsmasq_remove_managed_block" in saved
assert "sed " not in helper and "eval " not in helper

read = body(smartdns, "Read_ini")
assert '[ -L "$smartdns_Ini" ]' in read
assert 'sed -n \'5p\' "$smartdns_Ini"' in read
assert "smartdns_dnsmasq_normalize_port" in read and 'case "$sdns_redirected"' in read
assert 'case "$smartdns_state_version" in' in read
assert 'case "$action" in' in read
assert 'start|restart|reset)' in read
assert read.index('case "$action" in') < read.index('smartdns_dnsmasq_normalize_port "$sdns_port"')
assert 'if Read_ini; then' in smartdns and "preserving DNSMasq configuration" in smartdns
assert '. "$smartdns_dnsmasq_config_lib"' in smartdns
assert 'smartdns_dnsmasq_config_lib="/usr/bin/smartdns_dnsmasq_config.sh"' in smartdns
assert '"$smartdns_state_version"' in smartdns

change = body(smartdns, "Change_dnsmasq")
assert "smartdns_dnsmasq_remove_saved_directives" in change
assert "sed -i" not in change
assert "server=127.0.0.1#$sdns_port" in change
assert 'smartdns_dnsmasq_publish_managed_block "$@"' in change
assert '>> "$dnsmasq_Conf"' not in change
start = body(smartdns, "Start_smartdns")
clean_at = start.find("smartdns_dnsmasq_remove_saved_directives")
write_at = start.find("smartdns_write_ini_state")
publish_at = start.find("Change_dnsmasq start")
assert clean_at >= 0 and write_at > clean_at and publish_at > write_at
assert 'sdns_ported="$sdns_port"' in start and 'sdns_redirected="$sdns_redirect"' in start
assert "smartdns_state_version=managed-v1" in start
stop = body(smartdns, "Stop_smartdns")
assert "Change_dnsmasq stop" in stop and "DNSMasq resolver cleanup incomplete" in stop
assert 'return "$smartdns_dnsmasq_status"' in stop

assert 'smartdns_dnsmasq_config.sh $(ROMFSDIR)/usr/bin/smartdns_dnsmasq_config.sh' in makefile
for forbidden in ("dnsmasq_Conf", "smartdns_Ini", "sed -i '/no-resolv/d'", "sed -i '/port=0/d'"):
    assert forbidden not in autostart, forbidden
for token in ("managed-block publication is atomic and preserves same-valued user directives",
              "duplicate or interrupted managed blocks fail closed",
              "marked cleanup preserves user-owned exact directives",
              "port 53 marker cleanup leaves same-valued user directives intact",
              "upgrade cleanup removes only exact legacy directives",
              "malformed marker block fails closed", "symlink configuration is refused",
              "missing configuration is a safe no-op",
              "all DNSMasq preliminary AWK scan failures propagate without changing live config",
              "rename failure preserves original config",
              "failed managed-block publication leaves original config intact",
              "SmartDNS DNSMasq configuration preservation regression: PASS"):
    assert token in test, token

paths = (
    "trunk/user/smartdns/smartdns.sh",
    "trunk/user/smartdns/smartdns_dnsmasq_config.sh",
    "trunk/user/smartdns/smartdns_config_file.sh",
    "trunk/user/smartdns/tests/smartdns_config_file.sh",
    "trunk/user/smartdns/tests/dnsmasq_config_preservation.sh",
    "trunk/user/smartdns/Makefile",
    "trunk/user/scripts/files/usr/bin/autostart.sh",
    "scripts/check_smartdns_dnsmasq_config.py",
)
push, pull = workflow.split("  pull_request:", 1)
pullpaths, _ = pull.split("  workflow_dispatch:", 1)
for path in paths:
    line = '      - "' + path + '"'
    assert push.count(line) == 1, "push path " + path
    assert pullpaths.count(line) == 1, "PR path " + path
assert "trunk/user/smartdns/**" in xy
assert "dnsmasq_config_preservation.sh" in workflow
assert "check_smartdns_dnsmasq_config.py" in workflow
config_file = (root / "trunk/user/smartdns/smartdns_config_file.sh").read_text()
config_file_test = (root / "trunk/user/smartdns/tests/smartdns_config_file.sh").read_text()
config_publish = body(config_file, "smartdns_config_publish_deduplicated")
for token in ('[ ! -f "$_smartdns_source" ]',
              '[ -L "$_smartdns_destination" ]',
              'mktemp "${_smartdns_destination}.tmp.XXXXXX"',
              'awk \'!seen[$0]++\' "$_smartdns_source" > "$_smartdns_stage"',
              'chmod 0644 "$_smartdns_stage"',
              'mv -f "$_smartdns_stage" "$_smartdns_destination"'):
    assert token in config_publish, token
assert '. "$smartdns_config_file_lib"' in smartdns
assert 'smartdns_config_file_lib="/usr/bin/smartdns_config_file.sh"' in smartdns
assert 'smartdns_tmp_Conf=$(mktemp "${smartdns_Conf}.input.XXXXXX")' in start
assert 'smartdns_config_publish_deduplicated "$smartdns_tmp_Conf" "$smartdns_Conf"' in start
assert 'awk \'!x[$0]++\' "$smartdns_tmp_Conf" > "$smartdns_Conf"' not in start
assert 'Stop_smartdns' in start and '配置发布失败后的 DNSMasq 回滚未完成' in start
assert '$(INSTALL_BIN) ./smartdns_config_file.sh $(ROMFSDIR)/usr/bin/smartdns_config_file.sh' in makefile
for token in ("generator failure preserves last-good config",
              "rename failure preserves last-good config",
              "symlink, special-file and missing-source targets fail closed",
              "SmartDNS atomic configuration publication regression: PASS"):
    assert token in config_file_test, token
rollback = body(smartdns, "smartdns_start_config_rollback")
assert "nvram set sdns_enable=0" in rollback
assert "Stop_smartdns" in rollback
assert "DNSMasq 回滚未完成" in rollback
assert start.count("smartdns_start_config_rollback") == 5
print("SmartDNS saved DNSMasq state, marked-block and atomic SmartDNS-config publication contracts: PASS")
