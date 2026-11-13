#!/usr/bin/env python3
"""Check Adbyby dnsmasq hosts cleanup and its regression workflow."""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "trunk/user/adbyby/adbyby.sh"
TEST = ROOT / "trunk/user/adbyby/tests/hosts_config_preservation.sh"
FIREWALL_TEST = ROOT / "trunk/user/adbyby/tests/firewall_snapshot.sh"
WORKFLOW = ROOT / ".github/workflows/adbyby-config-stability.yml"
SERVICE = ROOT / "trunk/user/rc/src/services.c"
LIFECYCLE_TEST = ROOT / "trunk/user/adbyby/tests/lifecycle_lock.sh"
RULES_TEST = ROOT / "trunk/user/adbyby/tests/rules_download.sh"
EXTERNAL_TEST = ROOT / "trunk/user/adbyby/tests/external_downloads.sh"
ANTI_AD_TEST = ROOT / "trunk/user/adbyby/tests/anti_ad_download.sh"
HOST_LIST_TEST = ROOT / "trunk/user/adbyby/tests/host_list_download.sh"
HOST_DEFAULT = ROOT / "trunk/user/adbyby/adbyby_host.sh"
ADBYBY_MAKEFILE = ROOT / "trunk/user/adbyby/Makefile"
HOST_UI = ROOT / "trunk/user/www/n56u_ribbon_fixed/Advanced_adbyby.asp"
HOST_ACTION = ROOT / "trunk/user/www/n56u_ribbon_fixed/Ad_action.asp"
HTTPD_VARIABLES = ROOT / "trunk/user/httpd/src/variables.c"
COPY_SCRIPTS = ROOT / "trunk/user/scripts/files/usr/bin/copyscripts.sh"


