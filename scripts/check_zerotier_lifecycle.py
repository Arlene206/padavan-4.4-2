#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
script_path = ROOT / "trunk/user/zerotier/zerotier.sh"
test_path = ROOT / "trunk/user/zerotier/tests/zerotier_lifecycle.sh"
makefile_path = ROOT / "trunk/user/zerotier/Makefile"
services_path = ROOT / "trunk/user/rc/src/services.c"
rc_header_path = ROOT / "trunk/user/rc/src/rc.h"
rc_path = ROOT / "trunk/user/rc/src/rc.c"
workflow_path = ROOT / ".github/workflows/zerotier-lifecycle.yml"
zerotier_page_path = ROOT / "trunk/user/www/n56u_ribbon_fixed/Advanced_zerotier.asp"

script = script_path.read_text()
test = test_path.read_text()
makefile = makefile_path.read_text()
services = services_path.read_text()
rc_header = rc_header_path.read_text()
rc = rc_path.read_text()
workflow = workflow_path.read_text()
zerotier_page = zerotier_page_path.read_text()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"ZeroTier lifecycle source contract: FAIL: {message}")


def function_body(source: str, signature: str) -> str:
    start = source.find(signature)
    require(start >= 0, f"missing function {signature}")
    opening = source.find("{", start + len(signature))
    require(opening >= 0, f"missing opening brace for {signature}")

    depth = 0
    state = "code"
    pos = opening
    while pos < len(source):
        char = source[pos]
        nxt = source[pos + 1] if pos + 1 < len(source) else ""
        if state == "code":
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "/" and nxt == "*":
                state = "block_comment"
                pos += 1
            elif char == "/" and nxt == "/":
                state = "line_comment"
                pos += 1
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[opening + 1:pos]
        elif state in ("string", "char"):
            if char == "\\\\":
                pos += 1
            elif state == "string" and char == '"':
                state = "code"
            elif state == "char" and char == "'":
                state = "code"
        elif state == "block_comment":
            if char == "*" and nxt == "/":
                state = "code"
                pos += 1
        elif state == "line_comment" and char == "\\n":
            state = "code"
        pos += 1
    raise AssertionError("unterminated function: " + signature)

wait_body = function_body(script, "zerotier_wait_stopped()")
kill_body = function_body(script, "kill_z()")
start_body = function_body(script, "start_zero()")
stop_body = function_body(script, "stop_zero()")
planet_body = function_body(script, "zerotier_publish_planet_base64()")
planet_import_body = function_body(script, "zerotier_import_planet_file()")
start_instance_body = function_body(script, "start_instance()")
nat_settings_body = function_body(script, "zerotier_read_nat_enabled()")
route_count_body = script.split("zerotier_validate_static_route_count() {", 1)[1].split("\n}\n", 1)[0]
route_preflight_body = function_body(script, "zerotier_preflight_static_routes()")
start_settings_body = function_body(script, "zerotier_load_start_settings()")
network_id_body = function_body(script, "zerotier_validate_network_id()")
creat_moon_body = script.split("creat_moon() {", 1)[1].split("\n\nzerotier_rollback_moon_remove()", 1)[0]
moon_ip_body = function_body(script, "zerotier_moon_ipv4_is_valid()")
moon_create_rollback_body = function_body(script, "zerotier_rollback_moon_create()")
remove_moon_body = script.split("remove_moon() {", 1)[1].split("\ncase $1 in", 1)[0]
moon_remove_rollback_body = function_body(script, "zerotier_rollback_moon_remove()")
config_dir_body = function_body(script, "zerotier_prepare_config_directory()")
identity_body = function_body(script, "zerotier_prepare_identity()")
add_join_body = function_body(script, "add_join()")
interface_body = function_body(script, "zerotier_interface_name()")
route_section = script.split("zerotier_kernel_route_segment() {", 1)[1].split("\nzerotier_wait_interface() {", 1)[0]
interface_wait_body = function_body(script, "zerotier_wait_interface()")
route_wait_body = function_body(script, "zerotier_wait_route()")
rules_section = script.split("rules() {", 1)[1].split("\ndel_rules() {", 1)[0]
firewall_helper_body = function_body(script, "zerotier_add_firewall_rule()")
cleanup_section = script.split("del_rules() {", 1)[1].split("\nzero_route() {", 1)[0]
del_rules_body = function_body(script, "del_rules()")
delete_rule_body = function_body(script, "zerotier_delete_firewall_rule()")
rule_action_body = function_body(script, "zerotier_firewall_rule_action()")
segment_body = function_body(script, "zerotier_ipv4_cidr_is_valid()")
route_tuple_body = function_body(script, "zerotier_static_route_tuple_is_valid()")
state_save_body = function_body(script, "zerotier_firewall_state_save()")
state_load_body = function_body(script, "zerotier_firewall_state_load()")
state_recover_body = function_body(script, "zerotier_firewall_state_recover_prefix()")
state_clear_body = function_body(script, "zerotier_firewall_state_clear()")
static_route_body = function_body(script, "zero_route()")
route_match_body = function_body(script, "zero_route_entry_matches()")

process_matches_body = function_body(script, "zerotier_process_matches()")
process_pids_body = function_body(script, "zerotier_process_pids()")
process_state_body = function_body(script, "zerotier_process_state()")
signal_body = function_body(script, "zerotier_signal_all()")
require('while [ "$zerotier_wait" -lt 5 ]; do' in wait_body and
        'zerotier_process_state' in wait_body and 'sleep 1' in wait_body,
        "daemon absence must be checked through five bounded one-second tri-state process polls")
require('readlink "$zt_match_exe_path"' in process_matches_body and
        'case "$zt_match_exe" in' in process_matches_body and
        '"$PROG"' in process_matches_body and
        "tr '\\000' '\\n'" in process_matches_body and
        '[ "$zt_match_argv0" = "$PROG" ]' in process_matches_body and
        '[ "$zt_match_argv1" = "$config_path" ]' in process_matches_body and
        'zt_match_comm=$(cat "$zt_match_proc_dir/comm" 2>/dev/null)' in process_matches_body,
        "process identity must match the exact executable and argv[0], not the mutable comm name")
require('"$ZT_PROC_ROOT"/[0-9]*' in process_pids_body and
        'zerotier_process_matches "$zt_scan_pid"' in process_pids_body and
        'return 2' in process_pids_body and
        '[ -r "$ZT_PROC_ROOT" ]' in process_pids_body,
        "process inventory must scan procfs and preserve UNKNOWN on incomplete metadata")
