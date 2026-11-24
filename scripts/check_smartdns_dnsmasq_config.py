#!/usr/bin/env python3
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
helper = (root / "trunk/user/smartdns/smartdns_dnsmasq_config.sh").read_text()
smartdns = (root / "trunk/user/smartdns/smartdns.sh").read_text()
enable_test = (root / "trunk/user/smartdns/tests/smartdns_enable_selector.sh").read_text()
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

enable_validator = body(smartdns, "smartdns_validate_enable_selector")
assert '[ "$#" -eq 1 ] || return 2' in enable_validator
assert "''|0|1)" in enable_validator
assert 'Invalid sdns_enable selector; refusing to change service state' in enable_validator
assert 'return 22' in enable_validator
lifecycle_nvram = body(smartdns, "smartdns_load_lifecycle_nvram_state")
assert 'if [ "$action" = "stop" ]; then' in lifecycle_nvram
stop_nvram_branch = lifecycle_nvram.split('if [ "$action" = "stop" ]; then', 1)[1].split("fi", 1)[0]
for assignment in ("sdns_enable=0", "sdns_port=", "sdns_redirect=",
                   "sdnse_enable=", "sdnse_port="):
    assert assignment in stop_nvram_branch, assignment
assert "return 0" in stop_nvram_branch
for key in ("sdns_enable", "sdns_port", "sdns_redirect", "sdnse_enable", "sdnse_port"):
    assert "smartdns_nvram_read_checked " + key + " || return $?" in lifecycle_nvram
assert 'smartdns_validate_enable_selector "$sdns_enable" || return $?' in lifecycle_nvram
assert 'start|restart|reset)' in lifecycle_nvram
runtime_nvram = body(smartdns, "smartdns_load_runtime_nvram_state")
runtime_keys = (
    "sdns_name", "sdns_tcp_server", "sdns_ipv6_server", "sdns_cache",
    "sdns_cache_persist", "sdns_tcp_idle_time", "sdns_rr_ttl",
    "sdns_rr_ttl_min", "sdns_rr_ttl_max", "sdns_rr_ttl_reply_max",
    "sdns_max_reply_ip_num", "sdns_speed", "sdns_speed_mode",
    "sdns_address", "sdns_ipset", "sdns_ipset_timeout", "sdns_as",
    "sdns_ip_change", "sdns_ip_change_time",
    "sdns_dualstack_ip_allow_force_aaaa", "sdns_force_aaaa_soa",
    "sdns_force_qtype_soa", "sdns_prefetch_domain", "sdns_exp",
    "sdns_exp_ttl", "sdns_exp_ttl_max", "sdns_exp_prefetch_time",
    "sdnse_tcp", "sdnse_speed", "sdnse_name", "sdnse_address",
    "sdnse_ns", "sdns_ns", "sdnse_ipset", "sdnse_as",
    "sdnse_ipv6_server", "sdnse_ipc", "sdnse_cache", "sdns_adblock",
    "sdns_white", "sdns_black", "sdns_coredump",
)
assert "smartdns_load_lifecycle_nvram_state || return $?" in runtime_nvram
for key in runtime_keys:
    assert "smartdns_nvram_read_checked " + key + " || return $?" in runtime_nvram
assert 'if [ "$action" != "stop" ]; then' in runtime_nvram
assert "IPS4=" in runtime_nvram and "IPS6=" in runtime_nvram and "dnsmasq_md5=" in runtime_nvram
assert "nvram get " not in smartdns, "all SmartDNS configuration reads must use checked NVRAM access"
lock_start = smartdns.index("if smartdns_lifecycle_lock_acquire; then")
snapshot_call = smartdns.index("if smartdns_load_runtime_nvram_state; then")
read_ini_call = smartdns.index("if Read_ini; then")
assert lock_start < snapshot_call < read_ini_call, "runtime snapshot must be taken under the lifecycle lock"
assert "smartdns runtime NVRAM snapshot failed" in smartdns
assert 'value=1' in (root / "trunk/user/www/n56u_ribbon_fixed/Advanced_smartdns.asp").read_text()
assert 'value=0' in (root / "trunk/user/www/n56u_ribbon_fixed/Advanced_smartdns.asp").read_text()
assert "SmartDNS binary enable selector validation: PASS" in enable_test
stop_nvram_test = (root / "trunk/user/smartdns/tests/smartdns_stop_nvram_failure.sh").read_text()
runtime_nvram_test = (root / "trunk/user/smartdns/tests/smartdns_runtime_nvram_snapshot.sh").read_text()
for marker in ("explicit stop bypasses every configuration NVRAM read: PASS",
               "each checked NVRAM read preserves its original failure status: PASS",
               "invalid enable selector still fails closed: PASS",
               "SmartDNS lifecycle NVRAM stop isolation regression: PASS"):
    assert marker in stop_nvram_test, marker
