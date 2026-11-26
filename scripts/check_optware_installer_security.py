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
    "for bootstrap_command in wget openssl awk sed grep mktemp wc mountpoint",
    'command -v "$bootstrap_command"',
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
require(updater.count("http://") == 1 and "grep -q 'http://' /opt/etc/opkg.conf" in updater,
        "the only HTTP literal in the updater must be the residual-feed rejection guard")
require("grep -q 'http://' /opt/etc/opkg.conf" in updater,
        "launcher must reject residual plaintext package feeds")
require("chmod +x $INSTALLER" not in updater and "sh $INSTALLER" not in updater,
        "do not execute the unvalidated fixed-path installer")

require("gsub(" in policy and "http://bin.entware.net/" in policy and
        "https://bin.entware.net/" in policy,
        "installer must rewrite the approved Entware origin to HTTPS")
for token in (
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
ipkg_source = read("trunk/user/optware/ipkg.sh")
ipkg_updater = read("trunk/user/optware/opt-ipkg-upd.sh")
ipkg_start = read("trunk/user/optware/opt-start.sh")
ipkg_transport = read("trunk/user/optware/ipkg_transport_policy.sh")
ipkg_transport_test = read("trunk/user/optware/tests/ipkg_transport_policy.sh")

require('. "$ipkg_transport_policy" || exit 1' in ipkg_source,
        "the privileged ipkg client must load its transport policy")
require("https://*)" in ipkg_source and "wget -T 60" in ipkg_source,
        "ipkg must bound HTTPS downloads")
require("http://* | ftp://*" in ipkg_source and
        "ipkg_transport_require_url" in ipkg_source,
        "ipkg must reject plaintext downloads and direct-install URLs")
require("ipkg_transport_require_url" in ipkg_transport and
        "ipkg_transport_config_is_secure" in ipkg_transport,
        "missing centralized ipkg transport policy")
require('mountpoint -q /opt' in ipkg_updater and
        'ipkg_transport_config_is_secure "$IPKG_CONF_DIR/ipkg.conf"' in ipkg_updater,
        "legacy package updater must require the exact mount and secure feed config")
require("http://ipkg.nslu2-linux.org/" not in ipkg_updater and
        "/usr/bin/ipkg.sh update" in ipkg_updater and
        "ipkg update" not in ipkg_updater,
        "legacy updater must not invoke the unaudited old ipkg binary")
require("legacy Optware package bootstrap/update failed" in ipkg_start and
        "optware_lifecycle_lock_release" in ipkg_start,
        "Optware startup must propagate updater failure and release its lifecycle lock")
require("$(ROMFSINST) -p +x /usr/lib/ipkg_transport_policy.sh" in makefile,
        "ROMFS must install the ipkg transport policy")
for token in (
    "permits HTTPS/local files and rejects HTTP/FTP/unknown schemes: PASS",
    "accepts HTTPS and local-file source configuration: PASS",
    "rejects HTTP/FTP/malformed/empty source configuration: PASS",
    "refuses symlink package configuration: PASS",
):
    require(token in ipkg_transport_test, "missing ipkg transport regression: " + token)
require("src optware http://ipkg.nslu2-linux.org/" in read("trunk/user/optware/ipkg.conf"),
        "do not silently rewrite the known HTTP-only legacy feed to an unverified HTTPS URL")

for path in (
    "trunk/user/optware/ipkg.sh",
    "trunk/user/optware/opt-ipkg-upd.sh",
    "trunk/user/optware/opt-start.sh",
    "trunk/user/optware/Makefile",
    "trunk/user/optware/ipkg_transport_policy.sh",
    "trunk/user/optware/tests/ipkg_transport_policy.sh",
):
    require(process_workflow.count('      - "' + path + '"') == 2,
            "both Process workflow path filters must include " + path)
require("sh trunk/user/optware/tests/ipkg_transport_policy.sh" in process_workflow,
        "Process workflow must execute ipkg transport regression")

print("Entware root-bootstrap and legacy ipkg verified-transport source contracts: PASS")