require('zt_process_pids=$(zerotier_process_pids)' in process_state_body and
        '[ -n "$zt_process_pids" ] && return 0' in process_state_body and
        'return 1' in process_state_body,
        "process inventory must distinguish RUNNING, confirmed ABSENT and UNKNOWN")
require('zerotier_process_pids' in signal_body and
        'zerotier_process_matches "$zt_signal_pid"' in signal_body and
        'kill "-$zt_signal" "$zt_signal_pid"' in signal_body and
        'killall' not in signal_body and
        'pidof' not in process_state_body,
        "signals must target only freshly revalidated exact executable PIDs")
require('if [ "$zerotier_stop_state" -ne 1 ]; then' in kill_body and
        kill_body.index('zerotier_wait_stopped') < kill_body.index('if [ "$zerotier_stop_state" -ne 1 ]; then') <
        kill_body.index('zerotier_signal_all KILL'),
        "UNKNOWN during graceful stop must block SIGKILL escalation")
require(kill_body.index('zerotier_signal_all TERM') < kill_body.index('zerotier_wait_stopped') <
        kill_body.index('zerotier_signal_all KILL') <
        kill_body.rindex('zerotier_wait_stopped'),
        "TERM grace period and force-stop absence postcondition are out of order")
require('return 1' not in kill_body or 'zerotier_wait_stopped' in kill_body,
        "stop failure must propagate the bounded absence result")
require('if ! kill_z; then' in start_body and
        start_body.index('if ! kill_z; then') < start_body.index("start_instance 'zerotier'"),
        "start must not launch a new daemon unless the previous daemon is absent")

orbit_rollback_body = function_body(script, "zerotier_rollback_failed_start()")
orbit_command = start_instance_body.index(
    'if "$PROGCLI" -D"$config_path" orbit "$moonid" "$moonid"; then'
)
orbit_success_log = start_instance_body.index('orbit moonid $moonid ok!', orbit_command)
orbit_failure_status = start_instance_body.index("zt_orbit_status=$?", orbit_command)
orbit_rollback = start_instance_body.index(
    "if ! zerotier_rollback_failed_start; then", orbit_failure_status
)
orbit_failure_return = start_instance_body.index("return 1", orbit_rollback)
rollback_stop = orbit_rollback_body.index("if ! kill_z; then")
rollback_firewall = orbit_rollback_body.index("if ! del_rules; then")
rollback_route = orbit_rollback_body.index('if ! zero_route "del"; then')
rollback_checkpoint = orbit_rollback_body.index("zerotier_firewall_state_clear")
require(
    orbit_command < orbit_success_log < orbit_failure_status < orbit_rollback < orbit_failure_return
    and rollback_stop < rollback_firewall < rollback_route < rollback_checkpoint
    and 'return "$zt_start_rollback_status"' in orbit_rollback_body,
    "Moon orbit must propagate CLI failure and only clear network recovery state after confirmed stop and complete cleanup",
)
for token in (
    "start_instance_production orbit-failure",
    "production launcher reported success after the Moon orbit command failed",
    "successful orbit-failure rollback did not stop then clean network state in order",
    "production launcher accepted an orbit failure when daemon stop failed",
    "incomplete firewall rollback discarded its checkpoint",
    "Moon orbit did not receive the configured ID as two distinct argv values",
):
    require(token in test, "Moon orbit failure regression missing " + token)

# Root-owned config writers must not inherit a permissive umask or trust
# NVRAM path components and persistent symlink destinations.
require("umask 077" in script[:300],
        "ZeroTier must not inherit a permissive launcher umask")
require('if [ -L "$config_path" ]' in config_dir_body and
        'chmod 700 "$config_path"' in config_dir_body and
        'if [ -L "$zt_network_dir" ]' in config_dir_body and
        'chmod 700 "$zt_network_dir"' in config_dir_body,
        "ZeroTier configuration directories must reject symlinks and be private")
require('mktemp -d "$config_path/.identity.XXXXXX"' in identity_body and
        'trap \'zerotier_identity_cleanup "$?"\' EXIT' in identity_body and
        '"$PROGIDT" generate "$zt_identity_secret_tmp" "$zt_identity_public_tmp"' in identity_body and
        '"$PROGIDT" getpublic "$zt_identity_secret_tmp"' in identity_body and
        'ln "$config_path/identity.secret" "$zt_identity_secret_backup"' in identity_body and
        'ln "$config_path/identity.public" "$zt_identity_public_backup"' in identity_body and
        'mv -f "$zt_identity_public_tmp" "$config_path/identity.public"' in identity_body and
        'mv -f "$zt_identity_secret_tmp" "$config_path/identity.secret"' in identity_body and
        'zt_identity_secret_had_original' in identity_body and
        'zt_identity_public_had_original' in identity_body and
        'zt_identity_preserve_backup=1' in identity_body and
        'rm -rf "$zt_identity_tmp_dir"' in identity_body,
        "identity pair publication must retain hard-link recovery points and roll back partial renames");
require(identity_body.index('mv -f "$zt_identity_secret_tmp" "$config_path/identity.secret"') <
        identity_body.index('nvram set zerotier_secret="$secret"') <
        identity_body.index('nvram commit') <
        identity_body.index('zt_identity_transaction_active=0'),
        "new identity secret must be persisted only after both files are published");
require('2|3) return 1' in test and
        'identity.public.previous' in test and
        'NVRAM commit failure was reported as success' in test and
        'NVRAM set failure was reported as success' in test,
        "identity rollback regression must cover second rename, failed recovery, and NVRAM failures")
require('if [ -L "$zt_identity_dest" ]' in identity_body and
        'nvram set zerotier_secret="$secret"' in identity_body and
        'nvram commit' in identity_body,
        "identity publication must reject symlink destinations and persist generated secrets")
require('case "$zt_start_moon_enable" in' in start_settings_body and
        '""|0|1) ;;' in start_settings_body and
        'if ! zerotier_validate_network_id "$zt_start_network_id"; then' in start_settings_body and
        'if ! zerotier_validate_network_id "$zt_network_id"; then' in add_join_body and
        '*[!0123456789abcdefABCDEF]*' in network_id_body and
        '[ "${#1}" -eq 16 ]' in network_id_body,
        "startup Moon toggle and network ID values must be validated before stopping the existing daemon")
require('[ -n "$zt_network_id" ] || return 0' in add_join_body and
        'zerotier_validate_network_id "$zt_network_id"' in add_join_body and
        'touch "$zt_network_file"' in add_join_body and
        'chmod 600 "$zt_network_file"' in add_join_body and
        'if [ -L "$zt_network_file" ]' in add_join_body,
        "network join writer must validate 16 hex digits and refuse path/symlink abuse")