for marker in ("every runtime NVRAM snapshot failure aborts before later reads: PASS",
               "valid runtime snapshot captures settings and LAN cleanup context: PASS",
               "explicit stop remains independent of live configuration NVRAM: PASS"):
    assert marker in runtime_nvram_test, marker

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
adbyby_read = body(smartdns, "smartdns_nvram_read_checked")
assert 'nvram get_checked "$1"' in adbyby_read
assert "Failed to read NVRAM key" in adbyby_read
assert 'return "$smartdns_nvram_status"' in adbyby_read
for key in ("sdns_enable", "sdns_port", "sdns_redirect", "sdnse_enable", "sdnse_port"):
    assert 'smartdns_nvram_read_checked ' + key + ' || return $?' in lifecycle_nvram
assert "Check_ss || return $?" in body(smartdns, "Main")
check_ss = body(smartdns, "Check_ss")
for key in ('ss_enable', 'ss_run_mode', 'pdnsd_enable'):
    assert 'smartdns_nvram_read_checked ' + key + ' || return $?' in check_ss
main_body = body(smartdns, "Main")
assert 'smartdns_nvram_read_checked adbyby_enable || return $?' in main_body
assert 'smartdns_nvram_read_checked adbyby_add || return $?' in main_body
assert 'pidof adbyby' not in smartdns
assert 'smartdns_nvram_read_checked sdns_adblock_url || return $?' in main_body
assert '"$sdns_adblock_url"' in body(smartdns, "Start_AD")
get_conf = body(smartdns, "Get_sdns_conf")
assert 'smartdns_nvram_read_checked sdns_staticnum_x || return $?' in get_conf
assert 'sdnss_enable_x$j' in get_conf and 'while [ "$j" -lt "$listnum" ]' in get_conf
assert '[ "$listnum" -le 64 ]' in get_conf
for field in ("name", "ip", "port", "type", "ipc", "named", "non", "ipset"):
    assert 'smartdns_nvram_read_checked "sdnss_' + field + '_x$j" || return $?' in get_conf
assert '$(nvram get ' not in get_conf and '$(seq 1 ' not in get_conf
adbyby_change = body(smartdns, "Change_adbyby")
assert 'smartdns_process_state adbyby' in adbyby_change
assert 'Cannot determine Adbyby process state' in adbyby_change
assert test.count("sed -n '/^smartdns_process_state()/,/^}/p'") == 2
assert "/usr/bin/adbyby.sh switch" not in adbyby_change, \
    "SmartDNS must not dispatch the unsupported Adbyby switch action"
for token in ("smartdns_nvram_read_checked adbyby_enable",
              "smartdns_nvram_read_checked adbyby_add",
              "if nvram set adbyby_add=0; then", "if nvram set adbyby_add=1; then",
              'return "$adbyby_nvram_status"'):
    assert token in adbyby_change, token
assert adbyby_change.index("nvram set adbyby_add=0") < adbyby_change.index('hosts_type="dnsmasq"')
assert adbyby_change.index("nvram set adbyby_add=1") < adbyby_change.index('hosts_type="SmartDNS"')

