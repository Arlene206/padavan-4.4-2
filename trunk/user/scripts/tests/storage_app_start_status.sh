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
printf '%s\n' '{"umask": 0, "download-dir": "/existing"}' > "$TMP_DIR/transmission/config/settings.json"
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
[ -z "${STORAGE_TEST_START_MARKER:-}" ] || printf '%s\n' "$$" >> "$STORAGE_TEST_START_MARKER"
exit "${STORAGE_TEST_START_STATUS:-23}"
EOF
cat > "$STUB_DIR/logger" <<'EOF'
#!/bin/sh
exit "${STORAGE_TEST_LOGGER_STATUS:-0}"
EOF
cat > "$STUB_DIR/transmission-daemon" <<'EOF'
#!/bin/sh
if [ "${STORAGE_TEST_GENERATOR_STATUS:-0}" -ne 0 ]; then
	exit "$STORAGE_TEST_GENERATOR_STATUS"
fi
if [ "${STORAGE_TEST_GENERATOR_EMPTY:-0}" -eq 0 ]; then
	if [ "${STORAGE_TEST_GENERATOR_NO_UMASK:-0}" -eq 0 ]; then
		printf '%s\n' '{"umask": 18, "download-dir": "/generated"}' >&2
	else
		printf '%s\n' '{"download-dir": "/generated"}' >&2
	fi
fi
exit 0
EOF
cat > "$STUB_DIR/sed" <<'EOF'
#!/bin/sh
if [ "${STORAGE_TEST_SED_STATUS:-0}" -ne 0 ]; then
	exit "$STORAGE_TEST_SED_STATUS"
fi
exec /bin/sed "$@"
EOF
cat > "$STUB_DIR/cat" <<'EOF'
#!/bin/sh
if [ "${STORAGE_TEST_CAT_STATUS:-0}" -ne 0 ]; then
	exit "$STORAGE_TEST_CAT_STATUS"
fi
if [ -n "${STORAGE_TEST_CAT_BARRIER:-}" ]; then
	: > "$STORAGE_TEST_CAT_BARRIER/entered.$$"
	attempt=0
	while :; do
		entered=0
		for marker in "$STORAGE_TEST_CAT_BARRIER"/entered.*; do
			[ -f "$marker" ] && entered=$((entered + 1))
		done
		[ "$entered" -ge 2 ] && break
		attempt=$((attempt + 1))
		[ "$attempt" -lt 30 ] || exit 91
		sleep 1
	done
	staged=0
	for path in "$STORAGE_TEST_CONFIG_DIR"/aria2.conf.tmp.*; do
		[ -f "$path" ] && staged=$((staged + 1))
	done
	printf '%s\n' "$staged" >> "$STORAGE_TEST_TEMP_COUNT"
	: > "$STORAGE_TEST_CAT_BARRIER/checked.$$"
	attempt=0
	while :; do
		checked=0
		for marker in "$STORAGE_TEST_CAT_BARRIER"/checked.*; do
			[ -f "$marker" ] && checked=$((checked + 1))
		done
		[ "$checked" -ge 2 ] && break
		attempt=$((attempt + 1))
		[ "$attempt" -lt 30 ] || exit 92
		sleep 1
	done
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
cat > "$STUB_DIR/mkdir" <<'EOF'
#!/bin/sh
if [ -n "${STORAGE_TEST_MKDIR_FAIL_PATH:-}" ]; then
	for path in "$@"; do
		[ "$path" = "$STORAGE_TEST_MKDIR_FAIL_PATH" ] && exit 9
	done
fi
exec /bin/mkdir "$@"
EOF
cat > "$STUB_DIR/chown" <<'EOF'
#!/bin/sh
if [ -n "${STORAGE_TEST_CHOWN_FAIL_PATH:-}" ]; then
	for path in "$@"; do
		[ "$path" = "$STORAGE_TEST_CHOWN_FAIL_PATH" ] && exit 9
	done
fi
exit 0
EOF
cat > "$STUB_DIR/chmod" <<'EOF'
#!/bin/sh
if [ -n "${STORAGE_TEST_CHMOD_FAIL_PATH:-}" ]; then
	last=""
	for path in "$@"; do last="$path"; done
	[ "$last" = "$STORAGE_TEST_CHMOD_FAIL_PATH" ] && exit 9
