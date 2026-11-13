#!/usr/bin/env python3
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:index]
    raise AssertionError(f"unterminated function: {signature}")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


source = (ROOT / "trunk/user/frp/frp.sh").read_text()
test = (ROOT / "trunk/user/frp/tests/frp_lifecycle.sh").read_text()
services = (ROOT / "trunk/user/rc/src/services.c").read_text()
stop_test = (ROOT / "trunk/user/frp/tests/frp_stop.sh").read_text()
lock_test = (ROOT / "trunk/user/frp/tests/frp_lock.sh").read_text()
web_ex = (ROOT / "trunk/user/httpd/src/web_ex.c").read_text()
script_policy = (ROOT / "trunk/user/httpd/src/httpd_script_policy.h").read_text()
script_policy_test = (ROOT / "trunk/user/httpd/src/tests/httpd_script_policy.c").read_text()
copy_scripts = (ROOT / "trunk/user/scripts/files/usr/bin/copyscripts.sh").read_text()
frp_ui = (ROOT / "trunk/user/www/n56u_ribbon_fixed/Advanced_frp.asp").read_text()
workflow = (ROOT / ".github/workflows/frp-lifecycle.yml").read_text()

check = function_body(source, "check_frp ()")
require(check.count("frp_start") == 1,
        "check_frp must dispatch no more than one restart per watchdog pass")
require(check.count("need_start=1") == 2,
        "check_frp must aggregate the frpc and frps missing states")
require(check.index('if [ "$frpc_enable" = "1" ]') <
        check.index('if [ "$frps_enable" = "1" ]') <
        check.index('if [ "$need_start" = "1" ]') <
        check.index("frp_start"),
        "the two service states must be checked before a single restart")
require('if [ "$result_net" = "1" ]' in check,
        "FRP restart must remain gated on successful WAN reachability")

autostart_source = (ROOT / "trunk/user/scripts/files/usr/bin/autostart.sh").read_text()
autostart = function_body(autostart_source, "frp_autostart()")
require('frp_client_enabled=$(nvram get frpc_enable)' in autostart and
        'frp_server_enabled=$(nvram get frps_enable)' in autostart and
        '[ "$frp_client_enabled" = "1" ] || [ "$frp_server_enabled" = "1" ]' in autostart,
        "boot must start FRP when either frpc or frps is enabled")
require("frp_launch" in autostart and
        "if [ $(nvram get frpc_enable) = 1 ]" not in autostart_source,
        "FRP boot activation must use the single shared launcher for either role")
autostart_test = (ROOT / "trunk/user/frp/tests/frp_autostart.sh").read_text()
for marker in ("client-only configuration", "server-only configuration",
               "both services enabled", "both services disabled",
               "exactly once"):
    require(marker in autostart_test, f"FRP autostart regression omits {marker}")

cron = function_body(source, "frp_manage_watchdog_cron()")
for marker in ("add|remove", "A-Za-z0-9_.-", "FRP_CRONTAB_DIR", "FRP_CRON_ROOT",
               "mktemp", "umask 077", "awk -v managed=", "chmod 0600", "mv -f"):
    require(marker in cron, f"managed FRP crontab transaction omits {marker}")
require("sed -i '/frp/d'" not in source and "cat >> /etc/storage/cron/crontabs/" not in source,
        "FRP must not delete arbitrary cron entries using substring matching")
require('FRP_WATCHDOG_CRON=' in source and
        'awk -v managed="$FRP_WATCHDOG_CRON"' in cron,
        "FRP watchdog cleanup must match only its exact managed line")
start = function_body(source, "frp_start ()")
disabled_guard = 'if [ "$frpc_enable" = "0" ] && [ "$frps_enable" = "0" ]; then'
require(disabled_guard in start and
        start.index(disabled_guard) < start.index("frp_run_script") and
        "frp_close" in start[start.index(disabled_guard):start.index("frp_run_script")],
        "a start request with both FRP roles disabled must clean up without running user code")
require(start.index("frp_run_script") < start.index("frp_manage_watchdog_cron add"),
        "do not publish a watchdog when the configured FRP start script fails")
close = function_body(source, "frp_close ()")
require("frp_manage_watchdog_cron remove" in close,
        "disabling both services must remove the exact managed watchdog")
services = (ROOT / "trunk/user/rc/src/services.c").read_text()
restart_services = function_body(services, "void restart_frp(void)")
require('eval("/usr/bin/frp.sh","restart")' in restart_services and
        "FRP restart failed" in restart_services and
        'eval("/usr/bin/frp.sh","stop")' not in restart_services and
        'eval("/usr/bin/frp.sh","start")' not in restart_services,
        "RC restart must delegate stop/start to one serialized launcher action")