start = body(smartdns, "Start_smartdns")
assert 'if Change_adbyby; then' in start
assert 'smartdns_restore_previous_instance || :' in start[start.index('if Change_adbyby; then'):start.index('sdns_redirected=')]
clean_at = start.find("smartdns_dnsmasq_remove_saved_directives")
write_at = start.find("smartdns_write_ini_state")
publish_at = start.find("Change_dnsmasq start")
assert clean_at >= 0 and write_at > clean_at and publish_at > write_at
assert 'sdns_ported="$sdns_port"' in start and 'sdns_redirected="$sdns_redirect"' in start
assert "smartdns_state_version=managed-v1" in start
stop = body(smartdns, "Stop_smartdns")
assert 'if Change_adbyby; then' in stop
assert stop.index("smartdns_stop_process") < stop.index('if Change_adbyby; then') < stop.index("Change_iptable")
assert 'return "$smartdns_adbyby_status"' in stop
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
              "critical SmartDNS resolver NVRAM reads fail closed before lifecycle changes",
              "Adbyby NVRAM read failures preserve both DNS ownership targets and propagate status",
              "Adbyby NVRAM write failures preserve the active DNS target and propagate status",
              "SmartDNS does not dispatch unsupported Adbyby switch action",
              "SmartDNS DNSMasq configuration preservation regression: PASS"):
    assert token in test, token