fi
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
	cat_barrier=${7:-}
	config_dir=${8:-}
	temp_count=${9:-}
	mkdir_fail_path=${13:-}
	chmod_fail_path=${14:-}
	chown_fail_path=${15:-}
	inherited_config_tmp=${16:-}
	generator_no_umask=${17:-0}
	generator_status=${10:-0}
	sed_status=${11:-0}
	generator_empty=${12:-0}
	if [ -n "${BUSYBOX_BIN:-}" ]; then
		PATH="$STUB_DIR:$PATH" STORAGE_TEST_START_STATUS="$start_status" STORAGE_TEST_LOGGER_STATUS="$logger_status" STORAGE_TEST_CAT_STATUS="$cat_status" STORAGE_TEST_MV_STATUS="$mv_status" STORAGE_TEST_START_MARKER="$start_marker" STORAGE_TEST_CAT_BARRIER="$cat_barrier" STORAGE_TEST_CONFIG_DIR="$config_dir" STORAGE_TEST_TEMP_COUNT="$temp_count" STORAGE_TEST_GENERATOR_STATUS="$generator_status" STORAGE_TEST_SED_STATUS="$sed_status" STORAGE_TEST_GENERATOR_EMPTY="$generator_empty" STORAGE_TEST_GENERATOR_NO_UMASK="$generator_no_umask" STORAGE_TEST_MKDIR_FAIL_PATH="$mkdir_fail_path" STORAGE_TEST_CHMOD_FAIL_PATH="$chmod_fail_path" STORAGE_TEST_CHOWN_FAIL_PATH="$chown_fail_path" CONFIG_TMP="$inherited_config_tmp" \
			"$BUSYBOX_BIN" ash "$launcher" start >/dev/null 2>&1
	else
		PATH="$STUB_DIR:$PATH" STORAGE_TEST_START_STATUS="$start_status" STORAGE_TEST_LOGGER_STATUS="$logger_status" STORAGE_TEST_CAT_STATUS="$cat_status" STORAGE_TEST_MV_STATUS="$mv_status" STORAGE_TEST_START_MARKER="$start_marker" STORAGE_TEST_CAT_BARRIER="$cat_barrier" STORAGE_TEST_CONFIG_DIR="$config_dir" STORAGE_TEST_TEMP_COUNT="$temp_count" STORAGE_TEST_GENERATOR_STATUS="$generator_status" STORAGE_TEST_SED_STATUS="$sed_status" STORAGE_TEST_GENERATOR_EMPTY="$generator_empty" STORAGE_TEST_GENERATOR_NO_UMASK="$generator_no_umask" STORAGE_TEST_MKDIR_FAIL_PATH="$mkdir_fail_path" STORAGE_TEST_CHMOD_FAIL_PATH="$chmod_fail_path" STORAGE_TEST_CHOWN_FAIL_PATH="$chown_fail_path" CONFIG_TMP="$inherited_config_tmp" \
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
run_transmission_config_failure()
{
	phase=$1
	generator_status=$2
	sed_status=$3
	mv_status=$4
	generator_empty=$5
	generator_no_umask=${6:-0}
	failure_root="$TMP_DIR/transmission-$phase"
	config_dir="$failure_root/config"
	mkdir -p "$config_dir" "$failure_root/incomplete" "$failure_root/watch" "$TMP_DIR/Downloads"
	sed -e "s|SVC_PATH=\"/usr/bin/transmission-daemon\"|SVC_PATH=\"$STUB_DIR/transmission-daemon\"|" \
	    -e "s|DIR_LINK=\"/mnt/transmission\"|DIR_LINK=\"$failure_root\"|" \
	    "$TRANSMISSION_SRC" > "$TMP_DIR/transmission-$phase.sh"
	marker="$failure_root/start-invoked"
	if run_launcher "$TMP_DIR/transmission-$phase.sh" 0 0 0 "$mv_status" "$marker" "" "$config_dir" "" "$generator_status" "$sed_status" "$generator_empty" "" "" "" "" "$generator_no_umask"; then
		echo "Transmission started despite $phase failure while publishing default settings" >&2
		exit 1
	fi
	[ ! -e "$marker" ] || { echo "Transmission daemon launch was attempted after $phase failure" >&2; exit 1; }
	[ ! -e "$config_dir/settings.json" ] || { echo "Transmission published settings after $phase failure" >&2; exit 1; }
	for tmp_settings in "$config_dir"/settings.json.tmp.*; do
		[ ! -e "$tmp_settings" ] || { echo "Transmission left a staging file after $phase failure" >&2; exit 1; }
	done
}
run_transmission_config_success()
{
	success_root="$TMP_DIR/transmission-config-success"
	config_dir="$success_root/config"
	mkdir -p "$config_dir" "$success_root/incomplete" "$success_root/watch" "$TMP_DIR/Downloads"
	: > "$config_dir/settings.json"
	sed -e "s|SVC_PATH=\"/usr/bin/transmission-daemon\"|SVC_PATH=\"$STUB_DIR/transmission-daemon\"|" \
	    -e "s|DIR_LINK=\"/mnt/transmission\"|DIR_LINK=\"$success_root\"|" \
	    "$TRANSMISSION_SRC" > "$TMP_DIR/transmission-config-success.sh"
	marker="$success_root/start-invoked"
	if ! run_launcher "$TMP_DIR/transmission-config-success.sh" 0 9 0 0 "$marker" "" "$config_dir" ""; then
		echo "Transmission failed despite successful default settings publication" >&2
		exit 1
	fi
	[ -s "$config_dir/settings.json" ] || { echo "Transmission did not publish nonempty settings" >&2; exit 1; }
	grep -q '"umask": 0,' "$config_dir/settings.json" || { echo "Transmission did not apply the umask transform" >&2; exit 1; }
	grep -q '"umask": 18,' "$config_dir/settings.json" && { echo "Transmission published untransformed settings" >&2; exit 1; }
	[ -f "$marker" ] || { echo "Transmission did not reach daemon launch after settings publication" >&2; exit 1; }
	for tmp_settings in "$config_dir"/settings.json.tmp.*; do
		[ ! -e "$tmp_settings" ] || { echo "Transmission left a staging file after success" >&2; exit 1; }
	done
}
run_transmission_config_failure generator 9 0 0 0
run_transmission_config_failure empty-output 0 0 0 1
run_transmission_config_failure missing-umask 0 0 0 0 1
run_transmission_config_failure transform 0 9 0 0
run_transmission_config_failure publish 0 0 9 0
run_transmission_config_success
run_transmission_staging_collision()
{
	collision_root="$TMP_DIR/transmission-stage-collision"
	config_dir="$collision_root/config"
	mkdir -p "$config_dir" "$collision_root/incomplete" "$collision_root/watch" "$TMP_DIR/Downloads"
	collision_file="$config_dir/settings.json.tmp.collision-test"
	printf '%s\n' 'preserve-existing-stage' > "$collision_file"
	sed -e "s|SVC_PATH=\"/usr/bin/transmission-daemon\"|SVC_PATH=\"$STUB_DIR/transmission-daemon\"|" \
	    -e "s|DIR_LINK=\"/mnt/transmission\"|DIR_LINK=\"$collision_root\"|" \
	    -e 's|tmp\.\$\$|tmp.collision-test|' \
	    "$TRANSMISSION_SRC" > "$TMP_DIR/transmission-stage-collision.sh"
	marker="$collision_root/start-invoked"
	if run_launcher "$TMP_DIR/transmission-stage-collision.sh" 0 0 0 0 "$marker" "" "$config_dir" ""; then
		echo "Transmission started despite a pre-existing settings staging collision" >&2
		exit 1
	fi
	[ ! -e "$marker" ] || { echo "Transmission launched after a staging collision" >&2; exit 1; }
	[ ! -e "$config_dir/settings.json" ] || { echo "Transmission published settings after a staging collision" >&2; exit 1; }
	[ -f "$collision_file" ] || { echo "Transmission deleted a pre-existing staging file" >&2; exit 1; }
	[ "$(cat "$collision_file")" = "preserve-existing-stage" ] || { echo "Transmission modified a pre-existing staging file" >&2; exit 1; }
}
run_transmission_staging_collision