require('eval("/usr/bin/frp.sh","stop")' in services and
        'eval("/usr/bin/frp.sh","start")' in services,
        "RC stop/start must use the installed FRP launcher")
rc = (ROOT / "trunk/user/rc/src/rc.c").read_text()
require('strcmp(entry->d_name, RCN_RESTART_FRP) == 0' in rc and
        "restart_frp();" in rc,
        "PID 1 must dispatch the FRP restart notification")
variables = (ROOT / "trunk/user/httpd/src/variables.c").read_text()
variables_normalized = " ".join(variables.split())
for marker in ('{"frpc_enable", "", NULL, EVM_RESTART_FRP}',
               '{"frps_enable", "", NULL, EVM_RESTART_FRP}',
               '{"scripts.frp_script.sh", "File", NULL, EVM_RESTART_FRP}',
               '{EVM_RESTART_FRP, EVT_RESTART_FRP, RCN_RESTART_FRP, 0}'):
    require(" ".join(marker.split()) in variables_normalized,
            f"HTTPD FRP persistence mapping omits {marker}")
ui = (ROOT / "trunk/user/www/n56u_ribbon_fixed/Advanced_frp.asp").read_text()
require('value="FrpConf;"' in ui and
        'name="scripts.frp_script.sh"' in ui and
        'name="frpc_enable"' in ui and 'name="frps_enable"' in ui,
        "FRP UI must expose both enable flags and the persisted custom script")
copy_scripts = (ROOT / "trunk/user/scripts/files/usr/bin/copyscripts.sh").read_text()
require('cp -f /etc_ro/frp_script.sh "$frp_script_path"' in copy_scripts and
        'chmod 700 "$frp_script_path"' in copy_scripts and
        '[ -L "$frp_script_path" ]' in copy_scripts and
        '[ ! -f "$frp_script_path" ]' in copy_scripts,
        "boot must seed the persistent FRP script safely and enforce mode 0700")
frp_template = (ROOT / "trunk/user/frp/frp_script.sh").read_text()
frp_config_test = (ROOT / "trunk/user/frp/tests/frp_config.sh").read_text()
require('frpc -c "$frp_runtime_dir/myfrpc.ini"' in frp_template and
        'frps -c "$frp_runtime_dir/myfrps.ini"' in frp_template,
        "persistent FRP script must retain both client and server launch paths")
cron_test = (ROOT / "trunk/user/frp/tests/frp_crontab.sh").read_text()
for marker in ("frp_start", "frp_close", "operator", "path-traversal",
               "symlink crontab", "atomic rename failure", "mode 0600",
               "FRP script runner rejects symlink and non-regular targets: PASS",
               "failed start script", "overlong username", "Removing a missing watchdog",
               "disabled start executed the custom FRP script",
               "server-only start did not invoke the custom script once"):
    require(marker in cron_test, f"FRP managed crontab regression omits {marker}")


refresh = function_body(source, "frp_refresh_nvram_state()")
require('nvram get frpc_enable' in refresh and 'nvram get frps_enable' in refresh and
        'nvram get http_username' in refresh and '0|1' in refresh,
        "mutable FRP role and cron state must be read and validated under the lifecycle lock")
lock_prepare = function_body(source, "frp_lifecycle_lock_prepare()")
for marker in ("stat -c '%u %a'", "stat -c '%u %a %h'", "set -C",
               '[ -L "$frp_lock_file" ]', '[ "$2" != 600 ]', '[ "$3" != 1 ]'):
    require(marker in lock_prepare, f"FRP lifecycle lock path validation omits {marker}")
lock_execute = function_body(source, "frp_lifecycle_lock_execute()")
for marker in ("flock -n 9", "frp_lock_wait_seconds", "exec 9>&-",
               "生命周期锁FD 9已被占用", "等待生命周期锁超时"):
    require(marker in lock_execute, f"FRP lifecycle lock execution omits {marker}")
require(lock_execute.index("flock -n 9") < lock_execute.index('"$@"') <
        lock_execute.index('exec 9>&-'),
        "FRP lifecycle lock must enclose the complete operation and release afterward")
lock_action = function_body(source, "frp_lifecycle_locked_action()")
require("frp_refresh_nvram_state" in lock_action and
        "frp_close || return $?" in lock_action and
        "frp_start" in lock_action and "check_frp" in lock_action,
        "start/stop/restart/watchdog actions must share one refreshed locked dispatcher")
