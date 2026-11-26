#!/bin/sh

self_name="Entware"
ca_bundle="/etc/ssl/certs/ca-certificates.crt"
bootstrap_helper="/usr/lib/optware_bootstrap_policy.sh"
installer_url="https://bin.entware.net/mipselsf-k3.4/installer/generic.sh"

optware_bootstrap_fail()
{
    logger -t "$self_name" "$*"
    exit 1
}

mountpoint -q /opt || optware_bootstrap_fail "/opt is not a mountpoint; refusing Entware bootstrap"
for bootstrap_dir in /opt/bin /opt/etc; do
    if [ -L "$bootstrap_dir" ] || { [ -e "$bootstrap_dir" ] && [ ! -d "$bootstrap_dir" ]; }; then
        optware_bootstrap_fail "Refusing unsafe Optware directory: $bootstrap_dir"
    fi
    if [ ! -d "$bootstrap_dir" ] && ! mkdir -p "$bootstrap_dir"; then
        optware_bootstrap_fail "Cannot create Optware directory: $bootstrap_dir"
    fi
    [ -d "$bootstrap_dir" ] && [ ! -L "$bootstrap_dir" ] ||
        optware_bootstrap_fail "Optware directory changed during setup: $bootstrap_dir"
done

for bootstrap_target in /opt/bin/opkg /opt/etc/opkg.conf; do
    if [ -L "$bootstrap_target" ] ||
       { [ -e "$bootstrap_target" ] && [ ! -f "$bootstrap_target" ]; }; then
        optware_bootstrap_fail "Refusing unsafe Entware target: $bootstrap_target"
    fi
done
[ -f /opt/bin/opkg ] && exit 0

[ -f "$ca_bundle" ] && [ ! -L "$ca_bundle" ] && [ -s "$ca_bundle" ] && [ -r "$ca_bundle" ] ||
    optware_bootstrap_fail "CA trust bundle is missing, empty, unreadable or unsafe: $ca_bundle"
for bootstrap_command in wget openssl awk sed grep mktemp wc mountpoint; do
    command -v "$bootstrap_command" >/dev/null 2>&1 ||
        optware_bootstrap_fail "Required verified-download command is unavailable: $bootstrap_command"
done
[ -f "$bootstrap_helper" ] && [ ! -L "$bootstrap_helper" ] && [ -r "$bootstrap_helper" ] ||
    optware_bootstrap_fail "Verified Entware bootstrap policy is unavailable"
. "$bootstrap_helper" || optware_bootstrap_fail "Cannot load Entware bootstrap policy"

SSL_CERT_FILE=$ca_bundle
export SSL_CERT_FILE
umask 077
bootstrap_tmp_dir=$(mktemp -d /tmp/entware-bootstrap.XXXXXX) ||
    optware_bootstrap_fail "Cannot create private Entware bootstrap staging directory"
[ -d "$bootstrap_tmp_dir" ] && [ ! -L "$bootstrap_tmp_dir" ] ||
    optware_bootstrap_fail "Unsafe Entware bootstrap staging directory"
trap 'rm -rf "$bootstrap_tmp_dir"' 0
trap 'exit 1' HUP INT TERM

bootstrap_raw="$bootstrap_tmp_dir/generic.raw"
bootstrap_installer="$bootstrap_tmp_dir/generic.sh"
bootstrap_log="$bootstrap_tmp_dir/installer.log"
if ! wget -q -T 60 -O "$bootstrap_raw" "$installer_url"; then
    optware_bootstrap_fail "Verified HTTPS download of Entware installer failed"
fi
[ -f "$bootstrap_raw" ] && [ ! -L "$bootstrap_raw" ] && [ -s "$bootstrap_raw" ] && [ -r "$bootstrap_raw" ] ||
    optware_bootstrap_fail "Downloaded Entware installer is not a readable regular file"
bootstrap_size=$(wc -c < "$bootstrap_raw") ||
    optware_bootstrap_fail "Cannot determine downloaded Entware installer size"
case "$bootstrap_size" in
    ''|*[!0-9]*) optware_bootstrap_fail "Invalid downloaded Entware installer size" ;;
esac
[ "$bootstrap_size" -le 65536 ] ||
    optware_bootstrap_fail "Downloaded Entware installer exceeds the 64 KiB safety limit"
if ! optware_bootstrap_patch_installer "$bootstrap_raw" "$bootstrap_installer"; then
    optware_bootstrap_fail "Entware installer did not match the pinned HTTPS bootstrap policy"
fi

if sh "$bootstrap_installer" > "$bootstrap_log" 2>&1; then
    :
else
    bootstrap_status=$?
    logger -t "$self_name" "Entware installer failed (status=$bootstrap_status)"
    while IFS= read -r bootstrap_line; do logger -t "$self_name" "$bootstrap_line"; done < "$bootstrap_log"
    exit 1
fi

[ -f /opt/bin/opkg ] && [ ! -L /opt/bin/opkg ] && [ -x /opt/bin/opkg ] ||
    optware_bootstrap_fail "Entware installer did not publish an executable /opt/bin/opkg"
[ -f /opt/etc/opkg.conf ] && [ ! -L /opt/etc/opkg.conf ] && [ -s /opt/etc/opkg.conf ] && [ -r /opt/etc/opkg.conf ] ||
    optware_bootstrap_fail "Entware installer did not publish a readable /opt/etc/opkg.conf"
if grep -q 'http://' /opt/etc/opkg.conf; then
    optware_bootstrap_fail "Entware package configuration still contains an insecure HTTP feed"
else
    bootstrap_grep_status=$?
    [ "$bootstrap_grep_status" -eq 1 ] ||
        optware_bootstrap_fail "Cannot verify Entware package feed URLs"
fi
logger -t "$self_name" "Entware bootstrap completed using verified HTTPS feeds"
exit 0