run_transmission_setup_failure()
{
	phase=$1
	mode=$2
	fail_path=$3
	inherited_tmp=${4:-}
	setup_root="$TMP_DIR/transmission-setup-$phase"
	link_root="$setup_root/mount"
	mkdir -p "$link_root"
	if [ -n "$inherited_tmp" ]; then printf "preserve-me\n" > "$inherited_tmp"; fi
	mkdir_fail_path=""
	chmod_fail_path=""
	chown_fail_path=""
	case "$mode" in
		mkdir) mkdir_fail_path="$fail_path" ;;
		chmod) chmod_fail_path="$fail_path" ;;
		chown) chown_fail_path="$fail_path" ;;
		*) echo "Unknown setup failure mode: $mode" >&2; exit 1 ;;
	esac
	sed -e "s|SVC_PATH=\"/usr/bin/transmission-daemon\"|SVC_PATH=\"$STUB_DIR/transmission-daemon\"|" \
	    -e "s|DIR_LINK=\"/mnt/transmission\"|DIR_LINK=\"$link_root\"|" \
	    "$TRANSMISSION_SRC" > "$TMP_DIR/transmission-setup-$phase.sh"
	marker="$link_root/start-invoked"
	if run_launcher "$TMP_DIR/transmission-setup-$phase.sh" 0 0 0 0 "$marker" "" "" "" 0 0 0 "$mkdir_fail_path" "$chmod_fail_path" "$chown_fail_path" "$inherited_tmp"; then
		echo "Transmission started despite $phase setup failure" >&2
		exit 1
	fi
	[ ! -e "$marker" ] || { echo "Transmission daemon launch was attempted after $phase setup failure" >&2; exit 1; }
	if [ -n "$inherited_tmp" ]; then
		[ -s "$inherited_tmp" ] || { echo "Transmission removed an inherited CONFIG_TMP path during $phase failure" >&2; exit 1; }
	fi
	for tmp_settings in "$link_root/config"/settings.json.tmp.*; do
		[ ! -e "$tmp_settings" ] || { echo "Transmission left a staging file after $phase setup failure" >&2; exit 1; }
	done
}
run_transmission_setup_failure mkdir-config mkdir "$TMP_DIR/transmission-setup-mkdir-config/mount/config"
run_transmission_setup_failure inherited-config-tmp mkdir "$TMP_DIR/transmission-setup-inherited-config-tmp/mount/config" "$TMP_DIR/transmission-inherited-config-tmp"
run_transmission_setup_failure mkdir-downloads mkdir "$TMP_DIR/transmission-setup-mkdir-downloads/mount/downloads"
run_transmission_setup_failure mkdir-incomplete mkdir "$TMP_DIR/transmission-setup-mkdir-incomplete/mount/incomplete"
run_transmission_setup_failure mkdir-watch mkdir "$TMP_DIR/transmission-setup-mkdir-watch/mount/watch"
run_transmission_setup_failure chmod-watch chmod "$TMP_DIR/transmission-setup-chmod-watch/mount/watch"
run_transmission_setup_failure chmod-root chmod "$TMP_DIR/transmission-setup-chmod-root/mount"
run_transmission_setup_failure chown-config chown "$TMP_DIR/transmission-setup-chown-config/mount/config"
run_transmission_setup_failure chown-incomplete chown "$TMP_DIR/transmission-setup-chown-incomplete/mount/incomplete"
run_transmission_setup_failure chmod-downloads chmod "$TMP_DIR/transmission-setup-chmod-downloads/mount/downloads"
# Exercise the extracted production stop function with deterministic PID state.
STOP_STUB_DIR="$TMP_DIR/stop-bin"
mkdir -p "$STOP_STUB_DIR"
cat > "$STOP_STUB_DIR/pidof" <<'EOF'
#!/bin/sh
[ -s "$STORAGE_TEST_STOP_STATE" ] || exit 1
exec /bin/cat "$STORAGE_TEST_STOP_STATE"
EOF
cat > "$STOP_STUB_DIR/killall" <<'EOF'
#!/bin/sh
[ -z "${STORAGE_TEST_STOP_KILLALL_LOG:-}" ] || printf '%s\n' "$*" >> "$STORAGE_TEST_STOP_KILLALL_LOG"
if [ "${STORAGE_TEST_STOP_KILLALL_CLEAR:-0}" -eq 1 ]; then
	: > "$STORAGE_TEST_STOP_STATE"