planet_snapshot_copy = planet_import_body.index('! cp "$zt_planet_source" "$zt_planet_stage"')
planet_snapshot_encode = planet_import_body.index('! base64 "$zt_planet_stage" > "$zt_planet_encoded"')
planet_snapshot_publish = planet_import_body.index('mv -f "$zt_planet_stage" "$zt_planet_target"')
planet_snapshot_nvram = planet_import_body.index('zt_planet_new_value="$(cat "$zt_planet_encoded")"')
require(planet_snapshot_copy < planet_snapshot_encode < planet_snapshot_publish <
        planet_snapshot_nvram and
        '! base64 "$zt_planet_source" > "$zt_planet_encoded"' not in planet_import_body,
        "uploaded planet file and NVRAM value must derive from one private snapshot")
require('mktemp -d "$config_path/.planet-import.XXXXXX"' in planet_import_body and
        planet_import_body.index("trap 'zerotier_planet_import_cleanup") <
        planet_import_body.index('zt_planet_old_value="$(nvram get_checked zerotier_planet)"') and
        'if [ "$zt_planet_transaction_active" = "1" ] &&' in planet_import_body and
        '[ "$zt_planet_delete_pending" = "1" ]; then' in planet_import_body and
        '[ -L "$zt_planet_source" ] || [ ! -f "$zt_planet_source" ]' in planet_import_body and
        'ln "$zt_planet_target" "$zt_planet_backup"' in planet_import_body and
        'mv -f "$zt_planet_stage" "$zt_planet_target"' in planet_import_body and
        'zt_planet_delete_pending=1' in planet_import_body and
        'nvram set zerotier_planet="$zt_planet_new_value"' in planet_import_body and
        'nvram set zerotier_planet="$zt_planet_old_value"' in planet_import_body and
        'planet.nvram.previous' in planet_import_body and
        'printf \'%s\' "$zt_planet_old_value" > "$zt_planet_import_dir/planet.nvram.previous"' in planet_import_body and
        'zt_planet_preserve_backup=1' in planet_import_body and
        'ln -f "$zt_planet_backup" "$zt_planet_target"' in planet_import_body,
        "uploaded planet import must retain old file/NVRAM state on failure and safely restore hard links")
require('if ! zerotier_import_planet_file "$PLANET"; then' in start_instance_body and
        'if [ -L "$PLANET" ]' in start_instance_body and
        'cp -f $PLANET $config_path/planet' not in start_instance_body and
        'nvram commit' not in start_instance_body,
        "uploaded planet import must reject unsafe input and use rollback-capable writer")
for marker in ("uploaded planet contents were not published",
               "empty planet import left stale NVRAM data",
               "symlink upload referent was modified",
               "failed planet NVRAM commit did not restore the previous file",
               "failed planet NVRAM rollback did not retain its old-value recovery snapshot",
               "planet NVRAM recovery snapshot was not private",
               "failed planet publish changed NVRAM",
               "failed planet publish rollback leaked its recovery directory",
               "failed NVRAM read leaked the private import directory",
               "planet import published a file that differs from the pre-encoding upload snapshot",
               "planet import NVRAM does not encode the published snapshot"):
    require(marker in test, f"uploaded planet import regression omits {marker}")
require('mktemp "$config_path/.planet.XXXXXX"' in planet_body and
        'printf \'%s\' "$1" | base64 -d > "$zt_planet_tmp"' in planet_body and
        'chmod 600 "$zt_planet_tmp"' in planet_body and
        'mv -f "$zt_planet_tmp" "$zt_planet_target"' in planet_body and
        '[ -L "$zt_planet_target" ]' in planet_body and
        'if ! zerotier_publish_planet_base64 "$planet"; then' in start_instance_body and
        'echo "$planet" >$config_path/planet.tmp' not in start_instance_body,
        "NVRAM planet payload must be validated and privately staged before atomic publication")
for marker in ("valid NVRAM planet payload was not published",
               "invalid planet payload damaged the last-good file",
               "planet symlink referent was modified",
               "directory planet destination was accepted"):
    require(marker in test, f"planet publication regression omits {marker}")
startup_settings = (
    ("zerotier_moonid", "moonid", "moonid"),
    ("zerotier_secret", "secret", "secret"),
    ("zerotiermoon_enable", "moon_enable", "enablemoonserv"),
    ("zerotier_planet", "planet", "planet"),
    ("zerotier_id", "network_id", "zt_network_id"),
)
startup_read_positions = []
startup_publish_positions = []
for key, temporary_name, public_name in startup_settings:
    read_expression = f'nvram get_checked {key}'
    read_guard = f'if ! zt_start_{temporary_name}="$(nvram get_checked {key})"; then'
    publish_expression = f'{public_name}="$zt_start_{temporary_name}"'
    require(start_settings_body.count(read_expression) == 1 and
            read_guard in start_settings_body and
            f'nvram get {key}' not in start_settings_body and
            f'nvram get {key}' not in start_instance_body,
            f"startup selector {key} must use exactly one checked NVRAM read")
    startup_read_positions.append(start_settings_body.index(read_expression))
    require(start_settings_body.count(publish_expression) == 1,
            f"startup selector {key} must be published exactly once after the full snapshot")
    startup_publish_positions.append(start_settings_body.index(publish_expression))
require(max(startup_read_positions) < min(startup_publish_positions),
        "startup settings must all be read successfully before any consumer-visible selector is published")
require('if ! zerotier_load_start_settings; then' in start_body and
        'if ! zerotier_read_nat_enabled; then' in start_body and
        'if ! zerotier_preflight_static_routes; then' in start_body and
        'if ! kill_z; then' in start_body and
        'if ! zerotier_replay_firewall_state; then' in start_body and
        "start_instance 'zerotier'" in start_body and
        'if ! zerotier_load_start_settings; then' not in start_instance_body and
        'if ! zerotier_prepare_config_directory; then' in start_instance_body and
        'if ! zerotier_prepare_identity; then' in start_instance_body and
        'if ! zerotier_import_planet_file "$PLANET"; then' in start_instance_body and
        'if ! add_join "$zt_network_id"; then' in start_instance_body and
        'zt_network_id="$(nvram get zerotier_id)"' not in start_instance_body,
        "startup settings and network selectors must be checked before any service stop or launch")
startup_gate_position = start_body.index('if ! zerotier_load_start_settings; then')
require(startup_gate_position < start_body.index('if ! zerotier_read_nat_enabled; then') <
        start_body.index('if ! zerotier_preflight_static_routes; then') <
        start_body.index('if ! kill_z; then') <
        start_body.index('if ! zerotier_replay_firewall_state; then') <
        start_body.index("start_instance 'zerotier'"),
        "all ZeroTier startup config preflights must precede stopping the old daemon")
