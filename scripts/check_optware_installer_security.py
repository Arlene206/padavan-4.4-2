#!/usr/bin/env python3
from pathlib import Path


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(message)


def read(path: str) -> str:
    return Path(path).read_text(encoding="utf-8")


updater = read("trunk/user/optware/opt-opkg-upd.sh")
policy = read("trunk/user/optware/optware_bootstrap_policy.sh")
regression = read("trunk/user/optware/tests/optware_bootstrap_policy.sh")
makefile = read("trunk/user/optware/Makefile")
process_workflow = read(".github/workflows/process-lifecycle-regression.yml")
xy_workflow = read(".github/workflows/xy-c1-firmware-validation.yml")
busybox_config = read("trunk/configs/boards/busybox.config")
xy_config = read("trunk/configs/templates/XY-C1.config")

for token in (
    "mountpoint -q /opt",
    "/etc/ssl/certs/ca-certificates.crt",
    "SSL_CERT_FILE=$ca_bundle",
    "command -v openssl",
    "umask 077",
    "mktemp -d /tmp/entware-bootstrap.XXXXXX",
    "https://bin.entware.net/mipselsf-k3.4/installer/generic.sh",
    "wget -q -T 60",
    "optware_bootstrap_patch_installer",
    'sh "$bootstrap_installer"',
    "/opt/bin/opkg",
    "/opt/etc/opkg.conf",
    "Entware bootstrap completed using verified HTTPS feeds",
):
    require(token in updater, "missing secure Entware bootstrap control: " + token)
require("http://" not in updater, "Entware bootstrap launcher contains a plaintext HTTP URL")
require("grep -q 'http://' /opt/etc/opkg.conf" in updater,
        "launcher must reject residual plaintext package feeds")
require("chmod +x $INSTALLER" not in updater and "sh $INSTALLER" not in updater,
        "do not execute the unvalidated fixed-path installer")

for token in (
    'gsub(/http:\\/\\/bin[.]entware[.]net\\//, "https://bin.entware.net/", line)',
    'print "set -e"',
    'sub(/wget[[:space:]]+/, "wget -T 60 ", line)',
    "config_download_count != 1",
    "download_count != 2",
    "insecure_url",
    "sh -n",
):
    require(token in policy, "missing fail-closed installer rewrite contract: " + token)

require("optware_bootstrap_policy.sh" in makefile and
        "$(ROMFSINST) -p +x /usr/lib/optware_bootstrap_policy.sh" in makefile,
        "ROMFS must install the checked Entware installer policy helper")
for token in (
    "missing and duplicate upstream download anchors fail closed: PASS",
    "unknown installer HTTP origins fail closed: PASS",
    "installer patching rejects symlink inputs and preserves existing targets: PASS",
    "Entware package feeds are upgraded to HTTPS before opkg update: PASS",
    "wget -T 60",
):
    require(token in regression, "missing Entware bootstrap regression: " + token)

require("CONFIG_WGET=y" in busybox_config and
        "CONFIG_FEATURE_WGET_LONG_OPTIONS=y" in busybox_config and
        "CONFIG_FEATURE_WGET_TIMEOUT=y" in busybox_config and
        "CONFIG_FEATURE_WGET_OPENSSL=y" in busybox_config,
        "BusyBox wget must provide bounded OpenSSL-backed HTTPS")
require("# CONFIG_FEATURE_WGET_SSL_HELPER is not set" in busybox_config,
        "do not enable ssl_helper: its BusyBox implementation does not validate certificates")
require("CONFIG_FIRMWARE_INCLUDE_OPENSSL_EXE=y" in xy_config,
        "XY-C1 must install the OpenSSL executable used by BusyBox wget TLS verification")

for path in (
    "trunk/user/optware/opt-opkg-upd.sh",
    "trunk/user/optware/optware_bootstrap_policy.sh",
    "trunk/user/optware/tests/optware_bootstrap_policy.sh",
    "scripts/check_optware_installer_security.py",
    "trunk/configs/boards/busybox.config",
):
    require(process_workflow.count('      - "' + path + '"') == 2,
            "both Process workflow path filters must include " + path)
for path in ("trunk/user/optware/**", "trunk/configs/boards/busybox.config"):
    require(xy_workflow.count('      - "' + path + '"') == 1,
            "XY-C1 firmware workflow must watch " + path)
require("sh trunk/user/optware/tests/optware_bootstrap_policy.sh" in process_workflow and
        "python3 scripts/check_optware_installer_security.py" in process_workflow,
        "Process workflow must execute the Entware bootstrap regression and contract")
print("Entware root-bootstrap TLS, staging and fail-closed source contract: PASS")