fi
exit "${STORAGE_TEST_STOP_KILLALL_STATUS:-0}"
EOF
cat > "$STOP_STUB_DIR/sleep" <<'EOF'
#!/bin/sh
exit 0
EOF
chmod 755 "$STOP_STUB_DIR/"*
{
	cat <<'EOF'
#!/bin/sh
SVC_NAME=Transmission
kill()
{
	[ "$#" -eq 2 ] && [ "$1" = "-9" ] || return 64
	printf '%s\n' "$2" >> "$STORAGE_TEST_SIGNAL_LOG"
	[ "${STORAGE_TEST_KILL_STATUS:-0}" -eq 0 ] || return "$STORAGE_TEST_KILL_STATUS"
	/bin/grep -vxF "$2" "$STORAGE_TEST_STOP_STATE" > "$STORAGE_TEST_STOP_STATE.tmp" || :
	/bin/mv "$STORAGE_TEST_STOP_STATE.tmp" "$STORAGE_TEST_STOP_STATE"
	return 0
}
EOF
	sed -n '/^func_stop()/,/^func_reload()/p' "$TRANSMISSION_SRC" | sed '$d'
	printf '%s\n' 'func_stop'
} > "$TMP_DIR/transmission-stop-runner.sh"
chmod 755 "$TMP_DIR/transmission-stop-runner.sh"

