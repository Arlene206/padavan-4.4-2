#!/usr/bin/env python3
"""Guard SmartDNS process-state handling and stop postconditions."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
script = (root / "trunk/user/smartdns/smartdns.sh").read_text()
helper = (root / "trunk/user/smartdns/smartdns_process_lifecycle.sh").read_text()
test = (root / "trunk/user/smartdns/tests/smartdns_process_lifecycle.sh").read_text()
autostart_test = (root / "trunk/user/scripts/tests/autostart_optional_start_status.sh").read_text()
makefile = (root / "trunk/user/smartdns/Makefile").read_text()
process_workflow = (root / ".github/workflows/process-lifecycle-regression.yml").read_text()
dnsmasq_workflow = (root / ".github/workflows/dnsmasq-lifecycle.yml").read_text()
storage_workflow = (root / ".github/workflows/storage-hotplug-stability.yml").read_text()
xy_workflow = (root / ".github/workflows/xy-c1-firmware-validation.yml").read_text()

def body(source, start, end):
    assert source.count(start) == 1, f"expected one function marker: {start}"
    return source.split(start, 1)[1].split(end, 1)[0]

assert 'smartdns_process_lifecycle_lib="/usr/bin/smartdns_process_lifecycle.sh"' in script
assert '. "$smartdns_process_lifecycle_lib"' in script
assert '$(INSTALL_BIN) ./smartdns_process_lifecycle.sh $(ROMFSDIR)/usr/bin/smartdns_process_lifecycle.sh' in makefile
assert 'smartdns_process_state()' in helper and 'smartdns_stop_process()' in helper
named_state = body(helper, 'smartdns_named_process_state()\n{', '\n}\n\n# Return start_time')
process_state = body(helper, 'smartdns_process_state()\n{', '\n}\n\n# Return 0 when absent')
stop_state = body(helper, 'smartdns_stop_process()\n{', '\n}')
assert '_smartdns_name=$1' in named_state
assert 'pidof "$_smartdns_name"' in named_state
change_adbyby = body(script, 'Change_adbyby () {', "\n}\n\nChange_dnsmasq")
assert 'smartdns_named_process_state adbyby' in change_adbyby
assert 'Cannot determine Adbyby process state' in change_adbyby
assert 'pidof adbyby' not in script
assert 'Adbyby process inventory errors preserve DNS ownership' in test
assert '[ -z "$_smartdns_pid_output" ] || return 2' in process_state
assert '*) return 2 ;;' in named_state
assert 'smartdns_signal_process_ids TERM' in stop_state and 'smartdns_signal_process_ids KILL' in stop_state
assert 'smartdns_wait_stopped' in stop_state
signal_helper = body(helper, 'smartdns_signal_process_ids()\n{', '\n}\n\nsmartdns_stop_process')
assert 'smartdns_process_identity "$_smartdns_signal_pid"' in signal_helper
assert '[ "$_smartdns_current_start" = "$_smartdns_signal_start" ] || continue' in signal_helper
assert 'kill "-$_smartdns_signal" "$_smartdns_signal_pid"' in signal_helper
assert 'killall' not in helper
assert '[ "$KILLALL_CALLS" -eq 0 ] || fail "PID-scoped stop never calls name-wide killall"' in test
reader = body(script, 'Read_ini () {', '\n}\n\nCheck_md5')
assert 'if [ -L "$smartdns_Ini" ]; then' in reader
assert '[ -r "$smartdns_Ini" ] && [ -s "$smartdns_Ini" ] || return 1' in reader
assert 'case "$hosts_type" in SmartDNS|dnsmasq|0) ;; *) return 1 ;; esac' in reader
assert 'sdns_ported=$(smartdns_dnsmasq_normalize_port "$sdns_ported") || return 1' in reader
assert 'managed-v1) ;;' in reader
assert 'elif [ -e "$smartdns_Ini" ]; then\n        return 1' in reader
assert 'if Read_ini; then\n    Main\nelse' in script
assert 'Saved SmartDNS resolver state is invalid; preserving DNSMasq configuration' in script
assert 'exit "$smartdns_ini_status"' in script
assert 'SMARTDNS_STATUS=45' in autostart_test
assert 'SmartDNS autostart failed (status=45)' in autostart_test
assert 'autostart consumed malformed SmartDNS saved state' in autostart_test
assert 'cmp -s "$TMP_DIR/dnsmasq.before" "$TMP_DIR/storage/dnsmasq/dnsmasq.conf"' in autostart_test
assert 'cmp -s "$TMP_DIR/smartdns-state.before" "$TMP_DIR/storage/smartdns_conf.ini"' in autostart_test
assert 'SMARTDNS_STATUS=0' in autostart_test
assert 'SmartDNS failure blocked independent DDNSTO startup' in autostart_test
assert 'SmartDNS failure blocked independent FRP startup' in autostart_test
start = body(script, 'Start_smartdns () {', '\n}\n\nStop_smartdns')
stop = body(script, 'Stop_smartdns () {', '\n}\n\nMain')
main = body(script, 'Main () {', '\n}\n\nRead_ini')
assert start.index('smartdns_process_state') < start.index('smartdns_stop_process')
assert start.index('smartdns_stop_process') < start.index('smartdns_dnsmasq_remove_saved_directives')
assert start.index('smartdns_dnsmasq_remove_saved_directives') < start.index('smartdns_write_ini_state')
assert start.index('smartdns_write_ini_state') < start.index('Change_dnsmasq start')
assert 'smartdns_state_version=managed-v1' in start
assert ':>"$smartdns_Ini"' not in start
writer = body(script, 'smartdns_write_ini_state () {', '\n}\n\n')
assert 'mktemp "${smartdns_Ini}.tmp.XXXXXX"' in writer
assert 'chmod 600 "$smartdns_ini_tmp"' in writer
assert 'mv -f "$smartdns_ini_tmp" "$smartdns_Ini"' in writer
assert 'smartdns_dnsmasq_publish_managed_block "$@"' in script
assert 'return 2' in start and 'return 1' in start
assert 'Cannot verify SmartDNS after launch' in start
assert 'pidof smartdns' not in script and 'killall' not in script
assert stop.index('smartdns_stop_process') < stop.index('Change_adbyby')
assert stop.index('if [ "$stop_status" -ne 0 ]') < stop.index('Change_adbyby')
assert 'return "$stop_status"' in stop
assert 'Change_dnsmasq stop' in stop
assert 'DNSMasq resolver cleanup incomplete' in stop
assert stop.index('if [ "$smartdns_dnsmasq_status" -ne 0 ]') < stop.index('rm -f "$smartdns_Ini"')
assert 'Start_smartdns\n        start_status=$?' in main
assert 'Stop_smartdns\n        stop_status=$?' in main
assert main.count('return "$start_status"') == 2
assert 'return "$stop_status"' in main
for phrase in ('status 1 plus empty output means absent','status 0 plus empty output is unknown',
               'pidof error is unknown','malformed PID list is unknown',
               'unknown daemon state fails closed','graceful signal is verified by fresh absent snapshot',
               'unknown post-signal state prevents force escalation',
               'force escalation occurs only after graceful timeout',
               'daemon still running after force escalation is reported'):
    assert phrase in test, phrase
adblock = body(script, 'smartdns_prepare_adblock_filter()', '\n}\n\nStart_AD')
adblock_start = body(script, 'Start_AD () {', '\n}\n\nChange_adbyby')
assert 'mktemp ' in adblock and 'cat "$_smartdns_adblock_input"' in adblock
assert 'grep -Eq' in adblock and 'awk ' in adblock
assert 'index(rule, "||") != 1' in adblock
assert 'converted == 0' in adblock
assert 'mv -f "$_smartdns_adblock_tmp" "$_smartdns_adblock_output"' in adblock
assert 'curl -fsS' in adblock_start
assert 'smartdns_prepare_adblock_filter "$_smartdns_adblock_download" "$_smartdns_adblock_output"' in adblock_start
assert 'check =' not in adblock_start
assert 'sdns_adblock_url' in adblock_start
for phrase in ('Adblock Plus rules convert to SmartDNS address directives',
               'native SmartDNS address rules remain unchanged',
               'failed adblock conversion preserves the previous output'):
    assert phrase in test, phrase
assert 'scripts/check_smartdns_lifecycle.py' in process_workflow
assert 'sh trunk/user/smartdns/tests/smartdns_process_lifecycle.sh' in process_workflow
assert 'python3 scripts/check_smartdns_lifecycle.py' in process_workflow
assert storage_workflow.count('trunk/user/smartdns/**') == 2
assert '"$busybox_bin" ash trunk/user/smartdns/tests/smartdns_process_lifecycle.sh' in storage_workflow
firewall_add = body(script, 'smartdns_firewall_add () {', '\\n}\\n\\nsmartdns_firewall_delete')
firewall_delete = body(script, 'smartdns_firewall_delete () {', '\\n}\\n\\nChange_iptable')
firewall = body(script, 'Change_iptable () {', '\\n}\\n\\nsmartdns_restore_previous_instance')
assert '"$@" >/dev/null 2>&1' in firewall_add
assert '"$firewall_tool" -t "$firewall_table" -C "$@"' in firewall_delete
assert '"$firewall_tool" -t "$firewall_table" -D "$@"' in firewall_delete
assert 'return "$check_status"' in firewall_delete
assert 'smartdns_firewall_delete iptables nat PREROUTING' in firewall
assert 'smartdns_firewall_delete ip6tables nat PREROUTING' in firewall
assert 'smartdns_firewall_add iptables -t nat -A PREROUTING' in firewall
assert 'smartdns_firewall_add ip6tables -t nat -A PREROUTING' in firewall
assert 'iptables -t nat -D' not in firewall and 'ip6tables -t nat -D' not in firewall
assert 'return 2' in firewall
assert 'if Change_iptable "$action"; then' in start
assert 'smartdns_start_config_rollback "$smartdns_firewall_status"' in start
assert start.index('if Change_iptable "$action"; then') < start.index('smartdns_start_config_rollback "$smartdns_firewall_status"')
assert 'if Change_iptable stop; then' in stop
assert 'preserving running daemon and saved state for retry' in stop
assert stop.index('if Change_iptable stop; then') < stop.index('smartdns_stop_process') < stop.index('rm -f "$smartdns_Ini"')
assert 'smartdns_firewall_status.sh' in dnsmasq_workflow
assert 'sh trunk/user/smartdns/tests/smartdns_firewall_status.sh' in dnsmasq_workflow
assert 'python3 scripts/check_smartdns_lifecycle.py' in dnsmasq_workflow
assert 'scripts/check_smartdns_lifecycle.py' in process_workflow
assert xy_workflow.count('trunk/user/smartdns/**') == 1
print("SmartDNS lifecycle, firewall status and fail-closed cleanup contract: PASS")
