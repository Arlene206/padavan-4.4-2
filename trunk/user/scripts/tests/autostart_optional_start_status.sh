#!/bin/sh
set -eu

TEST_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
SOURCE="$TEST_DIR/../files/usr/bin/autostart.sh"
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/autostart-status.XXXXXX")
trap 'rm -rf "$TMP_DIR"' EXIT HUP INT TERM

mkdir -p "$TMP_DIR/bin" "$TMP_DIR/storage" "$TMP_DIR/ssl/certs"
sed \
	-e "s|/etc/storage|$TMP_DIR/storage|g" \
	-e "s|/usr/bin|$TMP_DIR/bin|g" \
	-e "s|/etc/ssl|$TMP_DIR/ssl|g" \
	-e "s|/tmp/dnsmasq.dom|$TMP_DIR/dnsmasq.dom|g" \
	"$SOURCE" > "$TMP_DIR/autostart.sh"

cat > "$TMP_DIR/bin/nvram" <<'STUB'
#!/bin/sh
[ "$1" = get ] || exit 1
if [ "${NVRAM_FAIL_KEY:-}" = "$2" ]; then
	exit "${NVRAM_FAIL_STATUS:-9}"
fi
case "$2" in
	adg_enable) echo "${ENABLE_ADG:-1}" ;;
	ddnsto_enable) echo "${ENABLE_DDNSTO:-1}" ;;
	sdns_enable) echo "${ENABLE_SMARTDNS:-0}" ;;
	sdns_port) echo "${SMARTDNS_PORT:-6053}" ;;
	frpc_enable) echo "${ENABLE_FRPC:-1}" ;;
	aliddns_enable) echo "${ENABLE_ALIDDNS:-0}" ;;
	frps_enable) echo 0 ;;
	*) echo 0 ;;
esac
STUB
cat > "$TMP_DIR/bin/ping" <<'STUB'
#!/bin/sh
exit 0
STUB
cat > "$TMP_DIR/bin/logger" <<'STUB'
#!/bin/sh
printf '%s\n' "$*" >> "$AUTOSTART_LOG"
STUB
cat > "$TMP_DIR/bin/ap_script.sh" <<'STUB'
#!/bin/sh
exit 0
STUB
cat > "$TMP_DIR/bin/sh_ezscript.sh" <<'STUB'
#!/bin/sh
exit 0
STUB
cat > "$TMP_DIR/bin/adguardhome.sh" <<'STUB'
#!/bin/sh
printf 'adguardhome:%s\n' "$*" >> "$AUTOSTART_CALLS"
exit "${ADGUARD_STATUS:-0}"
STUB
cat > "$TMP_DIR/bin/ddnsto.sh" <<'STUB'
#!/bin/sh
printf 'ddnsto:%s\n' "$*" >> "$AUTOSTART_CALLS"
exit "${DDNSTO_STATUS:-0}"
STUB
cat > "$TMP_DIR/bin/frp.sh" <<'STUB'
#!/bin/sh
printf 'frp:%s\n' "$*" >> "$AUTOSTART_CALLS"
exit "${FRP_STATUS:-0}"
STUB
cat > "$TMP_DIR/bin/smartdns.sh" <<'STUB'
#!/bin/sh
printf 'smartdns:%s\n' "$*" >> "$AUTOSTART_CALLS"
printf 'smartdns-port:%s\n' "${SMARTDNS_PORT:-}" >> "$AUTOSTART_CALLS"
exit "${SMARTDNS_STATUS:-0}"
STUB
cat > "$TMP_DIR/bin/aliddns.sh" <<'STUB'
#!/bin/sh
printf 'aliddns:%s\n' "$*" >> "$AUTOSTART_CALLS"
exit "${ALIDDNS_STATUS:-0}"
STUB
chmod 755 "$TMP_DIR/bin/"*

AUTOSTART_LOG="$TMP_DIR/logger.log"
AUTOSTART_CALLS="$TMP_DIR/calls.log"
export AUTOSTART_LOG AUTOSTART_CALLS
PATH="$TMP_DIR/bin:$PATH"
export PATH
ADGUARD_STATUS=41
DDNSTO_STATUS=42
FRP_STATUS=43
export ADGUARD_STATUS DDNSTO_STATUS FRP_STATUS
ENABLE_ALIDDNS=0
ALIDDNS_STATUS=0
export ENABLE_ALIDDNS ALIDDNS_STATUS

