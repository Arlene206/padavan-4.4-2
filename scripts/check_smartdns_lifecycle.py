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
storage_workflow = (root / ".github/workflows/storage-hotplug-stability.yml").read_text()
xy_workflow = (root / ".github/workflows/xy-c1-firmware-validation.yml").read_text()

def body(source, start, end):
    assert source.count(start) == 1, f"expected one function marker: {start}"
    return source.split(start, 1)[1].split(end, 1)[0]

assert 'smartdns_process_lifecycle_lib="/usr/bin/smartdns_process_lifecycle.sh"' in script
assert '. "$smartdns_process_lifecycle_lib"' in script
assert '$(INSTALL_BIN) ./smartdns_process_lifecycle.sh $(ROMFSDIR)/usr/bin/smartdns_process_lifecycle.sh' in makefile
assert 'smartdns_process_state()' in helper and 'smartdns_stop_process()' in helper
assert '[ -z "$_smartdns_pid_output" ] && return 1' in helper
assert '*)\n            return 2' in helper
assert 'killall smartdns' in helper and 'killall -9 smartdns' in helper
assert 'smartdns_wait_stopped' in helper
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
assert xy_workflow.count('trunk/user/smartdns/**') == 1
print("SmartDNS lifecycle wiring and fail-closed stop contract: PASS")