lock_run = function_body(source, "frp_lifecycle_lock_run()")
require("frp_lifecycle_lock_prepare" in lock_run and
        "command -v flock" in lock_run and
        "frp_lifecycle_lock_execute" in lock_run and
        "start|stop|restart|C" in lock_run,
        "every external FRP lifecycle action must fail closed behind the verified flock")
run_script = function_body(source, "frp_run_script()")
require('/bin/sh "$frp_script_path" 9>&-' in run_script,
        "operator script and daemon children must not retain the launcher lock descriptor")
for marker in ("stable private inode", "symlink lock path", "hard-linked lock path",
               "contended lock action", "action failure status"):
    require(marker in lock_test, f"FRP lifecycle lock regression omits {marker}")

network = function_body(source, "check_net()")
require("if /bin/ping " in network and "== " not in network,
        "WAN probe must use a POSIX if/status check")
require(network.index('logger -t "frp"') < network.index("return 2"),
        "WAN failure log must not be unreachable after return")

for marker in (
    'expect_start_count "both enabled services missing" 1',
    'expect_start_count "client missing" 1',
    'expect_start_count "server missing" 1',
    'expect_start_count "both services alive" 0',
    'expect_start_count "all services disabled" 0',
    'expect_start_count "WAN unavailable" 0',
    "Deliberately leave both PIDs absent",
):
    require(marker in test, f"missing asynchronous lifecycle regression: {marker}")

paths = (
    ".github/workflows/frp-lifecycle.yml",
    "trunk/user/frp/frp.sh",
    "trunk/user/frp/tests/frp_lifecycle.sh",
    "trunk/user/frp/tests/frp_crontab.sh",
    "trunk/user/frp/tests/frp_stop.sh",
    "trunk/user/frp/tests/frp_lock.sh",
    "trunk/user/frp/tests/frp_autostart.sh",
    "trunk/user/frp/tests/frp_config.sh",
    "trunk/user/scripts/files/usr/bin/autostart.sh",
    "trunk/user/frp/frp_script.sh",
    "trunk/user/scripts/files/usr/bin/copyscripts.sh",
    "trunk/user/rc/src/services.c",
    "trunk/user/rc/src/rc.c",
    "trunk/user/httpd/src/variables.c",
    "trunk/user/www/n56u_ribbon_fixed/Advanced_frp.asp",
    "scripts/check_frp_lifecycle.py",
)
filters = workflow.split("jobs:", 1)[0]
push_filters, pull_filters = filters.split("  pull_request:", 1)
for path in paths:
    token = f'      - "{path}"'
    require(push_filters.count(token) == 1 and pull_filters.count(token) == 1,
            f"{path} must appear exactly once in push and pull_request path filters")

run_script = function_body(source, "frp_run_script()")
require('[ -L "$frp_script_path" ]' in run_script and
        '[ ! -f "$frp_script_path" ]' in run_script and
        '/bin/sh "$frp_script_path" 9>&-' in run_script,
        "FRP runner must refuse symlink/non-regular custom scripts before root execution")

# Shutdown/reboot must stop both daemons regardless of their enable flags.
close = function_body(source, "frp_close ()")
require("frp_manage_watchdog_cron remove" in close and
        close.index("frp_manage_watchdog_cron remove") < close.index("frp_signal_all TERM") and
        "frp_wait_stopped 5" in close and
        "frp_signal_all KILL" in close and "frp_wait_stopped 2" in close and
        "frp_close_status" in close and "frpc_enable" not in close and
        "frps_enable" not in close,
        "FRP close must remove its watchdog and confirm both daemon exits with bounded TERM/KILL, independent of enable flags")
require("frp_any_running" in source and 'killall "-$1" frpc frps' in source,
        "FRP shutdown must signal only its daemon processes")
stop = function_body(services, "void stop_frp(void)")
start = function_body(services, "void start_frp(void)")
restart = function_body(services, "void restart_frp(void)")
require("status != 0" in stop and "FRP stop failed" in stop and
        "status != 0" in start and "FRP start failed" in start,
        "RC stop/start wrappers must report FRP command failures")
require('eval("/usr/bin/frp.sh","restart")' in restart and
        "FRP restart failed" in restart and
        'eval("/usr/bin/frp.sh","stop")' not in restart and
        'eval("/usr/bin/frp.sh","start")' not in restart,
        "RC restart must use the serialized launcher action and preserve the stop-before-start gate")
for marker in ("enabled FRPC survived stop", "enabled FRPS survived stop",
               "KILL escalation", "stubborn FRPS", "cron removal",
               "five-plus-two poll bound"):
    require(marker in stop_test, f"FRP stop regression omits {marker}")