sh -n "$TMP_DIR/autostart.sh"
sh "$TMP_DIR/autostart.sh" >/dev/null 2>&1 || {
	echo "autostart aborted after an optional add-on failure" >&2
	exit 1
}
grep -Fxq 'adguardhome:start' "$AUTOSTART_CALLS" || {
	echo "AdGuardHome start was not attempted" >&2
	exit 1
}
grep -Fxq 'ddnsto:start' "$AUTOSTART_CALLS" || {
	echo "DDNSTO start was not attempted after an earlier add-on failure" >&2
	exit 1
}
grep -Fxq 'frp:start' "$AUTOSTART_CALLS" || {
	echo "FRP start was not attempted after earlier add-on failures" >&2
	exit 1
}
grep -Fq 'AdGuardHome autostart failed (status=41)' "$AUTOSTART_LOG" || {
	echo "AdGuardHome launcher failure status was not logged" >&2
	exit 1
}
grep -Fq 'DDNSTO autostart failed (status=42)' "$AUTOSTART_LOG" || {
	echo "DDNSTO launcher failure status was not logged" >&2
	exit 1
}
grep -Fq 'FRP autostart failed (status=43)' "$AUTOSTART_LOG" || {
	echo "FRP launcher failure status was not logged" >&2
	exit 1
}

: > "$AUTOSTART_LOG"
: > "$AUTOSTART_CALLS"
ADGUARD_STATUS=0
DDNSTO_STATUS=0
FRP_STATUS=0
export ADGUARD_STATUS DDNSTO_STATUS FRP_STATUS
sh "$TMP_DIR/autostart.sh" >/dev/null 2>&1 || {
	echo "autostart failed when optional launchers succeeded" >&2
	exit 1
}
if grep -Eq 'AdGuardHome autostart failed|DDNSTO autostart failed|FRP autostart failed' "$AUTOSTART_LOG"; then
	echo "successful launcher emitted a false failure diagnostic" >&2
	exit 1
fi


: > "$AUTOSTART_LOG"
: > "$AUTOSTART_CALLS"
mkdir -p "$TMP_DIR/storage/dnsmasq"
printf '去广告规则\nadbyby\ncustom-rule\n' > "$TMP_DIR/storage/smartdns_custom.conf"
printf 'no-resolv\nserver=127.0.0.1#6053\nserver=127.0.0.1#53\nserver=/example.com/127.0.0.1#6053\nport=0\nserver=8.8.8.8#53\n' > "$TMP_DIR/storage/dnsmasq/dnsmasq.conf"
printf 'stale-state\n' > "$TMP_DIR/storage/smartdns_conf.ini"
cp "$TMP_DIR/storage/dnsmasq/dnsmasq.conf" "$TMP_DIR/dnsmasq.before"
cp "$TMP_DIR/storage/smartdns_conf.ini" "$TMP_DIR/smartdns-state.before"
ENABLE_SMARTDNS=1
SMARTDNS_PORT=6053
# A malformed persisted-state fixture makes the production SmartDNS Read_ini()
# path fail closed. Autostart must report the failure but continue independent
# add-ons; it must not bypass the resolver transaction or discard the state.
SMARTDNS_STATUS=45
export ENABLE_SMARTDNS SMARTDNS_PORT SMARTDNS_STATUS
sh "$TMP_DIR/autostart.sh" >/dev/null 2>&1 || {
	echo "autostart failed while dispatching the SmartDNS launcher" >&2
	exit 1
}
cmp -s "$TMP_DIR/dnsmasq.before" "$TMP_DIR/storage/dnsmasq/dnsmasq.conf" || {
	echo "autostart modified DNSMasq configuration outside SmartDNS lifecycle" >&2
	exit 1
}
cmp -s "$TMP_DIR/smartdns-state.before" "$TMP_DIR/storage/smartdns_conf.ini" || {
	echo "autostart consumed malformed SmartDNS saved state" >&2
	exit 1
}
grep -Fxq 'smartdns:start' "$AUTOSTART_CALLS" || {
	echo "SmartDNS launcher was not attempted" >&2
	exit 1
}
grep -Fq 'SmartDNS autostart failed (status=45)' "$AUTOSTART_LOG" || {
	echo "SmartDNS launch failure was not reported" >&2
	exit 1
}
grep -Fxq 'ddnsto:start' "$AUTOSTART_CALLS" || {
	echo "SmartDNS failure blocked independent DDNSTO startup" >&2
	exit 1
}
grep -Fxq 'frp:start' "$AUTOSTART_CALLS" || {
	echo "SmartDNS failure blocked independent FRP startup" >&2
	exit 1
}
grep -Fxq 'custom-rule' "$TMP_DIR/storage/smartdns_custom.conf" || {
	echo "autostart removed a user-owned SmartDNS custom rule" >&2
	exit 1
}
if grep -Eq '去广告|adbyby' "$TMP_DIR/storage/smartdns_custom.conf"; then
	echo "autostart did not remove obsolete ad-blocking custom lines" >&2
	exit 1
