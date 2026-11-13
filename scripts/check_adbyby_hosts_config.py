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
helper = function_body(source, "adbyby_remove_generated_hosts_config")
hosts_ads = function_body(source, "hosts_ads")
del_dns = function_body(source, "del_dns")
lock_prepare = function_body(source, "adbyby_lifecycle_lock_prepare")
lock_execute = function_body(source, "adbyby_lifecycle_lock_execute")
locked_action = function_body(source, "adbyby_lifecycle_locked_action")
lock_run = function_body(source, "adbyby_lifecycle_lock_run")
restart_adbyby = function_body(service, "restart_adbyby")

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
require(hosts_ads.count("adbyby_remove_generated_hosts_config") == 1,
        "hosts_ads must call the scoped cleanup helper exactly once")
require(del_dns.count("adbyby_remove_generated_hosts_config") == 1,
        "del_dns must call the scoped cleanup helper exactly once")
append_index = hosts_ads.find("cat >> /etc/storage/dnsmasq/dnsmasq.conf")
require(append_index >= 0 and
        hosts_ads.index("adbyby_remove_generated_hosts_config") < append_index,
        "hosts_ads must clean the old entry before appending the replacement")
require("return 1" in hosts_ads[:append_index],
        "hosts_ads must fail closed when it cannot remove the prior entry")
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
    "sh trunk/user/adbyby/tests/hosts_config_preservation.sh",
    "sh trunk/user/adbyby/tests/lifecycle_lock.sh",
    "sh trunk/user/adbyby/tests/firewall_snapshot.sh",
    "python3 scripts/check_adbyby_hosts_config.py",
):
    require(command in workflow, f"workflow omits required command: {command}")

print("Adbyby managed dnsmasq hosts cleanup and CI integration contracts: PASS")
