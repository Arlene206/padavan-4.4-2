#!/bin/sh
# Validate Transmission and Aria startup exit-status propagation.
set -eu
ROOT_DIR=$(pwd)
TRANSMISSION_SRC="$ROOT_DIR/trunk/user/transmission/transmission.sh"
ARIA_SRC="$ROOT_DIR/trunk/user/aria2/aria.sh"
[ -f "$TRANSMISSION_SRC" ] && [ -f "$ARIA_SRC" ] || {
	echo "Run this test from the Padavan repository root" >&2
	exit 1
}
TMP_DIR=$(mktemp -d)
trap 'rm -rf "$TMP_DIR"' EXIT HUP INT TERM
STUB_DIR="$TMP_DIR/bin"
mkdir -p "$STUB_DIR" \
	"$TMP_DIR/transmission/config" "$TMP_DIR/transmission/incomplete" \
	"$TMP_DIR/transmission/watch" "$TMP_DIR/aria/config" \
	"$TMP_DIR/Downloads"
: > "$TMP_DIR/transmission/config/settings.json"
: > "$TMP_DIR/aria/config/aria2.conf"
: > "$TMP_DIR/aria/config/incomplete.lst"
cat > "$STUB_DIR/pidof" <<'EOF'
#!/bin/sh
exit 1
EOF
cat > "$STUB_DIR/nvram" <<'EOF'
#!/bin/sh
case "$2" in
	trmd_pport) echo 51413 ;;
	trmd_rport) echo 9091 ;;
	aria_pport) echo 16888 ;;
	aria_rport) echo 6800 ;;
	http_username) echo admin ;;
	http_passwd) echo test-pass ;;
	http_proto) echo 0 ;;
	*) exit 1 ;;
esac
EOF
cat > "$STUB_DIR/start-stop-daemon" <<'EOF'
#!/bin/sh
[ -z "${STORAGE_TEST_START_MARKER:-}" ] || : > "$STORAGE_TEST_START_MARKER"
exit "${STORAGE_TEST_START_STATUS:-23}"
EOF
cat > "$STUB_DIR/logger" <<'EOF'
#!/bin/sh
exit "${STORAGE_TEST_LOGGER_STATUS:-0}"
EOF
cat > "$STUB_DIR/cat" <<'EOF'
#!/bin/sh
if [ "${STORAGE_TEST_CAT_STATUS:-0}" -ne 0 ]; then
	exit "$STORAGE_TEST_CAT_STATUS"
fi
exec /bin/cat "$@"
EOF
cat > "$STUB_DIR/mv" <<'EOF'
#!/bin/sh
if [ "${STORAGE_TEST_MV_STATUS:-0}" -ne 0 ]; then
	exit "$STORAGE_TEST_MV_STATUS"
fi
exec /bin/mv "$@"
EOF
cat > "$STUB_DIR/chown" <<'EOF'
#!/bin/sh
exit 0
EOF
cat > "$STUB_DIR/chmod" <<'EOF'
#!/bin/sh
exit 0
EOF
chmod 755 "$STUB_DIR/"*
sed \
	-e "s|SVC_PATH=\"/usr/bin/transmission-daemon\"|SVC_PATH=\"$STUB_DIR/transmission-daemon\"|" \
	-e "s|DIR_LINK=\"/mnt/transmission\"|DIR_LINK=\"$TMP_DIR/transmission\"|" \
	"$TRANSMISSION_SRC" > "$TMP_DIR/transmission.sh"
sed \
	-e "s|SVC_PATH=\"/usr/bin/aria2c\"|SVC_PATH=\"$STUB_DIR/aria2c\"|" \
	-e "s|DIR_LINK=\"/mnt/aria\"|DIR_LINK=\"$TMP_DIR/aria\"|" \
	"$ARIA_SRC" > "$TMP_DIR/aria.sh"
run_launcher()
{
	launcher=$1
	start_status=$2
	logger_status=$3
	cat_status=${4:-0}
	mv_status=${5:-0}
	start_marker=${6:-}
	if [ -n "${BUSYBOX_BIN:-}" ]; then
		PATH="$STUB_DIR:$PATH" STORAGE_TEST_START_STATUS="$start_status" STORAGE_TEST_LOGGER_STATUS="$logger_status" STORAGE_TEST_CAT_STATUS="$cat_status" STORAGE_TEST_MV_STATUS="$mv_status" STORAGE_TEST_START_MARKER="$start_marker" \
			"$BUSYBOX_BIN" ash "$launcher" start >/dev/null 2>&1
	else
		PATH="$STUB_DIR:$PATH" STORAGE_TEST_START_STATUS="$start_status" STORAGE_TEST_LOGGER_STATUS="$logger_status" STORAGE_TEST_CAT_STATUS="$cat_status" STORAGE_TEST_MV_STATUS="$mv_status" STORAGE_TEST_START_MARKER="$start_marker" \
			sh "$launcher" start >/dev/null 2>&1
	fi
}
assert_failure()
{
	name=$1
	launcher=$2
	if run_launcher "$launcher" 23 0; then
		echo "$name reported success after start-stop-daemon failed" >&2
		exit 1
	fi
	if run_launcher "$launcher" 23 9; then
		echo "$name reported success when both launch and logging failed" >&2
		exit 1
	fi
}
assert_success()
{
	name=$1
	launcher=$2
	if ! run_launcher "$launcher" 0 9; then
		echo "$name reported failure because logger failed after successful launch" >&2
		exit 1
	fi
}
assert_failure Transmission "$TMP_DIR/transmission.sh"
assert_failure Aria "$TMP_DIR/aria.sh"
assert_success Transmission "$TMP_DIR/transmission.sh"
assert_success Aria "$TMP_DIR/aria.sh"
run_aria_config_failure()
{
	phase=$1
	cat_status=$2
	mv_status=$3
	failure_root="$TMP_DIR/aria-$phase"
	mkdir -p "$failure_root/config" "$TMP_DIR/Downloads"
	: > "$failure_root/config/incomplete.lst"
	sed -e "s|SVC_PATH=\"/usr/bin/aria2c\"|SVC_PATH=\"$STUB_DIR/aria2c\"|" \
	    -e "s|DIR_LINK=\"/mnt/aria\"|DIR_LINK=\"$failure_root\"|" \
	    "$ARIA_SRC" > "$TMP_DIR/aria-$phase.sh"
	marker="$failure_root/start-invoked"
	if run_launcher "$TMP_DIR/aria-$phase.sh" 0 0 "$cat_status" "$mv_status" "$marker"; then
		echo "Aria started despite $phase failure while publishing default config" >&2
		exit 1
	fi
	[ ! -f "$marker" ] || {
		echo "Aria daemon launch was attempted after $phase failure" >&2
		exit 1
	}
	[ ! -f "$failure_root/config/aria2.conf" ] || {
		echo "Aria published a config after $phase failure" >&2
		exit 1
	}
	for tmp_config in "$failure_root"/config/aria2.conf.tmp.*; do
		[ ! -e "$tmp_config" ] || {
			echo "Aria left a temporary config after $phase failure" >&2
			exit 1
		}
	done
}
run_aria_config_failure "config-write" 9 0
run_aria_config_failure "config-rename" 0 9
echo "Transmission and Aria startup exit-status regression: PASS"