fi
SMARTDNS_STATUS=0
export SMARTDNS_STATUS

: > "$AUTOSTART_LOG"
: > "$AUTOSTART_CALLS"
printf 'server=127.0.0.1#6053\nserver=127.0.0.1#53\n' > "$TMP_DIR/storage/dnsmasq/dnsmasq.conf"
cp "$TMP_DIR/storage/dnsmasq/dnsmasq.conf" "$TMP_DIR/dnsmasq.invalid-port.before"
SMARTDNS_PORT='6053/d'
export SMARTDNS_PORT
sh "$TMP_DIR/autostart.sh" >/dev/null 2>&1 || {
	echo "autostart failed while dispatching SmartDNS with an invalid port" >&2
	exit 1
}
grep -Fxq 'smartdns:start' "$AUTOSTART_CALLS" || {
	echo "autostart did not dispatch SmartDNS for launcher-side validation" >&2
	exit 1
}
grep -Fxq 'smartdns-port:6053/d' "$AUTOSTART_CALLS" || {
	echo "autostart did not pass the configured SmartDNS port to its launcher" >&2
	exit 1
}
cmp -s "$TMP_DIR/dnsmasq.invalid-port.before" "$TMP_DIR/storage/dnsmasq/dnsmasq.conf" || {
	echo "autostart modified DNSMasq upstreams before SmartDNS validated the port" >&2
	exit 1
}


# AliDDNS depends on the firmware CA bundle. Its failure to prepare must skip
# only AliDDNS, not the other independent autostart services.
: > "$AUTOSTART_LOG"
: > "$AUTOSTART_CALLS"
ENABLE_ALIDDNS=1
ALIDDNS_STATUS=44
ENABLE_SMARTDNS=0
export ENABLE_ALIDDNS ALIDDNS_STATUS ENABLE_SMARTDNS
rm -f "$TMP_DIR/ssl/certs/ca-certificates.crt"
sh "$TMP_DIR/autostart.sh" >/dev/null 2>&1 || {
	echo "autostart failed when AliDDNS CA bundle was missing" >&2
	exit 1
}
if grep -Fxq 'aliddns:start' "$AUTOSTART_CALLS"; then
	echo "AliDDNS launched without its required CA trust bundle" >&2
	exit 1
fi
grep -Fq 'AliDDNS autostart skipped: CA trust bundle is not readable' "$AUTOSTART_LOG" || {
	echo "missing AliDDNS CA dependency was not logged" >&2
	exit 1
}
grep -Fxq 'ddnsto:start' "$AUTOSTART_CALLS" || {
	echo "missing AliDDNS CA bundle incorrectly blocked DDNSTO" >&2
	exit 1
}
grep -Fxq 'frp:start' "$AUTOSTART_CALLS" || {
	echo "missing AliDDNS CA bundle incorrectly blocked FRP" >&2
	exit 1
}

: > "$AUTOSTART_LOG"
: > "$AUTOSTART_CALLS"
printf 'test CA bundle\n' > "$TMP_DIR/ssl/certs/ca-certificates.crt"
sh "$TMP_DIR/autostart.sh" >/dev/null 2>&1 || {
	echo "autostart failed when AliDDNS CA bundle was present" >&2
	exit 1
}
grep -Fxq 'aliddns:start' "$AUTOSTART_CALLS" || {
	echo "AliDDNS did not start after its CA trust bundle became readable" >&2
	exit 1
}
grep -Fq 'AliDDNS autostart failed (status=44)' "$AUTOSTART_LOG" || {
	echo "AliDDNS launcher failure status was not logged after CA readiness" >&2
	exit 1
}

# A failed NVRAM read skips only that dependent feature, logs its key/status,
# and does not prevent unrelated add-ons from starting.
: > "$AUTOSTART_LOG"
: > "$AUTOSTART_CALLS"
ADGUARD_STATUS=0
DDNSTO_STATUS=0
FRP_STATUS=0
ENABLE_ADG=1
ENABLE_DDNSTO=1
ENABLE_FRPC=1
ENABLE_ALIDDNS=0
ENABLE_SMARTDNS=0
export ADGUARD_STATUS DDNSTO_STATUS FRP_STATUS ENABLE_ADG ENABLE_DDNSTO
export ENABLE_FRPC ENABLE_ALIDDNS ENABLE_SMARTDNS
NVRAM_FAIL_STATUS=9
export NVRAM_FAIL_STATUS