paths = (
    "trunk/user/smartdns/smartdns.sh",
    "trunk/user/adbyby/adbyby.sh",
    "trunk/user/smartdns/smartdns_dnsmasq_config.sh",
    "trunk/user/smartdns/smartdns_config_file.sh",
    "trunk/user/smartdns/smartdns_lifecycle_lock.sh",
    "trunk/user/smartdns/tests/smartdns_lifecycle_lock.sh",
    "trunk/user/smartdns/tests/smartdns_enable_selector.sh",
    "trunk/user/smartdns/tests/smartdns_stop_nvram_failure.sh",
    "trunk/user/smartdns/tests/smartdns_runtime_nvram_snapshot.sh",
    "trunk/user/rc/src/services.c",
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
assert "smartdns_start_config_rollback" in start
assert '$(INSTALL_BIN) ./smartdns_config_file.sh $(ROMFSDIR)/usr/bin/smartdns_config_file.sh' in makefile
config_append = body(config_file, "smartdns_config_append_filtered")
assert 'awk \'$0 !~ /^#/ && $0 != "" { print }\'' in config_append
assert 'smartdns_config_append_filtered "$smartdns_source_conf" "$smartdns_tmp_Conf"' in start
assert 'SmartDNS 自定义配置读取失败' in start
assert 'grep -v \'^#\' "$smartdns_address_Conf"' not in start
for token in ("optional config append filters comments and empty lines",
              "append generator failure is propagated before publication",
              "generator failure preserves last-good config",
              "rename failure preserves last-good config",
              "symlink, special-file and missing-source targets fail closed",
              "SmartDNS atomic configuration publication regression: PASS"):
    assert token in config_file_test, token

adbyby = (root / "trunk/user/adbyby/adbyby.sh").read_text()
adbyby_dns = body(adbyby, "add_dns")
adbyby_hosts = body(adbyby, "hosts_ads")
assert 'ADBYBY_DNSMASQ_AD_DIR="/etc/storage/dnsmasq-adbyby.d"' in adbyby
assert 'ADBYBY_ANTI_AD_FILE="$ADBYBY_DNSMASQ_AD_DIR/anti-ad-for-dnsmasq.conf"' in adbyby
assert 'WAN_FILE="$ADBYBY_DNSMASQ_AD_DIR/03-adbyby-ipset.conf"' in adbyby
assert 'mkdir -p "$ADBYBY_DNSMASQ_AD_DIR"' in adbyby_dns
assert 'printf \'%s\\n\' "conf-dir=$ADBYBY_DNSMASQ_AD_DIR"' in adbyby_dns
assert '$ADBYBY_DNSMASQ_AD_DIR/06-dnsmasq.esc' in adbyby_dns
assert '$ADBYBY_DNSMASQ_AD_DIR/07-dnsmasq.black' in adbyby_dns
assert '$ADBYBY_DNSMASQ_AD_DIR/08-dnsmasq.douyin' in adbyby_dns
assert 'ADBYBY_HOSTS_CONFIG_FILE' in adbyby_dns
assert 'adbyby_remove_generated_hosts_config' in adbyby_hosts
assert 'printf \'addn-hosts=%s\\n\' "$PROG_PATH/hosts"' in adbyby_hosts
for token in ("Change_dnsmasq stop", "Change_dnsmasq start",
              "Change_adbyby", "hosts_type=dnsmasq", "hosts_type=SmartDNS",
              "adbyby_add=0", "adbyby_add=1",
              "ADBYBY_DNSMASQ_AD_DIR",
              "addn-hosts=$PROG_PATH/hosts",
              "conf-file /tmp/anti-ad-for-smartdns.conf",
              "SmartDNS config imported DNSMasq-only Adbyby fragments",
              "SmartDNS config imported the DNSMasq-only Adbyby hosts file",
              "Adbyby add_dns publishes actual DNSMasq fragments and conf-dir consumer",
              "Alternate-port SmartDNS keeps DNSMasq Adbyby rules active across both ownership directions",
              "SmartDNS port-53 mode keeps DNSMasq-only Adbyby rules separate from SmartDNS input",
              "SmartDNS/DNSMasq ownership transitions preserve Adbyby DNSMasq inputs"):
    assert token in test, token
rollback = body(smartdns, "smartdns_start_config_rollback")
assert "nvram set sdns_enable=0" in rollback
assert "Stop_smartdns" in rollback
assert "DNSMasq 回滚未完成" in rollback
assert start.count("smartdns_start_config_rollback") == 7
# DNSMasq must successfully reload the resolver config before SmartDNS is
# stopped, and a failed reload must preserve the daemon and saved retry state.
stop_compact = " ".join(stop.split())
assert stop.index("Change_dnsmasq stop") < stop.index("/sbin/restart_dhcpd") < stop.index("smartdns_stop_process")
assert '[ "$smartdns_state_saved" = 1 ]' in stop
assert 'return "$dnsmasq_restart_status"' in stop
assert 'preserving running daemon and saved state' in stop
assert 'dnsmasq_restart_status=$?' in start
assert 'smartdns_start_config_rollback "$dnsmasq_restart_status"' in start
restore_previous = body(smartdns, "smartdns_restore_previous_instance")
assert "[ \"${smartdns_previous_running:-0}\" = 1 ] || return 0" in restore_previous
assert 'smartdns_process_state' in restore_previous
assert '"$smartdns_Bin" -f -c "$smartdns_Conf" "${args:-}"' in restore_previous
assert 'smartdns_previous_running=1' in start
assert start.count("smartdns_restore_previous_instance") == 4
assert 'smartdns_restore_previous_instance || :' in rollback
lifecycle_lock = (root / "trunk/user/smartdns/smartdns_lifecycle_lock.sh").read_text()
lifecycle_lock_test = (root / "trunk/user/smartdns/tests/smartdns_lifecycle_lock.sh").read_text()
lock_acquire = body(lifecycle_lock, "smartdns_lifecycle_lock_acquire")
lock_release = body(lifecycle_lock, "smartdns_lifecycle_lock_release")
assert 'mkdir -m 0700 "$_smartdns_lock_path"' in lock_acquire
assert 'kill -0 "$_smartdns_lock_owner"' in lock_acquire
assert 'mv "$_smartdns_lock_path" "$_smartdns_stale_path"' in lock_acquire
assert '$_smartdns_lock_owner" = "' + chr(36) * 2 + '"' in lock_release
assert 'smartdns_lifecycle_lock_lib="/usr/bin/smartdns_lifecycle_lock.sh"' in smartdns
assert '. "$smartdns_lifecycle_lock_lib"' in smartdns
assert 'if smartdns_lifecycle_lock_acquire; then' in smartdns
assert "trap 'smartdns_lifecycle_lock_release" in smartdns
assert 'Another lifecycle operation is active or its lock is unsafe' in smartdns
for token in ("exclusive acquire, owner validation and release",
              "live-owner exclusion and dead-owner reclamation",
              "malformed owner and symlink locks fail closed",
              "SmartDNS lifecycle lock regression: PASS"):
    assert token in lifecycle_lock_test, token
services = (root / "trunk/user/rc/src/services.c").read_text()
restart_signature = "int restart_smartdns(void){"
assert services.count(restart_signature) == 1
restart_wrapper = services.split(restart_signature, 1)[1].split(chr(10) + "}", 1)[0]
assert 'eval("/usr/bin/smartdns.sh","restart")' in restart_wrapper
assert "stop_smartdns()" not in restart_wrapper
assert 'if [ "$sdns_enable" != "1" ]; then' in smartdns
print("SmartDNS saved DNSMasq state, atomic config and serialized lifecycle contracts: PASS")