for marker in ("startup settings read error was accepted",
               "startup accepted invalid Moon-enable selector",
               "startup accepted invalid network ID",
               "start accepted invalid selector $setting",
               "invalid selector $setting stopped the existing daemon",
               "startup settings partially published secret",
               "static route preflight accepted invalid enable selector 2",
               "static route executor accepted invalid enable selector 2",
               "invalid route enable selector stopped the daemon during start",
               "invalid route enable selector stopped the daemon during stop",
               "missing identity secret did not retain the legacy empty default"):
    require(marker in test, f"startup NVRAM read regression omits {marker}")
for token in ['config directory was not restricted to mode 700',
              'config preparation followed a networks.d symlink',
              'path-traversal network ID',
              'network join writer followed a symlink leaf',
              'generated identity secret was not mode 600',
              'stored identity preparation failed',
              'failed identity staging replaced the prior secret',
              'identity publisher followed a symlink destination',
              'second identity-file publication failure was reported as success',
              'incomplete identity rollback was reported as success']:
    require(token in test, f"private ZeroTier writer regression missing {token}")


require('nvram get_checked zerotier_nat' in nat_settings_body and
        'nvram get zerotier_nat' not in nat_settings_body and
        'case "$nat_enable" in' in nat_settings_body and
        'if ! zerotier_read_nat_enabled; then' in rules_section and
        'if ! zerotier_read_nat_enabled; then' in stop_body,
        "runtime NAT state must use a checked read in both firewall publication and stop preflight")
require('ZT_MAX_STATIC_ROUTES=64' in script and
        'while [ "${zt_route_count#0}" != "$zt_route_count" ]; do' in route_count_body and
        '[ "${#zt_route_count}" -le 2 ] || return 1' in route_count_body and
        '[ "$zt_route_count" -le "$ZT_MAX_STATIC_ROUTES" ]' in route_count_body and
        'zerotier_validate_static_route_count "$zt_preflight_route_count"' in route_preflight_body and
        'zerotier_validate_static_route_count "$rulesnum"' in static_route_body and
        'markGroupRULES(this, 64' in zerotier_page,
        "static route iteration must enforce the same 64-entry ceiling as the UI before seq")
require('static-route preflight accepted unsupported count' in test and
        'static-route executor accepted unsupported count' in test and
        '999999999999999999999999' in test and
        '00064' in test,
        "static route count regression must cover oversized, overflow-length and compatible normalized values")

require('1) ;;' in route_preflight_body and
        '1) ;;' in static_route_body and
        'if [ "$zt_preflight_enable" -eq 0 ]; then' not in route_preflight_body and
        'if [ "$route_enable" -eq 0 ]; then' not in static_route_body,
        "static route enable selectors must accept only the UI's 0/1 values")
require('for invalid_ip in default blackhole' in test and
        '192.0.2.0/33' in test and
        '192.0.2.1/24' in test and
        'for invalid_gateway in default 192.0.2.256' in test and
        '192.0.2.1/24' in test and
        'valid 0.0.0.0/0 destination' in test,
        "static-route tuple regressions must reject iproute2 keywords and malformed tuples while retaining explicit default CIDR support")
require('nvram get_checked zero_staticnum_x' in route_preflight_body and
        'nvram get_checked zero_enable_x$zt_preflight_j' in route_preflight_body and
        'nvram get_checked zero_ip_x$zt_preflight_j' in route_preflight_body and
        'nvram get_checked zero_route_x$zt_preflight_j' in route_preflight_body and
        route_preflight_body.index('nvram get_checked zero_staticnum_x') <
        route_preflight_body.index('nvram get_checked zero_enable_x$zt_preflight_j') <
        route_preflight_body.index('nvram get_checked zero_ip_x$zt_preflight_j') <
        route_preflight_body.index('nvram get_checked zero_route_x$zt_preflight_j'),
        "static route settings must be fully checked before restarting or stopping the daemon")
require('nvram get_checked zero_staticnum_x' in static_route_body and
        'nvram get_checked zero_enable_x$j' in static_route_body and
        'nvram get_checked zero_ip_x$j' in static_route_body and
        'nvram get_checked zero_route_x$j' in static_route_body and
        'nvram get zero_staticnum_x' not in static_route_body and
        'nvram get zero_enable_x$j' not in static_route_body,
        "static-route executor must propagate checked NVRAM errors during add/delete")
require('prefix = 32' in segment_body and
        'if (octets[i] + 0 != 0)' in segment_body and
        'for (j = remaining; j < 8; ++j)' in segment_body and
        'if ((octets[i] + 0) % block != 0)' in segment_body,
        "IPv4 CIDR validation must reject host bits set in route destinations")
require('zerotier_ipv4_cidr_is_valid "$1" || return 1' in route_tuple_body and
        '[ "${2#*/}" = "$2" ] || return 1' in route_tuple_body and
        'zerotier_ipv4_cidr_is_valid "$2"' in route_tuple_body and
        'zerotier_static_route_tuple_is_valid "$zt_preflight_ip" "$zt_preflight_gateway"' in route_preflight_body and
        'zerotier_static_route_tuple_is_valid "$zero_ip" "$zero_route"' in static_route_body,
        "static routes must validate IPv4/CIDR destinations and bare IPv4 gateways before invoking iproute2")
require('if ! zerotier_preflight_static_routes; then' in stop_body and
        stop_body.index('if ! zerotier_preflight_static_routes; then') <
        stop_body.index('if ! kill_z; then'),
        "stop must validate route configuration before signalling the daemon")

require('zt_state_file="$config_path/.firewall-state"' in stop_body and
        'if [ -e "$zt_state_file" ] || [ -L "$zt_state_file" ]; then' in stop_body and
        'if ! zerotier_firewall_state_load; then' in stop_body and
        'if ! zerotier_firewall_state_recover_prefix; then' in stop_body and
        'ZeroTier防火墙恢复检查点无效' in stop_body and
        stop_body.index('if ! zerotier_firewall_state_load; then') <
        stop_body.index('if ! zerotier_firewall_state_recover_prefix; then') <
        stop_body.index('if ! zerotier_read_nat_enabled; then') and
        'stop ignored a firewall deletion failure after recovering a valid checkpoint prefix' in test and
        'recovered checkpoint did not retain its canonical retry tuple' in test and
        'stop recovered a truncated checkpoint prefix' in test and
        'stop accepted an unterminated trailing checkpoint fragment' in test and
        'stop recovered a damaged checkpoint with an unreadable live route inventory' in test and
        'stop recovered a damaged checkpoint despite a live static route on the saved interface' in test and
        'stop recovered a damaged checkpoint with a multipath nexthop on the saved interface' in test,
        "stop may sanitize only a complete, validated checkpoint prefix; a truncated or semantically invalid tuple must still block shutdown")
