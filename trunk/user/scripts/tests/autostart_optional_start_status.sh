#!/bin/sh
set -eu

TEST_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
SOURCE="$TEST_DIR/../files/usr/bin/autostart.sh"
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/autostart-status.XXXXXX")
trap 'rm -rf "$TMP_DIR"' EXIT HUP INT TERM

mkdir -p "$TMP_DIR/bin" "$TMP_DIR/storage"
sed \
	-e "s|/etc/storage|$TMP_DIR/storage|g" \
	-e "s|/usr/bin|$TMP_DIR/bin|g" \
	-e "s|/tmp/dnsmasq.dom|$TMP_DIR/dnsmasq.dom|g" \
	"$SOURCE" > "$TMP_DIR/autostart.sh"

cat > "$TMP_DIR/bin/nvram" <<'STUB'
#!/bin/sh
[ "$1" = get ] || exit 1
case "$2" in
	adg_enable) echo "${ENABLE_ADG:-1}" ;;
	ddnsto_enable) echo "${ENABLE_DDNSTO:-1}" ;;
	frpc_enable) echo "${ENABLE_FRPC:-1}" ;;
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

echo "optional add-on autostart failure propagation regression: PASS"