def fail(message):
    print(f"Adbyby hosts configuration contract: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def require(condition, message):
    if not condition:
        fail(message)


def function_body(source, name):
    match = re.search(rf"(?ms)^{re.escape(name)}\s*\(\)\s*\{{(.*?)^\}}", source)
    if not match:
        fail(f"missing function {name}()")
    return match.group(1)


source = SOURCE.read_text(encoding="utf-8")
test = TEST.read_text(encoding="utf-8")
firewall_test = FIREWALL_TEST.read_text(encoding="utf-8")
workflow = WORKFLOW.read_text(encoding="utf-8")
service = SERVICE.read_text(encoding="utf-8")
lifecycle_test = LIFECYCLE_TEST.read_text(encoding="utf-8")
rules_test = RULES_TEST.read_text(encoding="utf-8")
external_test = EXTERNAL_TEST.read_text(encoding="utf-8")
anti_ad_test = ANTI_AD_TEST.read_text(encoding="utf-8")
host_list_test = HOST_LIST_TEST.read_text(encoding="utf-8")
host_default = HOST_DEFAULT.read_text(encoding="utf-8")
adbyby_makefile = ADBYBY_MAKEFILE.read_text(encoding="utf-8")
host_ui = HOST_UI.read_text(encoding="utf-8")
host_action = HOST_ACTION.read_text(encoding="utf-8")
httpd_variables = HTTPD_VARIABLES.read_text(encoding="utf-8")
copy_scripts = COPY_SCRIPTS.read_text(encoding="utf-8")
helper = function_body(source, "adbyby_remove_generated_hosts_config")
hosts_ads = function_body(source, "hosts_ads")
del_dns = function_body(source, "del_dns")
lock_prepare = function_body(source, "adbyby_lifecycle_lock_prepare")
lock_execute = function_body(source, "adbyby_lifecycle_lock_execute")
locked_action = function_body(source, "adbyby_lifecycle_locked_action")
lock_run = function_body(source, "adbyby_lifecycle_lock_run")
valid_url = function_body(source, "adbyby_valid_https_url")
valid_md5 = function_body(source, "adbyby_valid_md5")
md5_file = function_body(source, "adbyby_md5_file")
manifest_digest = function_body(source, "adbyby_manifest_digest")
download_https = function_body(source, "adbyby_download_https_200")
add_rules = function_body(source, "add_rules")
user_cache = function_body(source, "adbyby_user_rules_cache_is_usable")
prepare_user_rules = function_body(source, "adbyby_prepare_user_rules")
prepare_user_transaction = function_body(source, "adbyby_prepare_user_rules_transaction")
anti_ad_validate = function_body(source, "adbyby_validate_dnsmasq_fragment")
anti_ad_cache = function_body(source, "adbyby_anti_ad_cache_is_usable")
prepare_anti_ad = function_body(source, "adbyby_prepare_anti_ad")
prepare_anti_ad_transaction = function_body(source, "adbyby_prepare_anti_ad_transaction")
anti_ad = function_body(source, "anti_ad")
anti_ad_remove = function_body(source, "adbyby_remove_anti_ad_fragment")
anti_ad_cleanup = function_body(source, "adbyby_cleanup_dnsmasq_ad_dir")
host_validator = function_body(source, "adbyby_validate_hosts_file")
host_cache = function_body(source, "adbyby_hosts_cache_is_usable")
host_transaction_match = re.search(r"(?ms)^adbyby_prepare_hosts_transaction\(\)[ \t]*\n\((.*?)^\)", source)
if not host_transaction_match:
    fail("missing subshell function adbyby_prepare_hosts_transaction()")
host_transaction = host_transaction_match.group(1)
add_dns = function_body(source, "add_dns")
del_dns = function_body(source, "del_dns")
start = function_body(source, "adbyby_start")
updater = function_body(source, "adbyby_uprules")
refresh_match = re.search(r"(?ms)^adbyby_refresh_managed_rules\(\)[ \t]*\n\((.*?)^\)", source)
if not refresh_match:
    fail("missing subshell function adbyby_refresh_managed_rules()")
refresh_rules = refresh_match.group(1)
restart_match = re.search(r"(?ms)void\s+restart_adbyby\s*\(\s*void\s*\)\s*\{(.*?)^\}", service)
if not restart_match:
    fail("missing C function restart_adbyby(void)")
restart_adbyby = restart_match.group(1)

require("/etc/storage/dnsmasq/dnsmasq.conf" in helper and
        "$PROG_PATH/hosts" in helper,
        "the helper default must target the generated Adbyby hosts file")
require('adbyby_hosts_entry="addn-hosts=$adbyby_hosts_file"' in helper,
        "cleanup target must be the exact Adbyby addn-hosts directive")
require("sed 's/" in helper and r"\\&/g" in helper,
        "the full path must be escaped before use as a BRE pattern")
require(r'sed -i "/^$adbyby_escaped_hosts_entry\$/d"' in helper,
        "cleanup must use an anchored exact-line match")
require("sed -i '/hosts/d'" not in source and 'sed -i "/hosts/d"' not in source,
        "unbounded /hosts/d deletion remains")
require(hosts_ads.count("adbyby_remove_generated_hosts_config") == 2,
        "hosts_ads must remove the managed entry both on fail-closed and publication paths")
require(del_dns.count("adbyby_remove_generated_hosts_config") == 1,
        "del_dns must call the scoped cleanup helper exactly once")
append_index = hosts_ads.find('>> "$ADBYBY_HOSTS_CONFIG_FILE"')
require(append_index >= 0 and
        hosts_ads.index("adbyby_remove_generated_hosts_config") < append_index,
        "hosts_ads must clean the old entry before appending the replacement")
require("return 1" in hosts_ads[:append_index],
        "hosts_ads must fail closed when it cannot remove the prior entry")

require("ADBYBY_HOST_LIST_FILE" in source and "/etc/storage/adbyby_host.sh" in source,
        "the configured host-list path must remain the persistent storage file")
require("ADBYBY_HOST_SOURCE_MAX_BYTES=4194304" in source and
        "ADBYBY_HOST_TOTAL_MAX_BYTES=16777216" in source and
        "ADBYBY_HOST_MAX_SOURCES=16" in source,
        "host-list downloads need explicit per-source, aggregate and source-count bounds")
require("adbyby_download_https_200" in host_transaction and
        "ADBYBY_HOST_SOURCE_MAX_BYTES" in host_transaction,
        "every configured host source must use the shared CA-verified HTTPS/HTTP-200 downloader")
require("while IFS= read -r adbyby_hosts_url" in host_transaction and
        '"$adbyby_hosts_url"' in host_transaction,
        "host source URLs must be read one line at a time and quoted as a single argument")
require("mktemp -d" in host_transaction and
        'mktemp -d "$PROG_PATH/.adbyby-hosts.XXXXXX"' in host_transaction and
        "umask 077" in host_transaction,
        "host downloads require unique private staging on the destination filesystem")
require("adbyby_validate_hosts_file" in host_transaction and
        "ADBYBY_HOST_TOTAL_MAX_BYTES" in host_transaction and
        "LC_ALL=C sort" in host_transaction and "uniq " in host_transaction and
        'mv -f "$adbyby_hosts_stage/hosts.next" "$adbyby_hosts_destination"' in host_transaction,
        "all sources must be syntactically validated, size-aggregated, deduplicated and atomically published")
require("[ ! -L" in host_transaction and "chmod 0644" in host_transaction,
        "the published host file must reject symlinks and have a readable mode")
require("adbyby_validate_hosts_file" in host_cache and
        '[ -s "$PROG_PATH/hosts" ]' in host_cache,
        "last-good fallback must be a non-empty validated regular host file")
require("adbyby_prepare_hosts_transaction" in hosts_ads and
        "adbyby_hosts_cache_is_usable" in hosts_ads and
        "保留并继续使用上一次完整列表" in hosts_ads,
        "failed refreshes must preserve and continue using a valid last-good list")
require("curl -k" not in hosts_ads and "/tmp/host.txt" not in source and
        "hostlist.txt" not in source,
        "legacy insecure/shared host download scratch paths must be removed")
require("grep -v '^#' /etc/storage/adbyby_host.sh" not in source and
        "rm -rf $PROG_PATH/hosts" not in source,
        "the updater must not split URLs with shell word splitting or delete the live list first")
for marker in ("URL splitting", "TLS, redirect and HTTP failures",
               "Invalid schemes and whitespace", "missing firmware CA",
               "Malformed and oversized", "Aggregate size", "last-good fallback",
               "publication failure", "stale /tmp/host.txt",
               "configured hosts URL splitting, bounded HTTPS"):
    require(marker.lower() in host_list_test.lower(),
            f"host-list regression misses: {marker}")
require("https://cdn.jsdelivr.net/gh/VeleSila/yhosts/hosts" in host_default,
        "the ROMFS default host-list URL is missing")
require('$(ROMFSINST) /etc_ro/adbyby_host.sh' in adbyby_makefile,
        "the default host-list template is not installed into ROMFS")
require('name="scripts.adbyby_host.sh"' in host_ui and
        'nvram_dump("scripts.adbyby_host.sh","")' in host_ui and
        'name="hosts_ad"' in host_ui,
        "the Adbyby UI no longer exposes the stored source list and enable toggle")
require('{"scripts.adbyby_host.sh", "File", NULL, EVM_RESTART_ADBYBY}' in httpd_variables,
        "HTTPD must persist the configured host source list as a file and restart Adbyby")
require('cp -rf /etc_ro/adbyby_host.sh /etc/storage/' in copy_scripts,
        "boot-time default seeding must preserve the operator's configured host list")
require("<% adbyby_action(); %>" in host_action,
        "manual Adbyby action dispatch endpoint is missing")
require("hosts_ads" in start and "hosts_ads" in updater and
        "F) hosts_ads" in locked_action,
        "startup, updater and locked manual refresh must all use the same host-list transaction")
require('logger -t "adbyby"' in del_dns,
        "del_dns must log a failed managed-entry cleanup")
require("adbyby_refresh_nvram_state" in firewall_test,
        "firewall host fixture must initialize the launcher state before direct helper calls")

require("stat -c '%u %a %h'" in lock_prepare and '[ -L "$adbyby_lock_file" ]' in lock_prepare,
        "lifecycle lock validates owner, mode, links and symlink state")
require('"$2" != 600' in lock_prepare and '"$3" != 1' in lock_prepare,
        "lifecycle lock rejects permissive or hard-linked files")
require("flock -n 9" in lock_execute and "exec 9>&-" in lock_execute,
        "lifecycle lock uses bounded FD locking and closes FD 9")
require("rm -f" not in lock_prepare,
        "lifecycle lock inode must remain stable")
require('ADBYBY_CA_BUNDLE="/etc/ssl/certs/ca-certificates.crt"' in source,
        "managed downloads must use the firmware CA bundle compiled into curl")
require("https://" in valid_url and "grep -Eq" in valid_url and "2048" in valid_url and
        "return 1" in valid_url,
        "configured third-party sources must be bounded, whitespace-free HTTPS URLs")
for option in ("--proto '=https'", "--max-filesize", "--limit-rate 512k",
               "--connect-timeout 5", "--max-time 30", "--retry 3",
               '[ "$adbyby_download_status" = 200 ]'):
    require(option in download_https, f"shared HTTPS downloader omits {option}")
require("wc -c" in download_https and "adbyby_download_max_bytes" in download_https,
        "shared downloader must enforce a post-transfer byte cap")
require("[[:xdigit:]]" in manifest_digest and r"\\{32\\}" in manifest_digest,
        "manifest parser must accept only complete hexadecimal MD5 fields")
require("32" in valid_md5 and "0123456789abcdefABCDEF" in valid_md5,
        "MD5 values must be exactly 32 hexadecimal characters")
require("md5sum" in md5_file and "[ ! -L \"$1\" ]" in md5_file and "[ -s \"$1\" ]" in md5_file,
        "local and staged rules must be regular, non-symlink, non-empty files before hashing")
for option in ("--cacert", "--fail", "--connect-timeout 5", "--max-time 30",
               "--retry 3", "--write-out '%{http_code}'", '[ "$adbyby_download_status" = 200 ]'):
    require(option in download_https, f"managed HTTPS fetch omits {option}")
require("-k" not in download_https and "--insecure" not in download_https,
        "managed rule downloads must retain TLS certificate verification")
require("adbyby_valid_https_url" in download_https and
        "[ -r \"$ADBYBY_CA_BUNDLE\" ]" in download_https and "[ -s \"$ADBYBY_CA_BUNDLE\" ]" in download_https,
        "managed fetch must apply the strict HTTPS URL validator and require the firmware trust store")
for marker in ("umask 077", "mktemp -d /tmp/adbyby-rules.XXXXXX",
               "adbyby_rules_cache_is_usable", "adbyby_manifest_digest",
               "adbyby_lazy_downloaded", "adbyby_video_downloaded",
               "lazy.previous", "mv -f", "adbyby_keep_stage=1"):
    require(marker in refresh_rules, f"managed update transaction is missing: {marker}")
require("return 0" in refresh_rules and "保留现有规则" in refresh_rules,
        "remote fetch/manifest/digest failures must preserve a usable cached pair")
require("adbyby_refresh_managed_rules" in add_rules and "return 1" in add_rules,
        "add_rules must stop processing when no safe managed rule state exists")
require("adbyby_prepare_user_rules" in add_rules and "return 1" in add_rules and
        "user2.txt" not in source and "curl -k" not in add_rules,
        "third-party rule failures must gate publication and never use shared insecure downloads")
require("user.txt" in user_cache and "[ ! -L" in user_cache and "[ -s" in user_cache,
        "last-good user rules cache must be a non-empty regular non-symlink file")
require("umask 077" in prepare_user_rules,
        "third-party user-rule staging must set a private umask before creating the stage directory")
for marker in ("mktemp -d /tmp/adbyby-user-rules.XXXXXX",
               "adbyby_download_https_200", "1048576", "user.next",
               "rule-", "chmod 0644", "mv -f", "user.bin"):
    require(marker in prepare_user_transaction, f"third-party user-rule transaction omits {marker}")
require("adbyby_user_rules_cache_is_usable" in prepare_user_transaction and
        "保留上一次完整规则" in prepare_user_transaction and "return 0" in prepare_user_transaction,
        "failed third-party fetches must preserve a last-good user.txt cache")
require("adbyby_prepare_user_rules_transaction" in prepare_user_rules,
        "user-rule staging must isolate umask and cleanup in a subshell")
require('ADBYBY_ANTI_AD_FILE="/etc/storage/dnsmasq-adbyby.d/anti-ad-for-dnsmasq.conf"' in source,
        "anti-AD fragment path must remain the persistent dnsmasq include")
require("adbyby_valid_https_url" in prepare_anti_ad_transaction and
        "adbyby_download_https_200" in prepare_anti_ad_transaction and
        "16777216" in prepare_anti_ad_transaction,
        "anti-AD must use the shared bounded HTTPS/CA/status downloader")
require("mktemp -d" in prepare_anti_ad_transaction and
        "adbyby_anti_ad_dir" in prepare_anti_ad_transaction and
        "mv -f" in prepare_anti_ad_transaction and "chmod 0644" in prepare_anti_ad_transaction,
        "anti-AD staging must share the target filesystem and publish by rename")
require("[ ! -L" in prepare_anti_ad_transaction and "[ ! -L" in anti_ad_cache,
        "anti-AD must reject symlink and non-regular cache destinations")
for marker in ("address=", "253", "63", "512", "16777216", "count < 1"):
    require(marker in anti_ad_validate, f"anti-AD validator omits {marker}")
require("adbyby_anti_ad_cache_is_usable" in anti_ad and
        "anti_ad_count" in anti_ad and "保留上一次有效规则" in anti_ad,
        "anti-AD update failures must retain valid last-good data and count")
require("curl -k" not in anti_ad and "adbyby_prepare_anti_ad" in anti_ad,
        "anti-AD must not use insecure direct downloads")
require("adbyby_remove_anti_ad_fragment" in anti_ad and
        'rm -f "$adbyby_remove_file"' in anti_ad_remove and
        "nvram set anti_ad_count=0" in anti_ad,
        "disabling anti-AD must remove the live fragment and reset its count")
require("adbyby_cleanup_dnsmasq_ad_dir" in del_dns and
        "adbyby_anti_ad_cache_is_usable" in anti_ad_cleanup and
        "ADBYBY_ANTI_AD_FILE" in anti_ad_cleanup and
        "adbyby_keep_anti_ad" in anti_ad_cleanup,
        "stop cleanup must retain only a verified enabled anti-AD last-good fragment")
require("if ! anti_ad; then" in add_dns and
        "继续处理其他dnsmasq规则" in add_dns and
        "awk" in add_dns,
        "optional anti-AD failures must not suppress unrelated Adbyby DNS rules")
require("rm -f /etc/storage/dnsmasq-adbyby.d/*" not in del_dns,
        "stop cleanup must not unconditionally delete the anti-AD last-good file")
for marker in ("http_redirect", "malformed", "bad_label", "empty", "missing CA bundle",
               "destination symlink", "disabled anti-AD fragment remained active",
               "enabled last-good", "stop cleanup", "publication failure", "last-good count",
               "Adbyby anti-ad HTTPS fragment validation"):
    require(marker in anti_ad_test, f"anti-AD regression misses: {marker}")
for marker in ("Query delimiters", "Invalid schemes and whitespace",
               "TLS, redirect and HTTP failures", "Response size", "last-good",
               "Adbyby HTTPS-only third-party URL validation"):
    require(marker in external_test, f"third-party download regression misses: {marker}")
require("if ! add_rules" in start and "return 1" in start,
        "normal start must not launch the daemon after managed rule preparation fails")
require("if ! add_rules" in updater and "return 1" in updater,
        "update/restart must not relaunch after managed rule preparation fails")
for fixed_path in ("/tmp/local-md5.json", "/tmp/md5.json", "/tmp/lazy.txt", "/tmp/video.txt"):
    require(fixed_path not in source, f"predictable shared staging path remains: {fixed_path}")
for marker in ("TLS outage", "redirect", "HTTP failure", "partial/corrupt body",
               "second rename failure", "Missing CA", 
               "Adbyby private HTTPS staging, digest validation, cache fallback and publication rollback"):
    require(marker in rules_test, f"rule download regression misses: {marker}")

require("adbyby_refresh_nvram_state || return 1" in locked_action,
        "NVRAM must refresh after lock acquisition")
require("updateadb" in lock_run and "restart" in lock_run,
        "restart and update use lifecycle lock")
require("G|updateadb) adbyby_uprules" in locked_action,
        "RC updateadb notification reaches the rule updater")
require('eval("/usr/bin/adbyby.sh","restart");' in restart_adbyby,
        "RC restart uses one atomic lifecycle invocation")
require('"$PROG_PATH/adbyby" 9>&- >/dev/null 2>&1 &' in source and "restart_dhcpd 9>&-" in source,
        "detached daemon and DHCP children close FD 9")
for marker in ("lifecycle transitions interleaved", "updateadb did not dispatch updater",
               "symlink lock accepted", "hard-linked lock accepted",
               "Adbyby lifecycle serialization, bounded lock"):
    require(marker in lifecycle_test, f"lifecycle test misses: {marker}")

for marker in (
    "addn-hosts=/etc/storage/custom-hosts",
    "addn-hosts=/tmp/adbyby/hosts-backup",
    "hostsdir=/etc/dnsmasq/hosts",
    "conf-file=/etc/storage/dnsmasq.d/hosts.conf",
    "address=/hosts.example/192.0.2.10",
    "Metacharacters in the configured file name",
    "idempotent",
    "Adbyby exact managed addn-hosts cleanup",
):
    require(marker in test, f"host regression is missing case: {marker}")

paths = (
    ".github/workflows/adbyby-config-stability.yml",
    "trunk/user/adbyby/adbyby.sh",
    "trunk/user/adbyby/tests/hosts_config_preservation.sh",
    "trunk/user/adbyby/tests/lifecycle_lock.sh",
    "trunk/user/adbyby/tests/rules_download.sh",
    "trunk/user/adbyby/tests/external_downloads.sh",
    "trunk/user/adbyby/tests/anti_ad_download.sh",
    "trunk/user/adbyby/tests/host_list_download.sh",
    "trunk/user/adbyby/adbyby_host.sh",
    "trunk/user/adbyby/Makefile",
    "trunk/user/www/n56u_ribbon_fixed/Advanced_adbyby.asp",
    "trunk/user/www/n56u_ribbon_fixed/Ad_action.asp",
    "trunk/user/httpd/src/variables.c",
    "trunk/user/scripts/files/usr/bin/copyscripts.sh",
    "trunk/user/adbyby/tests/firewall_snapshot.sh",
    "trunk/user/rc/src/services.c",
    "scripts/check_adbyby_hosts_config.py",
)
filters = workflow.split("jobs:", 1)[0]
require("  pull_request:" in filters, "pull_request path filter is missing")
push_filters, pull_filters = filters.split("  pull_request:", 1)
for path in paths:
    require(push_filters.count(path) == 1 and pull_filters.count(path) == 1,
            f"{path} must appear exactly once in push and pull_request path filters")

for command in (
    "sh -n trunk/user/adbyby/adbyby.sh",
    "sh -n trunk/user/adbyby/tests/hosts_config_preservation.sh",
    "sh -n trunk/user/adbyby/tests/lifecycle_lock.sh",
    "sh -n trunk/user/adbyby/tests/rules_download.sh",
    "sh -n trunk/user/adbyby/tests/external_downloads.sh",
    "sh -n trunk/user/adbyby/tests/anti_ad_download.sh",
    "sh -n trunk/user/adbyby/tests/host_list_download.sh",
    "sh trunk/user/adbyby/tests/hosts_config_preservation.sh",
    "sh trunk/user/adbyby/tests/lifecycle_lock.sh",
    "sh trunk/user/adbyby/tests/rules_download.sh",
    "sh trunk/user/adbyby/tests/external_downloads.sh",
    "sh trunk/user/adbyby/tests/anti_ad_download.sh",
    "sh trunk/user/adbyby/tests/host_list_download.sh",
    "sh trunk/user/adbyby/tests/firewall_snapshot.sh",
    "python3 scripts/check_adbyby_hosts_config.py",
):
    require(command in workflow, f"workflow omits required command: {command}")

print("Adbyby managed, third-party, anti-AD and configured hosts downloads, dnsmasq cleanup, producer chain and CI integration contracts: PASS")