run_transmission_stop()
{
	state_file=$1
	signal_log=$2
	kill_status=$3
	killall_clear=$4
	PATH="$STOP_STUB_DIR:$STUB_DIR:$PATH" \
		STORAGE_TEST_STOP_STATE="$state_file" \
		STORAGE_TEST_SIGNAL_LOG="$signal_log" \
		STORAGE_TEST_KILL_STATUS="$kill_status" \
		STORAGE_TEST_STOP_KILLALL_CLEAR="$killall_clear" \
		"$TMP_DIR/transmission-stop-runner.sh" >/dev/null 2>&1
}

# Graceful exit: no SIGKILL should be needed.
stop_state="$TMP_DIR/stop-graceful-pids"
stop_signals="$TMP_DIR/stop-graceful-signals"
stop_killall="$TMP_DIR/stop-graceful-killall"
printf '%s\n' 101 202 > "$stop_state"
if ! STORAGE_TEST_STOP_STATE="$stop_state" STORAGE_TEST_STOP_KILLALL_LOG="$stop_killall" \
	STORAGE_TEST_STOP_KILLALL_CLEAR=1 PATH="$STOP_STUB_DIR:$STUB_DIR:$PATH" \
	"$TMP_DIR/transmission-stop-runner.sh" >/dev/null 2>&1; then
	echo "Transmission graceful stop failed" >&2
	exit 1
fi
[ ! -s "$stop_state" ] || { echo "Transmission graceful stop left daemon PIDs" >&2; exit 1; }
[ ! -s "$stop_signals" ] || { echo "Transmission graceful stop sent an unnecessary SIGKILL" >&2; exit 1; }

# Forced stop must signal each PID separately and verify final absence.
stop_state="$TMP_DIR/stop-forced-pids"
stop_signals="$TMP_DIR/stop-forced-signals"
: > "$stop_state"
: > "$stop_signals"
printf '%s\n' 303 404 > "$stop_state"
if ! run_transmission_stop "$stop_state" "$stop_signals" 0 0; then
	echo "Transmission forced stop failed after successful per-PID SIGKILL" >&2
	exit 1
fi
[ ! -s "$stop_state" ] || { echo "Transmission forced stop left daemon PIDs" >&2; exit 1; }
[ "$(wc -l < "$stop_signals")" -eq 2 ] || { echo "Transmission did not signal each PID separately" >&2; exit 1; }
[ "$(sed -n '1p' "$stop_signals")" = 303 ] && [ "$(sed -n '2p' "$stop_signals")" = 404 ] || {
	echo "Transmission SIGKILL PID list was not processed individually" >&2
	exit 1
}