NVRAM_FAIL_KEY=adg_enable
export NVRAM_FAIL_KEY
sh "$TMP_DIR/autostart.sh" >/dev/null 2>&1 || fail "autostart aborted on adg_enable read failure"
if grep -Fxq 'adguardhome:start' "$AUTOSTART_CALLS"; then fail "AdGuardHome launched after NVRAM read failure"; fi
grep -Fxq 'ddnsto:start' "$AUTOSTART_CALLS" || fail "AdGuardHome read failure blocked DDNSTO"
grep -Fxq 'frp:start' "$AUTOSTART_CALLS" || fail "AdGuardHome read failure blocked FRP"
grep -Fq 'NVRAM read failed for adg_enable (status=9)' "$AUTOSTART_LOG" || fail "AdGuardHome NVRAM failure was not logged"

: > "$AUTOSTART_LOG"
: > "$AUTOSTART_CALLS"
NVRAM_FAIL_KEY=ddnsto_enable
export NVRAM_FAIL_KEY
sh "$TMP_DIR/autostart.sh" >/dev/null 2>&1 || fail "autostart aborted on ddnsto_enable read failure"
if grep -Fxq 'ddnsto:start' "$AUTOSTART_CALLS"; then fail "DDNSTO launched after NVRAM read failure"; fi
grep -Fxq 'adguardhome:start' "$AUTOSTART_CALLS" || fail "DDNSTO read failure blocked AdGuardHome"
grep -Fxq 'frp:start' "$AUTOSTART_CALLS" || fail "DDNSTO read failure blocked FRP"
grep -Fq 'NVRAM read failed for ddnsto_enable (status=9)' "$AUTOSTART_LOG" || fail "DDNSTO NVRAM failure was not logged"

: > "$AUTOSTART_LOG"
: > "$AUTOSTART_CALLS"
NVRAM_FAIL_KEY=frpc_enable
export NVRAM_FAIL_KEY
sh "$TMP_DIR/autostart.sh" >/dev/null 2>&1 || fail "autostart aborted on frpc_enable read failure"
if grep -Fxq 'frp:start' "$AUTOSTART_CALLS"; then fail "FRP launched after NVRAM read failure"; fi
grep -Fxq 'ddnsto:start' "$AUTOSTART_CALLS" || fail "FRP read failure blocked DDNSTO"
grep -Fq 'NVRAM read failed for frpc_enable (status=9)' "$AUTOSTART_LOG" || fail "FRP NVRAM failure was not logged"
grep -Fq 'FRP autostart failed (status=9)' "$AUTOSTART_LOG" || fail "FRP did not preserve NVRAM failure status"
unset NVRAM_FAIL_KEY NVRAM_FAIL_STATUS


# Persistent script symlinks can be restored from an uploaded storage archive.
# Boot-time default seeding must neither follow a dangling destination link
# nor execute a linked custom ap_script as root.
: > "$AUTOSTART_LOG"
: > "$AUTOSTART_CALLS"
rm -f "$TMP_DIR/storage/sh_ezscript.sh" "$TMP_DIR/storage/ap_script.sh"
echo 'do-not-overwrite-this-target' > "$TMP_DIR/sh-ezscript-target"
ln -s "$TMP_DIR/sh-ezscript-target" "$TMP_DIR/storage/sh_ezscript.sh"
cat > "$TMP_DIR/ap-script-target" <<'STUB'
#!/bin/sh
echo executed >> "$AP_SCRIPT_TARGET_LOG"
STUB
chmod 755 "$TMP_DIR/ap-script-target"
ln -s "$TMP_DIR/ap-script-target" "$TMP_DIR/storage/ap_script.sh"
echo executed >> "$AP_SCRIPT_TARGET_LOG"
export AP_SCRIPT_TARGET_LOG
sh "$TMP_DIR/autostart.sh" >/dev/null 2>&1 || fail "autostart failed while rejecting persistent script symlinks"
[ -L "$TMP_DIR/storage/sh_ezscript.sh" ] || fail "sh_ezscript symlink was replaced or followed"
echo 'do-not-overwrite-this-target' > "$TMP_DIR/sh-ezscript-target"
[ -L "$TMP_DIR/storage/ap_script.sh" ] || fail "ap_script symlink was replaced or followed"
echo executed >> "$AP_SCRIPT_TARGET_LOG"
grep -Fq 'Refusing to follow persistent script symlink' "$AUTOSTART_LOG" || fail "sh_ezscript symlink rejection was not logged"
grep -Fq 'Skipping symlinked persistent ap_script.sh' "$AUTOSTART_LOG" || fail "ap_script symlink rejection was not logged"

echo "optional add-on and SmartDNS autostart regression: PASS"