for marker in ("rules accepted an unreadable NAT selector as disabled",
               "NAT selector read failure stopped the daemon",
               "static-route add accepted a checked-read error",
               "stop accepted a static-route preflight error",
               "start stopped the daemon after"):
    require(marker in test, f"checked route/NAT regression omits {marker}")

require('nvram get_checked zerotiermoon_ip' in creat_moon_body and
        'nvram get zerotiermoon_ip' not in creat_moon_body and
        'if ! moonip="$(nvram get_checked zerotiermoon_ip)"; then' in creat_moon_body,
        "Moon creation must abort before WAN-IP fallback when its checked address read fails")
require('NF != 4 { exit 1 }' in moon_ip_body and
        '$i + 0 > 255' in moon_ip_body and
        'if ! zerotier_moon_ipv4_is_valid "$ip_addr"; then' in creat_moon_body,
        "Moon creation must reject malformed and out-of-range IPv4 endpoints")
require('if ! "$PROGIDT" initmoon "$zt_moon_create_identity_public" > "$zt_moon_create_json_stage"; then' in creat_moon_body and
        'if ! sed -i ' in creat_moon_body and
        'if `sed -i ' not in creat_moon_body and
        'grep -F' in creat_moon_body and
        'if ! (cd "$zt_moon_create_stage_dir" && "$PROGIDT" genmoon' in creat_moon_body and
        '[ ! -s "$zt_moon_create_moon_stage" ]' in creat_moon_body,
        "Moon creation must check every generation/edit step before publication")
require('ln "$zt_moon_create_moon_file" "$zt_moon_create_moon_backup"' in creat_moon_body and
        'ln "$zt_moon_create_json_file" "$zt_moon_create_json_backup"' in creat_moon_body and
        'if ! mv -f "$zt_moon_create_moon_stage" "$zt_moon_create_moon_file"; then' in creat_moon_body and
        'if ! mv -f "$zt_moon_create_json_stage" "$zt_moon_create_json_file"; then' in creat_moon_body and
        'if ! nvram set zerotiermoon_id="$zt_moon_create_id" || ! nvram commit; then' in creat_moon_body,
        "Moon JSON/signature and persistent ID must be published as a checked transaction")
require('ln -f "$zt_moon_create_moon_backup" "$zt_moon_create_moon_file"' in moon_create_rollback_body and
        'ln -f "$zt_moon_create_json_backup" "$zt_moon_create_json_file"' in moon_create_rollback_body and
        'nvram set zerotiermoon_id="$zt_moon_create_old_id"' in moon_create_rollback_body,
        "Moon creation rollback must restore both file snapshots and the prior identity")
require('nvram get_checked zerotiermoon_id' in remove_moon_body and
        'nvram get zerotiermoon_id' not in remove_moon_body and
        '*[!0123456789abcdefABCDEF]*' in remove_moon_body and
        '"${#zt_moon_remove_old_id}" -ne 10' in remove_moon_body and
        'ln "$zt_moon_remove_file" "$zt_moon_remove_file_backup"' in remove_moon_body and
        'ln "$zt_moon_remove_json" "$zt_moon_remove_json_backup"' in remove_moon_body and
        'if ! nvram set zerotiermoon_id="" || ! nvram commit; then' in remove_moon_body and
        'if ! rm -f "$zt_moon_remove_file" "$zt_moon_remove_json"; then' in remove_moon_body and
        remove_moon_body.index('if ! nvram set zerotiermoon_id="" || ! nvram commit; then') <
        remove_moon_body.index('if ! rm -f "$zt_moon_remove_file" "$zt_moon_remove_json"; then'),
        "Moon removal must back up files and persist ID clear before destructive cleanup")
require('ln -f "$zt_moon_remove_file_backup" "$zt_moon_remove_file"' in moon_remove_rollback_body and
        'ln -f "$zt_moon_remove_json_backup" "$zt_moon_remove_json"' in moon_remove_rollback_body and
        'nvram set zerotiermoon_id="$zt_moon_remove_old_id"' in moon_remove_rollback_body,
        "Moon removal rollback must restore both files and original NVRAM ID")
require('if ! creat_moon; then' in start_instance_body and
        'if ! remove_moon; then' in start_instance_body,
        "Moon setup/removal failures must reach the start result")
moon_manage_position = start_instance_body.index('if [ -n "$enablemoonserv" ]; then')
daemon_launch_position = start_instance_body.index('$PROG $args $config_path >/dev/null 2>&1 &')
rules_position = start_instance_body.index('if ! rules; then')
require(moon_manage_position < daemon_launch_position < rules_position,
        "Moon creation/removal must finish before daemon launch or firewall setup")
for marker in ("Moon creation ignored initmoon failure",
               "Moon creation accepted an unsafe endpoint replacement string",
               "Moon creation ignored endpoint-edit failure",
               "Moon creation ignored genmoon failure",
               "Moon creation ignored NVRAM set failure after write",
               "Moon creation ignored JSON publication failure",
               "valid Moon generation failed",
               "successful Moon JSON did not include the configured stable endpoint",
               "successful Moon generation did not publish the signed file",
               "JSON publication rollback retained a staging directory",
               "NVRAM set failure rollback retained a staging directory",
               "NVRAM commit failure rollback retained a staging directory",
               "partial deletion rollback retained a staging directory",
               "Moon removal ignored NVRAM set failure",
               "Moon removal ignored NVRAM commit failure",
               "Moon removal ignored partial file deletion failure",
               "partial deletion failure did not restore the Moon signature",
               "partial deletion failure did not restore the Moon JSON",
               "partial deletion failure did not restore the Moon ID",
               "Moon creation accepted an unreadable address selector",
               "Moon address read failure triggered the WAN-interface fallback",
               "Moon removal accepted an unreadable identity selector",
               "Moon removal accepted an unsafe non-hex identity path",
               "valid Moon identity was not removed",
               "empty Moon ID deleted unrelated Moon JSON"):
    require(marker in test, f"Moon lifecycle regression omits {marker}")