# Failed SIGKILL must not be reported as a successful stop.
stop_state="$TMP_DIR/stop-failed-kill-pids"
stop_signals="$TMP_DIR/stop-failed-kill-signals"
: > "$stop_signals"
printf '%s\n' 505 606 > "$stop_state"
if run_transmission_stop "$stop_state" "$stop_signals" 9 0; then
	echo "Transmission reported success while daemon PIDs survived SIGKILL failure" >&2
	exit 1
fi
[ -s "$stop_state" ] || { echo "Failed SIGKILL fixture unexpectedly removed daemon PIDs" >&2; exit 1; }
[ "$(wc -l < "$stop_signals")" -eq 2 ] || { echo "Transmission did not attempt SIGKILL for every remaining PID" >&2; exit 1; }

# Reject malformed pidof output without turning it into a signal argument.
stop_state="$TMP_DIR/stop-invalid-pids"
stop_signals="$TMP_DIR/stop-invalid-signals"
: > "$stop_signals"
printf '%s\n' '505 invalid' > "$stop_state"
if run_transmission_stop "$stop_state" "$stop_signals" 0 0; then
	echo "Transmission accepted malformed pidof output" >&2
	exit 1
fi
[ ! -s "$stop_signals" ] || { echo "Transmission signaled an invalid PID list" >&2; exit 1; }

# Restart must not proceed to the start action when stop fails.
restart_block=$(sed -n '/^restart)/,/^;;/p' "$TRANSMISSION_SRC")
case "$restart_block" in
	*"func_stop || exit 1"*) ;;
	*) echo "Transmission restart is not gated on successful stop" >&2; exit 1 ;;
esac

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
run_aria_config_concurrency()
{
	concurrency_root="$TMP_DIR/aria-concurrency"
	config_dir="$concurrency_root/config"
	barrier="$concurrency_root/barrier"
	mkdir -p "$config_dir" "$concurrency_root/Downloads" "$barrier"
	: > "$config_dir/incomplete.lst"
	sed -e "s|SVC_PATH=\"/usr/bin/aria2c\"|SVC_PATH=\"$STUB_DIR/aria2c\"|" \
	    -e "s|DIR_LINK=\"/mnt/aria\"|DIR_LINK=\"$concurrency_root\"|" \
	    "$ARIA_SRC" > "$TMP_DIR/aria-concurrency.sh"
	marker="$concurrency_root/start-invoked"
	temp_count="$concurrency_root/temp-counts"
	: > "$temp_count"
	run_launcher "$TMP_DIR/aria-concurrency.sh" 0 0 0 0 "$marker" "$barrier" "$config_dir" "$temp_count" &
	first_pid=$!
	run_launcher "$TMP_DIR/aria-concurrency.sh" 0 0 0 0 "$marker" "$barrier" "$config_dir" "$temp_count" &
	second_pid=$!
	if ! wait "$first_pid"; then
		echo "First concurrent Aria start failed" >&2
		exit 1
	fi
	if ! wait "$second_pid"; then
		echo "Second concurrent Aria start failed" >&2
		exit 1
	fi
	[ "$(wc -l < "$temp_count")" -eq 2 ] || {
		echo "Concurrent Aria starts did not both observe staged configs" >&2
		exit 1
	}
	while IFS= read -r staged_count; do
		[ "$staged_count" -eq 2 ] || {
			echo "Concurrent Aria starts reused a staging filename" >&2
			exit 1
		}
	done < "$temp_count"
	[ "$(wc -l < "$marker")" -eq 2 ] || {
		echo "Both Aria starts did not reach the daemon launch" >&2
		exit 1
	}
	[ -f "$config_dir/aria2.conf" ] || {
		echo "Concurrent Aria starts did not publish a config" >&2
		exit 1
	}
	for tmp_config in "$config_dir"/aria2.conf.tmp.*; do
		[ ! -e "$tmp_config" ] || {
			echo "Concurrent Aria start left a temporary config" >&2
			exit 1
		}
	done
}
run_aria_config_concurrency
echo "Transmission and Aria startup exit-status regression: PASS"
