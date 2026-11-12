#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
script_path = ROOT / "trunk/user/adguardhome/adguardhome.sh"
test_path = ROOT / "trunk/user/adguardhome/tests/adguardhome_config_security.sh"
package_makefile_path = ROOT / "trunk/user/adguardhome/Makefile"
user_makefile_path = ROOT / "trunk/user/Makefile"
services_path = ROOT / "trunk/user/rc/src/services.c"
rc_header_path = ROOT / "trunk/user/rc/src/rc.h"
rc_path = ROOT / "trunk/user/rc/src/rc.c"
workflow_path = ROOT / ".github/workflows/adguardhome-config-security.yml"

script = script_path.read_text()
test = test_path.read_text()
package_makefile = package_makefile_path.read_text()
user_makefile = user_makefile_path.read_text()
services = services_path.read_text()
rc_header = rc_header_path.read_text()
rc = rc_path.read_text()
workflow = workflow_path.read_text()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"AdGuardHome config security contract: FAIL: {message}")


def function_body(source: str, signature: str) -> str:
    match = re.search(rf"(?m)^{re.escape(signature)}[ \\t]*\\{{(.*?)^[ \\t]*\\}}", source, re.S)
    require(match is not None, f"missing function {signature}")
    return match.group(1)


config = function_body(script, "getconfig(){")
start = function_body(script, "start_adg(){")
require('adg_file="${ADG_CONFIG_FILE:-/etc/storage/adg.sh}"' in config,
        "testable config path must retain the production default")
require('if [ -L "$adg_file" ]; then' in config,
        "symlink config destinations must be rejected before writes")
require('if [ -e "$adg_file" ] && [ ! -f "$adg_file" ]; then' in config,
        "non-regular config destinations must be rejected")
require('mktemp "${adg_file}.tmp.XXXXXX"' in config,
        "default config must use a unique same-directory temporary")
require('cat > "$adg_tmp" <<-' in config and 'umask 077' in config,
        "default config contents must be written to a private stage")
require('chmod 600 "$adg_tmp"' in config and 'mv -f "$adg_tmp" "$adg_file"' in config,
        "private stage must be permissioned before atomic publication")
require('chmod 600 "$adg_file"' in config and 'chmod 755 "$adg_file"' not in config,
        "all existing and generated config files must be mode 0600")
require('rm -f "$adg_tmp"' in config and 'return 1' in config,
        "failed staging, chmod or rename must clean up and fail closed")
require('auth_pass: adguardhome' in config and 'private_key: ""' in config,
        "source contract must recognize secret-bearing config fields")
require('if ! getconfig; then' in start and
        start.index('if ! getconfig; then') < start.index('change_dns') <
        start.index('set_iptable') < start.index('eval "AdGuardHome'),
        "unsafe config preparation must stop before DNS/firewall changes or launch")

require('$(INSTALL_BIN) ./adguardhome.sh $(ROMFSDIR)/usr/bin/adguardhome.sh' in package_makefile,
        "ROMFS must install the audited launcher")
require('$(INSTALL_BIN) $(PKG_BUILD_DIR)/AdGuardHome $(ROMFSDIR)/usr/bin/AdGuardHome' in package_makefile,
        "ROMFS must install the daemon inspected by the lifecycle caller")
require('dir_$(CONFIG_FIRMWARE_INCLUDE_ADGUARDHOME)' in user_makefile and
        '+= adguardhome' in user_makefile,
        "user package graph must retain the firmware configuration gate")
require('eval("/usr/bin/adguardhome.sh","stop")' in services and
        'eval("/usr/bin/adguardhome.sh","start")' in services and
        'restart_adguardhome(void)' in services,
        "RC service wrapper must call the ROMFS-installed launcher")
require('void restart_adguardhome(void);' in rc_header and
        'RCN_RESTART_ADGUARDHOME' in rc and 'restart_adguardhome();' in rc,
        "RC prototype and notification dispatch must remain connected")

for marker in (
    "world-readable mode is repaired",
    "symlink config destination was accepted",
    "FIFO config destination was accepted",
    "rename failure did not clean the private stage",
    "chmod failure published a config",
    "startup changed DNS/firewall before config validation",
):
    require(marker in test, f"host regression missing case: {marker}")

paths = [
    ".github/workflows/adguardhome-config-security.yml",
    "scripts/check_adguardhome_config.py",
    "trunk/user/adguardhome/adguardhome.sh",
    "trunk/user/adguardhome/tests/adguardhome_config_security.sh",
    "trunk/user/adguardhome/Makefile",
    "trunk/user/Makefile",
    "trunk/user/rc/src/services.c",
    "trunk/user/rc/src/rc.h",
    "trunk/user/rc/src/rc.c",
]
push = workflow.split("  push:", 1)[1].split("  pull_request:", 1)[0]
pull = workflow.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for path in paths:
    token = f'      - "{path}"'
    require(push.count(token) == 1 and pull.count(token) == 1,
            f"{path} must appear exactly once in push and pull_request path filters")

print("AdGuardHome config privacy, atomic publication and RC integration contracts: PASS")