require("ZEROTIER_RULE_DELETE_LIMIT=32" in script and
        'zerotier_rule_delete_count' in delete_rule_body and
        'zerotier_firewall_rule_action -C "$zerotier_rule_table" "$@"' in delete_rule_body and
        'zerotier_firewall_rule_action -D "$zerotier_rule_table" "$@"' in delete_rule_body and
        'case "$zerotier_rule_table" in' in delete_rule_body and
        'iptables -t "$zerotier_firewall_action_table" "$zerotier_firewall_action" "$@"' in rule_action_body and
        'iptables "$zerotier_firewall_action" "$@"' in rule_action_body,
        "firewall deletion must check rule presence, bound duplicates and put the table selector before -C/-D")
require('1) return 0' in delete_rule_body and
        'zerotier_rule_check_status' in delete_rule_body and
        'return 1' in delete_rule_body,
        "only iptables check status 1 may be treated as an absent rule; other failures must propagate")
require('mktemp "$config_path/.firewall-state.XXXXXX"' in state_save_body and
        'chmod 600' in state_save_body and
        'mv -f "$zt_state_tmp" "$zt_state_file"' in state_save_body,
        "firewall retry identity must be atomically published with private permissions")
require('[ -f "$zt_state_file" ] && [ ! -L "$zt_state_file" ]' in state_load_body and
        'zt_state_saved_interface' in state_load_body and
        'zt_state_saved_segment' in state_load_body,
        "firewall retry state must reject symlinks and restore the validated interface/network")

require('zt_state_extra=""' in state_load_body and
        '[ -z "$zt_state_extra" ] || return 1' in state_load_body,
        "strict checkpoint loading must reject unterminated trailing bytes instead of treating read's EOF status as an empty file")


require('[ -f "$zt_state_file" ] && [ ! -L "$zt_state_file" ]' in state_recover_body and
        'IFS= read -r zt_state_saved_interface || return 1' in state_recover_body and
        'IFS= read -r zt_state_saved_segment || return 1' in state_recover_body and
        'IFS= read -r zt_state_extra || return 1' in state_recover_body and
        'zerotier_ipv4_cidr_is_valid "$zt_state_saved_segment"' in state_recover_body and
        'zt_recovery_routes="$(ip route)" || return 1' in state_recover_body and
        'if ($i == "dev" && $(i + 1) == zt_if)' in state_recover_body and
        'dev = $(i + 1)' not in state_recover_body and
        'zt_recovery_routes="$(ip route)" || return 1' in state_recover_body and
        "printf '%s\\n' \"$zt_recovery_routes\"" in state_recover_body and
        '[ "$zt_recovery_route_found" = "0" ] || return 1' in state_recover_body and
        state_recover_body.index('[ "$zt_recovery_route_found" = "0" ] || return 1') <
        state_recover_body.index('zt_firewall_state_checkpoint_loaded=1'),
        "stop-only recovery requires a clean route inventory with no route on the saved interface, complete fields, safe path and canonical tuple")


require('zerotier_ipv4_cidr_is_valid "$ip_segment"' in state_save_body and
        'zerotier_ipv4_cidr_is_valid "$zt_state_saved_segment"' in state_load_body and
        'awk -F/' in segment_body and
        'octet_count != 4' in segment_body and
        'octets[i] + 0 > 255' in segment_body and
        '$2 + 0 > 32' in segment_body and
        'length($2) > 1' in segment_body,
        "firewall checkpoint save/load must validate canonical IPv4 host/CIDR tuples before using iptables source matches")
for marker in ("stop accepted malformed source-NAT checkpoint tuple",
               "malformed checkpoint signalled the daemon",
               "malformed checkpoint changed firewall state",
               "malformed checkpoint was rewritten"):
    require(marker in test, f"firewall checkpoint regression omits {marker}")
require(del_rules_body.count('zerotier_delete_firewall_rule') == 5 and
        'del_rules_status=1' in del_rules_body and
        'return "$del_rules_status"' in del_rules_body,
        "all applicable firewall rules must be attempted and aggregate deletion failures")
require('if ! del_rules; then' in rules_section and
        rules_section.index('if ! del_rules; then') < rules_section.index('zerotier_add_firewall_rule -A INPUT'),
        "new firewall rules must not publish when stale-rule cleanup is unverified")
require('if [ -n "$zt0" ] && ! zerotier_firewall_state_save; then' in stop_body and
        stop_body.index('if [ -n "$zt0" ] && ! zerotier_firewall_state_save; then') < stop_body.index('if ! kill_z; then') <
        stop_body.index('if ! del_rules; then') < stop_body.index('if ! zero_route "del"; then'),
        "stop must persist cleanup identity first and preserve config/routes until checked firewall deletion succeeds")

require('if [ -z "$zt0" ]; then' in stop_body and
        stop_body.index('zt0="$(zerotier_interface_name)"') <
        stop_body.index('ip_segment="$(zerotier_kernel_route_segment "$zt0")"') <
        stop_body.index('if ! kill_z; then'),
        "stop must capture both the ZeroTier interface and NAT source segment before daemon shutdown")
checkpoint_load = stop_body.index("if ! zerotier_firewall_state_load; then")
checkpoint_recovery = stop_body.index("if ! zerotier_firewall_state_recover_prefix; then", checkpoint_load)
saved_interface = stop_body.index('zt0="$zt_state_saved_interface"', checkpoint_recovery)
saved_segment = stop_body.index('ip_segment="$zt_state_saved_segment"', saved_interface)
checkpoint_refresh = stop_body.index("zerotier_firewall_state_save", saved_segment)
require(
    checkpoint_load < checkpoint_recovery < saved_interface < saved_segment < checkpoint_refresh <
    stop_body.index("if ! kill_z; then"),
    "a valid or safely recovered checkpoint must remain authoritative and be atomically normalized before daemon signalling",
)

require(
    "live route can change while that older rule remains" in test
    and "stop did not delete the saved NAT source segment" in test
    and "stop deleted the newly observed route instead of the saved firewall source segment" in test,
    "regression must prove stop deletes the exact published NAT tuple when the live route changes",
)
require('if [ -e "$zt_state_file" ] || [ -L "$zt_state_file" ]; then' in del_rules_body and
        'zt0="$zt_state_saved_interface"' in del_rules_body and
        'ip_segment="$zt_state_saved_segment"' in del_rules_body and
        'if [ -z "$ip_segment" ] && [ "$zt_firewall_state_checkpoint_loaded" != 1 ]; then' in del_rules_body and
        'ip_segment="$(zerotier_kernel_route_segment "$zt0")"' in del_rules_body and
        'ZeroTier防火墙清理检查点无效' in del_rules_body,
        "firewall cleanup must honor an empty checkpoint, reuse saved segments and fail closed on corrupt state")