# FRP private configuration must not be exposed as a world-readable script.
require("# The persistent FRP script contains tokens and dashboard credentials." in copy_scripts and
        'chmod 700 "$frp_script_path"' in copy_scripts and
        '[ -L "$frp_script_path" ]' in copy_scripts and
        '[ ! -f "$frp_script_path" ]' in copy_scripts,
        "boot must reject unsafe FRP script paths and enforce mode 0700 without overwriting user content")
require("HTTPD_FILE_TYPE_PRIVATE_SCRIPT" in script_policy and
        "httpd_script_file_mode" in script_policy and "0700" in script_policy and
        "httpd_script_file_read_allowed" in script_policy and
        'strcmp(name, "frp_script.sh")' in script_policy,
        "HTTPD must apply FRP-specific private-script read, write and mode policy")
require('httpd_script_file_type(file_name)' in web_ex and
        'get_login_private_key_access()' in web_ex and
        'lstat(real_path, &target_stat)' in web_ex and
        'S_ISREG(target_stat.st_mode)' in web_ex and
        '/.frp_script.XXXXXX' in web_ex and
        'chmod(temp_path, httpd_script_file_mode(file_type))' in web_ex and
        'rename(temp_path, real_path)' in web_ex,
        "HTTPD must stage FRP script privately, reject symlinks/non-regular targets and atomically publish mode 0700")
dump = function_body(web_ex, "dump_file(webs_t wp, char *filename)")
require('STORAGE_SCRIPTS_DIR "/frp_script.sh"' in dump and
        "httpd_script_file_read_allowed" in dump and
        "get_login_private_key_access()" in dump,
        "FRP script dump must hide credentials from off-LAN peers, including HTTPS sessions")
hook = function_body(web_ex, "static int login_state_hook")
require("function login_private_key_access()" in hook and
        "get_login_private_key_access()" in hook,
        "UI must receive the trusted-network-only secret access state")
require("login_private_key_access()" in frp_ui and
        "textarea_scripts_enabled(0)" in frp_ui and
        "仅LAN/VPN可查看和修改" in frp_ui,
        "FRP UI must disable the secret script editor outside the trusted LAN/VPN")
for marker in ("HTTPD_FILE_TYPE_PRIVATE_SCRIPT", "httpd_script_file_type",
               "httpd_script_file_mode", "httpd_script_file_read_allowed",
               "frp_script.sh", "0700", "0755"):
    require(marker in script_policy_test,
            f"HTTPD policy regression omits FRP private-script contract {marker}")

runtime_dir = function_body(frp_template, "frp_prepare_runtime_dir()")
require('[ -L "$frp_check_dir" ]' in runtime_dir and
        'ls -lnd "$frp_check_dir"' in runtime_dir and
        '[ "$3" = "$frp_expected_owner" ]' in runtime_dir and
        'chmod 700 "$frp_check_dir"' in runtime_dir,
        "FRP runtime directory must reject symlinks/non-root ownership and enforce mode 0700")
validate_credentials = function_body(frp_template, "frp_validate_credentials()")
require("CHANGE_ME_TO_A_UNIQUE_RANDOM_TOKEN" in validate_credentials and
        "CHANGE_ME_TO_A_UNIQUE_PASSWORD" in validate_credentials and
        '${#frp_token}' in validate_credentials and
        '${#frp_dashboard_password}' in validate_credentials,
        "FRP must fail closed on sample credentials and short/unsafe values")
client_stage = function_body(frp_template, "frp_stage_client_config()")
server_stage = function_body(frp_template, "frp_stage_server_config()")
for body, name in ((client_stage, "myfrpc.ini"), (server_stage, "myfrps.ini")):
    require(f'mktemp "$frp_runtime_dir/.{name}.XXXXXX"' in body and
            'chmod 0600' in body and 'cat > "$frp_' in body,
            f"FRP {name} must be written to a private same-directory stage")
publish = function_body(frp_template, "frp_publish_staged_config()")
require("frp_validate_config_target" in publish and
        'mv -f "$frp_stage_path" "$frp_config_target"' in publish and
        '[ ! -L "$frp_stage_path" ]' in publish,
        "FRP config publication must reject unsafe targets and atomically rename a private staged file")
for marker in ("FRP private runtime directory and credential config publication: PASS",
               "FRP failed staging/rename preserves last-good config: PASS",
               "FRP rejects symlink and non-regular runtime paths: PASS"):
    require(marker in frp_config_test, f"FRP config regression omits {marker}")
print("FRP asynchronous restart, private credential storage and workflow contracts: PASS")