require('zt_firewall_state_checkpoint_loaded=1' in state_save_body and
        'zt_firewall_state_checkpoint_loaded=0' in state_load_body and
        'zt_firewall_state_checkpoint_loaded=1' in state_load_body and
        'zt_firewall_state_checkpoint_loaded=0' in state_clear_body,
        "checkpoint authority must be explicit across save, validated load and clear")
require("empty saved source segment is authoritative" in test and
        "empty checkpoint cleanup removed an unrelated source-NAT rule" in test and
        "empty checkpoint cleanup consulted or deleted the source segment from the current route" in test,
        "regression must preserve an unrelated source-NAT rule when a valid checkpoint has no source segment")
require("if grep -E -- '^-t nat -(C|D) POSTROUTING -s 10\\.148\\.18\\.0/24 -j MASQUERADE'" in test,
        "empty-checkpoint cleanup assertion must ignore fixture -A commands and reject only current-route checks/deletes")
require('if ! zerotier_read_nat_enabled; then' in stop_body and
        'if [ "$nat_enable" = "1" ] && [ -z "$ip_segment" ]; then' in stop_body and
        'return 1' in stop_body and
        stop_body.index('if ! zerotier_read_nat_enabled; then') <
        stop_body.index('if [ "$nat_enable" = "1" ] && [ -z "$ip_segment" ]; then') <
        stop_body.index('if ! kill_z; then'),
        "NAT-enabled stop must use the checked NAT selector and validate the cleanup route before daemon termination")
require('if ! kill_z; then' in stop_body and
        stop_body.index('if ! kill_z; then') < stop_body.index('del_rules') <
        stop_body.index('if ! zero_route "del"; then') <
        stop_body.index('rm -rf "$config_path"') and
        'return 1' in stop_body[stop_body.index('if ! zero_route "del"; then'):stop_body.index('rm -rf "$config_path"')],
        "failed daemon or route cleanup must preserve persistent identity/config")
require(not re.search(r'\bkill\s+-9\s+"?\$zerotier_process', script),
        "stale numeric PID must never be force-signalled")
require('pidof zerotier-one' not in kill_body and
        'killall' not in kill_body and
        'killall' not in signal_body,
        "managed ZeroTier stop must not use name-wide process signalling")
for marker in (
    "same-name impostor executable was signalled",
    "executable with mismatched argv[0] was signalled",
    "an independent ZeroTier instance was signalled",
    "stop signalled a same-name process without executable identity",
    "stop signalled a process with incomplete executable identity",
    "stop signalled with an unavailable procfs root",
    "stop escalated after process identity became UNKNOWN",
    "graceful stop did not signal only the exact daemon PID",
    "force stop did not revalidate and signal the exact PID",
):
    require(marker in test, f"exact-process stop regression omits {marker}")

require("ZT_READY_TIMEOUT=30" in script,
        "interface and kernel-route readiness must have a bounded 30-second deadline")
require("ifconfig | awk '$1 ~ /^zt/" in interface_body,
        "interface discovery must select actual ZeroTier interface names")
require('awk -v zt_if="$1"' in route_section and
        'dev == zt_if && proto == "kernel"' in route_section,
        "kernel route parsing must match exact interface and protocol fields")
require('while :' in interface_wait_body and
        'zerotier_interface_name' in interface_wait_body and
        'sleep 1' in interface_wait_body and
        '"$zerotier_wait" -ge "$ZT_READY_TIMEOUT"' in interface_wait_body,
        "interface readiness wait must be bounded and re-check at the deadline")
require('while :' in route_wait_body and
        'zerotier_kernel_route_segment "$1"' in route_wait_body and
        'sleep 1' in route_wait_body and
        '"$zerotier_wait" -ge "$ZT_READY_TIMEOUT"' in route_wait_body,
        "kernel-route readiness wait must be bounded and re-check at the deadline")
require('zerotier_wait_interface' in rules_section and
        'zerotier_wait_route "$zt0"' in rules_section and
        rules_section.index('zerotier_wait_route "$zt0"') < rules_section.index('del_rules') <
        rules_section.index('zerotier_add_firewall_rule -A INPUT'),
        "firewall rules must be published only after interface and required route readiness")
require('if iptables "$@"; then' in firewall_helper_body and
        'del_rules' in firewall_helper_body and
        'return 1' in firewall_helper_body,
        "a failed firewall append must trigger rollback and a nonzero result")
for required_rule in (
    'zerotier_add_firewall_rule -A INPUT -i "$zt0" -j ACCEPT',
    'zerotier_add_firewall_rule -A FORWARD -i "$zt0" -o "$zt0" -j ACCEPT',
    'zerotier_add_firewall_rule -A FORWARD -i "$zt0" -j ACCEPT',
    'zerotier_add_firewall_rule -t nat -A POSTROUTING -o "$zt0" -j MASQUERADE',
    'zerotier_add_firewall_rule -t nat -A POSTROUTING -s "$ip_segment" -j MASQUERADE',
):
    require(required_rule in rules_section,
            "every required INPUT/FORWARD/NAT append must use the checked firewall publisher")
require(rules_section.count('zerotier_add_firewall_rule') == 5,
        "the full three-filter/two-NAT firewall ruleset must use checked appends")
require('if ! zerotier_read_nat_enabled; then' in rules_section and
        '[ "$nat_enable" = "1" ]' in rules_section and
        'case "$nat_enable" in' in nat_settings_body,
        "NAT configuration must use the checked selector helper and a quoted policy comparison")
cleanup_branch = start_instance_body.index('if ! kill_z; then')
cleanup_failure_return = start_instance_body.index('return 1', cleanup_branch)
require('if ! rules; then' in start_instance_body and
        start_instance_body.index('if ! rules; then') < cleanup_branch < cleanup_failure_return,
        "daemon startup must fail and attempt cleanup when interface/route readiness times out")
require('if ! zero_route "add"; then' in rules_section and
        'del_rules' in rules_section and
        'zero_route "del"' in rules_section,
        "a failed static-route add must roll back the published firewall/route state")
require("grep -cE '(^-D |^-t nat -D )'" in test,
        "firewall regression must count both filter and table-qualified NAT deletes")
require('running-only' in test and
        'firewall cleanup lost the interface after daemon exit' in test and
        'static route cleanup lost the saved interface after daemon exit' in test and
        'daemon-only)' in test and
        'firewall cleanup lost the pre-stop NAT source segment' in test and
        'stop accepted iptables deletion failure' in test and
        'firewall deletion failure did not persist retry state' in test and
        'stop could not retry firewall cleanup from saved state after daemon exit' in test and
        'retry did not check the saved NAT source segment' in test and
        '"-t nat -C POSTROUTING -s 10.147.17.0/24 -j MASQUERADE"' in test and
        'stop accepted NAT-enabled shutdown without a connected route segment' in test,
        "stop regression must model disappearance of both the interface and kernel NAT source route")
missing_nat_route_case = test.split("# Missing NAT route is bounded", 1)[1].split("# Malformed NVRAM NAT state", 1)[0]
require("TEST_NAT_ENABLE=1" in missing_nat_route_case,
        "missing-kernel-route regression must explicitly enable NAT before expecting route readiness to fail")
require('if ! ip route add "$zero_ip" via "$zero_route" dev "$zt0"; then' in static_route_body and
        'return 1' in static_route_body and
        '静态路由安装失败' in static_route_body,
        "static-route add errors must propagate instead of being discarded")
require('ip route show "$zero_ip"' in static_route_body and
        'ip route del "$zero_ip" via "$zero_route" dev "$zt0"' in static_route_body and
        'route_match_status' in static_route_body and
        '静态路由删除失败' in static_route_body and
        '静态路由删除后仍然存在' in static_route_body,
        "static-route cleanup must inspect the exact tuple, propagate delete failure and verify absence")
require('function route_destination_key(value, slash)' in route_match_body and
        'route_destination_key($1) == route_destination_key(destination)' in route_match_body and
        'return value "/32"' in route_match_body and
        'substr(value, slash + 1) == "32"' in route_match_body and
        'destination_found = 1' in route_match_body and
        'route_hop_count++' in route_match_body and
        'if (route_hop_count > 1)' in route_match_body and
        'multipath_found = 1' in route_match_body and
        'if (multipath_found)' in route_match_body and
        'hop_gateway == gateway && hop_interface == interface' in route_match_body and
        'if (destination_found)' in route_match_body and
        'exit 2' in route_match_body,
        "static-route matching must normalize host routes and reject multipath aliases that Linux 4.4 cannot partially delete")
require('TEST_ROUTE_SHOW_OUTPUT' in test and
        'stop accepted a conflicting route to the configured destination' in test and
        'stop accepted and deleted a multipath route containing a configured first nexthop' in test and
        'stop accepted a multipath route containing a configured later nexthop' in test and
        'stop failed to match a /32 route displayed without its prefix' in test,
        "static-route regressions must reject first- and later-hop multipath aliases while preserving /32 display handling")

require('zerotier_kernel_route_segment "$zt0"' in cleanup_section,
        "firewall cleanup must use the same token-based route parser")

require('$(ROMFSINST) -p +x $(THISDIR)/zerotier.sh /usr/bin/zerotier.sh' in makefile,
        "ROMFS must install the exercised ZeroTier launcher")
start = function_body(services, "int start_zerotier(void)")
start_compact = "".join(start.split())
require('eval("/usr/bin/zerotier.sh","stop")' in services and
        'service_nvram_match_status("zerotier_enable","1",0,&zerotier_enabled);' in start_compact and
        'if(config_status!=0)returnconfig_status;' in start_compact and
        'if(!zerotier_enabled)return0;' in start_compact and
        'returneval("/usr/bin/zerotier.sh","start");' in start_compact and
        'int start_zerotier(void);' in rc_header and
        'nvram_match("zerotier_enable", "1")' not in start,
        "RC ZeroTier start wrapper must propagate checked enable-read and launch statuses and preserve exact disabled mode")
stop = function_body(services, "int stop_zerotier(void)")
require('return eval("/usr/bin/zerotier.sh","stop");' in stop and
        'int stop_zerotier(void);' in rc_header,
        "RC ZeroTier stop wrapper must propagate launcher status")
restart_start = services.rfind("int restart_zerotier(void)")
require(restart_start >= 0, "missing checked ZeroTier restart wrapper")
restart = function_body(services[restart_start:], "int restart_zerotier(void)")
restart_compact = " ".join(restart.split())
require('int stop_status = stop_zerotier();' in restart_compact and
        'if (stop_status != 0)' in restart_compact and
        'return stop_status;' in restart_compact and
        'return start_zerotier();' in restart_compact and
        'start_zerotier();' not in restart_compact.replace('return start_zerotier();', '') and
        restart_compact.index('return stop_status;') < restart_compact.index('return start_zerotier();'),
        "RC ZeroTier restart must preserve stop and start launcher statuses")
require('int restart_zerotier(void);' in rc_header and
        'action_result = rc_notify_merge_action_result(action_result, restart_zerotier());' in rc,
        "RC ZeroTier restart status must reach the notification result")
stop_services = function_body(services, "int\nstop_services(int stopall)")
require('record_service_stop_status(&stop_status, "ZeroTier", stop_zerotier())' in stop_services,
        "stop_services must aggregate ZeroTier stop status")

for token in ['graceful', '-9-exits', 'stuck', 'start_zero', 'stop_zero',
              'identity.secret', 'same-name impostor executable was signalled',
              'unavailable procfs root', 'exact daemon PID', 'five one-second polls',
              'interface readiness', 'kernel route', 'single-space',
              'configured static route add failed', 'zero_route:add',
              'failed static route deletion', 'unreadable static route inventory',
              'route that remained after a successful delete command']:
    require(token in test, f"regression coverage missing {token}")
require('grep -Fqx -- "-t nat -D POSTROUTING -o ztunit -j MASQUERADE" "$TEST_IPTABLES_LOG"' in test,
        "NAT append rollback must assert the table-qualified deletion command")
require("grep -cE '(^| )-A '" in test,
        "static-route rollback regression must count filter and NAT appends whose -A token occurs at different columns")

paths = [
    '.github/workflows/zerotier-lifecycle.yml',
    'scripts/check_zerotier_lifecycle.py',
    'trunk/user/zerotier/zerotier.sh',
    'trunk/user/zerotier/tests/zerotier_lifecycle.sh',
    'trunk/user/zerotier/Makefile',
    'trunk/user/rc/src/services.c',
    'trunk/user/rc/src/rc.h',
    'trunk/user/rc/src/rc.c',
]
push = workflow.split('  push:', 1)[1].split('  pull_request:', 1)[0]
pull = workflow.split('  pull_request:', 1)[1].split('permissions:', 1)[0]
for path in paths:
    token = f'      - "{path}"'
    require(push.count(token) == 1 and pull.count(token) == 1,
            f"{path} must appear exactly once in push and pull_request path filters")

print("ZeroTier bounded lifecycle, persistent-config and RC integration contracts: PASS")
