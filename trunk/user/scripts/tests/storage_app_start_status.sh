#!/bin/sh
# Validate Transmission and Aria start-state and exit-status propagation.
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
[ -z "${STORAGE_TEST_TRACE_FILE:-}" ] || printf 'pidof called=%s running=%s post=%s initial=%s\n' \
	"${STORAGE_TEST_LAUNCH_CALLED_FILE:-}" "${STORAGE_TEST_RUNNING_FILE:-}" \
	"${STORAGE_TEST_POST_PIDOF_STATUS:-}" "${STORAGE_TEST_START_PIDOF_STATUS:-}" >> "$STORAGE_TEST_TRACE_FILE"
if [ -e "${STORAGE_TEST_LAUNCH_CALLED_FILE:-/nonexistent}" ]; then
	if [ "${STORAGE_TEST_POST_PIDOF_STATUS:-auto}" = "2" ]; then
		exit 2
	fi
	if [ -e "$STORAGE_TEST_RUNNING_FILE" ]; then
		printf '%s\n' 123
		exit 0
	fi
	exit 1
fi
if [ -n "${STORAGE_TEST_START_PIDOF_OUTPUT:-}" ]; then
	printf '%s\n' "$STORAGE_TEST_START_PIDOF_OUTPUT"
fi
exit "${STORAGE_TEST_START_PIDOF_STATUS:-1}"
EOF
cat > "$STUB_DIR/nvram" <<'EOF'
#!/bin/sh
case "$2" in
	trmd_pport) echo 51413 ;;
	trmd_rport) echo 9091 ;;
	aria_pport) echo 16888 ;;
	aria_rport) echo 6800 ;;
	http_username) echo admin ;;
	http_passwd) echo unused ;;
	http_proto) echo 0 ;;
	*) exit 1 ;;
esac
EOF
cat > "$STUB_DIR/hexdump" <<'EOF'
#!/bin/sh
if [ "${STORAGE_TEST_HEXDUMP_STATUS:-0}" -ne 0 ]; then
	exit "$STORAGE_TEST_HEXDUMP_STATUS"
fi
printf '%s' '0123456789abcdef0123456789abcdef'
EOF
chmod 755 "$STUB_DIR/hexdump"

cat > "$STUB_DIR/start-stop-daemon" <<'EOF'
#!/bin/sh
[ -z "${STORAGE_TEST_TRACE_FILE:-}" ] || echo start-stop-daemon >> "$STORAGE_TEST_TRACE_FILE"
[ -z "${STORAGE_TEST_START_MARKER:-}" ] || printf '%s\n' "$" >> "$STORAGE_TEST_START_MARKER"
: > "$STORAGE_TEST_LAUNCH_CALLED_FILE"
if [ "${STORAGE_TEST_START_STATUS:-23}" -ne 0 ]; then
	exit "$STORAGE_TEST_START_STATUS"
fi
if [ "${STORAGE_TEST_START_NO_PID:-0}" = 0 ]; then
	: > "$STORAGE_TEST_PENDING_FILE"
fi
exit 0
EOF
cat > "$STUB_DIR/sleep" <<'EOF'
#!/bin/sh
[ -z "${STORAGE_TEST_TRACE_FILE:-}" ] || echo "sleep: $*" >> "$STORAGE_TEST_TRACE_FILE"
printf '%s\n' "$*" >> "$STORAGE_TEST_SLEEP_LOG"
if [ -e "$STORAGE_TEST_PENDING_FILE" ]; then
	rm -f "$STORAGE_TEST_PENDING_FILE"
	: > "$STORAGE_TEST_RUNNING_FILE"
	if [ -n "${STORAGE_TEST_PROC_ROOT:-}" ]; then
		mkdir -p "$STORAGE_TEST_PROC_ROOT/123"
		ln -sf "$STORAGE_TEST_DAEMON_EXE" "$STORAGE_TEST_PROC_ROOT/123/exe"
		if [ "${STORAGE_TEST_POST_PIDOF_STATUS:-auto}" = 2 ]; then
			printf '%s\n' 'malformed process stat' > "$STORAGE_TEST_PROC_ROOT/123/stat"
		else
			printf '%s (%s) S 1 1 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 1 0 2001\n' 123 "$STORAGE_TEST_DAEMON_COMM" > "$STORAGE_TEST_PROC_ROOT/123/stat"
		fi
	fi
	exit 0
fi
exec /bin/sleep "$@"
EOF
cat > "$STUB_DIR/logger" <<'EOF'
#!/bin/sh
[ -z "${STORAGE_TEST_LOGGER_TRACE_FILE:-}" ] ||
    printf 'logger:%s\n' "$*" >> "$STORAGE_TEST_LOGGER_TRACE_FILE"
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

cat > "$STUB_DIR/aria2c" <<'EOF'
#!/bin/sh
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
exec /bin/chmod "$@"
EOF
if [ -n "${BUSYBOX_BIN:-}" ]; then
	# Transmission's procfs identity check requires stat; provide the target
	# firmware BusyBox applet on the same PATH as the other test commands.
	ln -s "$BUSYBOX_BIN" "$STUB_DIR/stat"
fi
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
	pidof_status=${18:-1}
	pidof_output=${19:-}
	start_no_pid=${20:-0}
	post_pidof_status=${21:-auto}
	hexdump_status=${22:-0}
	launch_state_dir=$(mktemp -d "$TMP_DIR/launch-state.XXXXXX")
	launch_called_file="$launch_state_dir/called"
	pending_file="$launch_state_dir/pending"
	running_file="$launch_state_dir/running"
	sleep_log="$launch_state_dir/sleep.log"
	trace_file="$launch_state_dir/trace.log"
	output_file="$launch_state_dir/output.log"
	proc_root="$launch_state_dir/proc"
	mkdir -p "$proc_root/1"
	ln -s /bin/sleep "$proc_root/1/exe"
	printf '%s\n' '1 (init) S 1 1 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 1 0 1001' > "$proc_root/1/stat"
	case "$launcher" in
		*aria*.sh) daemon_comm=aria2c; daemon_exe="$STUB_DIR/aria2c" ;;
		*) daemon_comm=transmission-daemon; daemon_exe="$STUB_DIR/transmission-daemon" ;;
	esac
	LAST_LAUNCH_STATE_DIR=$launch_state_dir
	LAST_LAUNCH_OUTPUT=$output_file
	if [ -n "${BUSYBOX_BIN:-}" ]; then
		PATH="$STUB_DIR:$PATH" STORAGE_TEST_START_STATUS="$start_status" STORAGE_TEST_LOGGER_STATUS="$logger_status" STORAGE_TEST_CAT_STATUS="$cat_status" STORAGE_TEST_MV_STATUS="$mv_status" STORAGE_TEST_START_MARKER="$start_marker" STORAGE_TEST_START_PIDOF_STATUS="$pidof_status" STORAGE_TEST_START_PIDOF_OUTPUT="$pidof_output" STORAGE_TEST_POST_PIDOF_STATUS="$post_pidof_status" STORAGE_TEST_START_NO_PID="$start_no_pid" STORAGE_TEST_LAUNCH_CALLED_FILE="$launch_called_file" STORAGE_TEST_PENDING_FILE="$pending_file" STORAGE_TEST_RUNNING_FILE="$running_file" STORAGE_TEST_SLEEP_LOG="$sleep_log" STORAGE_TEST_TRACE_FILE="$trace_file" STORAGE_TEST_LOGGER_TRACE_FILE="$trace_file.logger" STORAGE_TEST_PROC_ROOT="$proc_root" STORAGE_TEST_DAEMON_EXE="$daemon_exe" STORAGE_TEST_DAEMON_COMM="$daemon_comm" TRANSMISSION_PROC_ROOT="$proc_root" ARIA_PROC_ROOT="$proc_root" STORAGE_TEST_CAT_BARRIER="$cat_barrier" STORAGE_TEST_CONFIG_DIR="$config_dir" STORAGE_TEST_TEMP_COUNT="$temp_count" STORAGE_TEST_GENERATOR_STATUS="$generator_status" STORAGE_TEST_SED_STATUS="$sed_status" STORAGE_TEST_GENERATOR_EMPTY="$generator_empty" STORAGE_TEST_GENERATOR_NO_UMASK="$generator_no_umask" STORAGE_TEST_MKDIR_FAIL_PATH="$mkdir_fail_path" STORAGE_TEST_CHMOD_FAIL_PATH="$chmod_fail_path" STORAGE_TEST_CHOWN_FAIL_PATH="$chown_fail_path" STORAGE_TEST_HEXDUMP_STATUS="$hexdump_status" CONFIG_TMP="$inherited_config_tmp" \
"$BUSYBOX_BIN" ash -x "$launcher" start >"$output_file" 2>&1
	else
		PATH="$STUB_DIR:$PATH" STORAGE_TEST_START_STATUS="$start_status" STORAGE_TEST_LOGGER_STATUS="$logger_status" STORAGE_TEST_CAT_STATUS="$cat_status" STORAGE_TEST_MV_STATUS="$mv_status" STORAGE_TEST_START_MARKER="$start_marker" STORAGE_TEST_START_PIDOF_STATUS="$pidof_status" STORAGE_TEST_START_PIDOF_OUTPUT="$pidof_output" STORAGE_TEST_POST_PIDOF_STATUS="$post_pidof_status" STORAGE_TEST_START_NO_PID="$start_no_pid" STORAGE_TEST_LAUNCH_CALLED_FILE="$launch_called_file" STORAGE_TEST_PENDING_FILE="$pending_file" STORAGE_TEST_RUNNING_FILE="$running_file" STORAGE_TEST_SLEEP_LOG="$sleep_log" STORAGE_TEST_TRACE_FILE="$trace_file" STORAGE_TEST_LOGGER_TRACE_FILE="$trace_file.logger" STORAGE_TEST_PROC_ROOT="$proc_root" STORAGE_TEST_DAEMON_EXE="$daemon_exe" STORAGE_TEST_DAEMON_COMM="$daemon_comm" TRANSMISSION_PROC_ROOT="$proc_root" ARIA_PROC_ROOT="$proc_root" STORAGE_TEST_CAT_BARRIER="$cat_barrier" STORAGE_TEST_CONFIG_DIR="$config_dir" STORAGE_TEST_TEMP_COUNT="$temp_count" STORAGE_TEST_GENERATOR_STATUS="$generator_status" STORAGE_TEST_SED_STATUS="$sed_status" STORAGE_TEST_GENERATOR_EMPTY="$generator_empty" STORAGE_TEST_GENERATOR_NO_UMASK="$generator_no_umask" STORAGE_TEST_MKDIR_FAIL_PATH="$mkdir_fail_path" STORAGE_TEST_CHMOD_FAIL_PATH="$chmod_fail_path" STORAGE_TEST_CHOWN_FAIL_PATH="$chown_fail_path" STORAGE_TEST_HEXDUMP_STATUS="$hexdump_status" CONFIG_TMP="$inherited_config_tmp" \
sh "$launcher" start >"$output_file" 2>&1
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
		cat "$LAST_LAUNCH_OUTPUT" >&2 || :
		cat "$LAST_LAUNCH_STATE_DIR/trace.log" >&2 || :
		cat "$LAST_LAUNCH_STATE_DIR/trace.log.logger" >&2 || :
		exit 1
	fi
	[ -e "$LAST_LAUNCH_STATE_DIR/running" ] || {
		echo "$name returned success before its delayed daemon PID appeared" >&2
		exit 1
	}
	[ "$(wc -l < "$LAST_LAUNCH_STATE_DIR/sleep.log" | tr -d ' ')" -ge 1 ] || {
		echo "$name did not wait for its daemonized child" >&2
		exit 1
	}
}
assert_failure Transmission "$TMP_DIR/transmission.sh"
assert_failure Aria "$TMP_DIR/aria.sh"
assert_success Transmission "$TMP_DIR/transmission.sh"
assert_success Aria "$TMP_DIR/aria.sh"

expect_start_readiness_failure()
{
	name=$1
	launcher=$2
	no_pid=$3
	post_status=$4
	marker="$TMP_DIR/$name-launch-marker"
	rm -f "$marker"
	if run_launcher "$launcher" 0 0 0 0 "$marker" "" "" "" 0 0 0 "" "" "" "" 0 1 "" "$no_pid" "$post_status"; then
		echo "$name reported success without a verified daemon" >&2
		exit 1
	fi
	[ -e "$marker" ] || {
		echo "$name did not reach the daemon launch" >&2
		exit 1
	}
	if [ "$post_status" = auto ]; then
		[ "$(wc -l < "$LAST_LAUNCH_STATE_DIR/sleep.log" | tr -d ' ')" -eq 3 ] || {
			echo "$name did not enforce the bounded readiness timeout" >&2
			exit 1
		}
	else
		expected_sleeps=0
		# The Transmission fixture creates its synthetic /proc entry on the
		# first readiness sleep; that scan then becomes UNKNOWN and must abort
		# without further retries. Aria2's pidof error is immediate.
		case "$name" in
			Transmission-unknown-after-launch|Aria-unknown-after-launch) expected_sleeps=1 ;;
		esac
		actual_sleeps=$(wc -l < "$LAST_LAUNCH_STATE_DIR/sleep.log" | tr -d ' ')
		[ "$actual_sleeps" -eq "$expected_sleeps" ] || {
			echo "$name had $actual_sleeps readiness sleeps (expected $expected_sleeps for UNKNOWN)" >&2
			exit 1
		}
	fi
}
expect_start_readiness_failure Transmission-no-child "$TMP_DIR/transmission.sh" 1 auto
expect_start_readiness_failure Aria-no-child "$TMP_DIR/aria.sh" 1 auto
expect_start_readiness_failure Transmission-unknown-after-launch "$TMP_DIR/transmission.sh" 0 2
expect_start_readiness_failure Aria-unknown-after-launch "$TMP_DIR/aria.sh" 0 2

expect_start_pidof_unknown()
{
	name=$1
	launcher=$2
	pidof_status=$3
	pidof_output=$4
	marker="$TMP_DIR/$name-start-marker"
	rm -f "$marker"
	if run_launcher "$launcher" 0 0 0 0 "$marker" "" "" "" 0 0 0 "" "" "" "" 0 "$pidof_status" "$pidof_output"; then
		echo "$name started or reported success with an unknown pidof result" >&2
		exit 1
	fi
	[ ! -e "$marker" ] || {
		echo "$name launched a daemon after an unknown pidof result" >&2
		exit 1
	}
}
expect_start_pidof_running()
{
	name=$1
	launcher=$2
	marker="$TMP_DIR/$name-start-marker"
	rm -f "$marker"
	if ! run_launcher "$launcher" 0 0 0 0 "$marker" "" "" "" 0 0 0 "" "" "" "" 0 0 "101 202"; then
		echo "$name failed to accept a valid existing daemon snapshot" >&2
		exit 1
	fi
	[ ! -e "$marker" ] || {
		echo "$name attempted a duplicate daemon launch" >&2
		exit 1
	}
}
# Daemon lifecycle behavior is covered by the package-specific identity tests.
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
# Exercise both production stop functions against deterministic PID state.
STOP_STUB_DIR="$TMP_DIR/stop-bin"
mkdir -p "$STOP_STUB_DIR"
cat > "$STOP_STUB_DIR/pidof" <<'EOF'
#!/bin/sh
if [ "${STORAGE_TEST_STOP_PIDOF_EMPTY_SUCCESS:-0}" -eq 1 ]; then
	exit 0
fi
case "${STORAGE_TEST_STOP_PIDOF_STATUS:-0}" in
	0) ;;
	1) exit 1 ;;
	*) exit "$STORAGE_TEST_STOP_PIDOF_STATUS" ;;
esac
[ -s "$STORAGE_TEST_STOP_STATE" ] || exit 1
if { [ -e "$STORAGE_TEST_STOP_KILLALL_LOG" ] ||
     [ -e "${STORAGE_TEST_STOP_TERM_LOG:-/nonexistent}" ]; } &&
   [ -n "${STORAGE_TEST_STOP_POST_PIDOF_STATUS:-}" ]; then
	[ -z "${STORAGE_TEST_STOP_POST_PIDOF_OUTPUT:-}" ] ||
		printf '%s\n' "$STORAGE_TEST_STOP_POST_PIDOF_OUTPUT"
	exit "$STORAGE_TEST_STOP_POST_PIDOF_STATUS"
fi
# Match pidof's whitespace-separated output, not the test file's line records.
tr '\012' ' ' < "$STORAGE_TEST_STOP_STATE"
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

create_stop_runner()
{
	source_file=$1
	output_file=$2
	{
		cat <<'EOF'
#!/bin/sh
SVC_NAME="${STORAGE_TEST_STOP_SERVICE_NAME:-StorageTest}"
kill()
{
	[ "$#" -eq 2 ] || return 64
	case "$1" in
		-TERM)
			printf '%s\n' "$2" >> "$STORAGE_TEST_STOP_TERM_LOG"
			[ "${STORAGE_TEST_STOP_KILLALL_CLEAR:-0}" -eq 1 ] || return 0
			;;
		-9)
			printf '%s\n' "$2" >> "$STORAGE_TEST_SIGNAL_LOG"
			[ "${STORAGE_TEST_KILL_STATUS:-0}" -eq 0 ] || return "$STORAGE_TEST_KILL_STATUS"
			;;
		*) return 64 ;;
	esac
	/bin/grep -vxF "$2" "$STORAGE_TEST_STOP_STATE" > "$STORAGE_TEST_STOP_STATE.tmp" || :
	/bin/mv "$STORAGE_TEST_STOP_STATE.tmp" "$STORAGE_TEST_STOP_STATE"
	return 0
}
EOF
		sed -n '/^func_stop()/,/^func_reload()/p' "$source_file" | sed '$d'
		printf '%s\n' 'func_stop'
	} > "$output_file"
	chmod 755 "$output_file"
}

run_stop_runner()
{
	runner_file=$1
	service_name=$2
	state_file=$3
	signal_log=$4
	kill_status=$5
	killall_clear=$6
	pidof_status=$7
	killall_status=$8
	pidof_empty_success=${9:-0}
	pidof_post_status=${10:-}
	pidof_post_output=${11:-}
	rm -f "$signal_log.term"
	if [ -n "${BUSYBOX_BIN:-}" ]; then
		PATH="$STOP_STUB_DIR:$STUB_DIR:$PATH" \
			STORAGE_TEST_STOP_SERVICE_NAME="$service_name" \
			STORAGE_TEST_STOP_STATE="$state_file" \
			STORAGE_TEST_SIGNAL_LOG="$signal_log" \
			STORAGE_TEST_STOP_TERM_LOG="$signal_log.term" \
			STORAGE_TEST_KILL_STATUS="$kill_status" \
			STORAGE_TEST_STOP_KILLALL_CLEAR="$killall_clear" \
			STORAGE_TEST_STOP_PIDOF_STATUS="$pidof_status" \
			STORAGE_TEST_STOP_KILLALL_STATUS="$killall_status" \
			STORAGE_TEST_STOP_KILLALL_LOG="$TMP_DIR/$service_name-killall.log" \
			STORAGE_TEST_STOP_PIDOF_EMPTY_SUCCESS="$pidof_empty_success" \
			STORAGE_TEST_STOP_POST_PIDOF_STATUS="$pidof_post_status" \
			STORAGE_TEST_STOP_POST_PIDOF_OUTPUT="$pidof_post_output" \
			"$BUSYBOX_BIN" ash "$runner_file" >/dev/null 2>&1
	else
		PATH="$STOP_STUB_DIR:$STUB_DIR:$PATH" \
			STORAGE_TEST_STOP_SERVICE_NAME="$service_name" \
			STORAGE_TEST_STOP_STATE="$state_file" \
			STORAGE_TEST_SIGNAL_LOG="$signal_log" \
			STORAGE_TEST_STOP_TERM_LOG="$signal_log.term" \
			STORAGE_TEST_KILL_STATUS="$kill_status" \
			STORAGE_TEST_STOP_KILLALL_CLEAR="$killall_clear" \
			STORAGE_TEST_STOP_PIDOF_STATUS="$pidof_status" \
			STORAGE_TEST_STOP_KILLALL_STATUS="$killall_status" \
			STORAGE_TEST_STOP_KILLALL_LOG="$TMP_DIR/$service_name-killall.log" \
			STORAGE_TEST_STOP_PIDOF_EMPTY_SUCCESS="$pidof_empty_success" \
			STORAGE_TEST_STOP_POST_PIDOF_STATUS="$pidof_post_status" \
			STORAGE_TEST_STOP_POST_PIDOF_OUTPUT="$pidof_post_output" \
			"$runner_file" >/dev/null 2>&1
	fi
}

check_stop_contracts()
{
	stop_prefix=$1
	stop_service=$2
	stop_source=$3
	stop_runner="$TMP_DIR/$stop_prefix-stop-runner.sh"
	create_stop_runner "$stop_source" "$stop_runner"

	# No daemon is a successful no-op and must not invoke killall.
	stop_state="$TMP_DIR/$stop_prefix-stop-absent-pids"
	stop_signals="$TMP_DIR/$stop_prefix-stop-absent-signals"
	stop_killall="$TMP_DIR/$stop_service-killall.log"
	: > "$stop_state"
	: > "$stop_signals"
	rm -f "$stop_killall"
	if ! run_stop_runner "$stop_runner" "$stop_service" "$stop_state" "$stop_signals" 0 0 1 0; then
		echo "$stop_service stop failed when pidof confirmed no daemon" >&2
		exit 1
	fi
	[ ! -e "$stop_killall" ] || { echo "$stop_service invoked killall with no daemon" >&2; exit 1; }
	[ ! -s "$stop_signals.term" ] || { echo "$stop_service sent SIGTERM with no daemon" >&2; exit 1; }

	# Graceful exit: validated per-PID SIGTERM or the existing Aria killall clears the snapshot.
	stop_state="$TMP_DIR/$stop_prefix-stop-graceful-pids"
	stop_signals="$TMP_DIR/$stop_prefix-stop-graceful-signals"
	printf '%s\n' 101 202 > "$stop_state"
	: > "$stop_signals"
	rm -f "$stop_killall"
	if ! run_stop_runner "$stop_runner" "$stop_service" "$stop_state" "$stop_signals" 0 1 0 0; then
		echo "$stop_service graceful stop failed" >&2
		exit 1
	fi
	[ ! -s "$stop_state" ] || { echo "$stop_service graceful stop left daemon PIDs" >&2; exit 1; }
	[ ! -s "$stop_signals" ] || { echo "$stop_service graceful stop sent an unnecessary SIGKILL" >&2; exit 1; }
	case "$stop_service" in
		Transmission)
			[ ! -e "$stop_killall" ] || {
				echo "$stop_service used process-name-wide killall after PID validation" >&2
				exit 1
			}
			[ "$(wc -l < "$stop_signals.term")" -eq 2 ] || {
				echo "$stop_service graceful stop did not signal both validated PIDs" >&2
				exit 1
			}
			[ "$(sed -n '1p' "$stop_signals.term")" = 101 ] &&
			[ "$(sed -n '2p' "$stop_signals.term")" = 202 ] || {
				echo "$stop_service graceful SIGTERM snapshot order changed" >&2
				exit 1
			}
			;;
		Aria2)
			[ ! -s "$stop_signals.term" ] || {
				echo "$stop_service unexpectedly used Transmission's SIGTERM path" >&2
				exit 1
			}
			;;
	esac

	# Forced stop sends one signal per PID; killall status is non-authoritative.
	stop_state="$TMP_DIR/$stop_prefix-stop-forced-pids"
	stop_signals="$TMP_DIR/$stop_prefix-stop-forced-signals"
	printf '%s\n' 303 404 > "$stop_state"
	: > "$stop_signals"
	if ! run_stop_runner "$stop_runner" "$stop_service" "$stop_state" "$stop_signals" 0 0 0 7; then
		echo "$stop_service forced stop failed after successful per-PID SIGKILL" >&2
		exit 1
	fi
	[ ! -s "$stop_state" ] || { echo "$stop_service forced stop left daemon PIDs" >&2; exit 1; }
	[ "$(wc -l < "$stop_signals")" -eq 2 ] || { echo "$stop_service did not signal each PID separately" >&2; exit 1; }
	[ "$(sed -n '1p' "$stop_signals")" = 303 ] && [ "$(sed -n '2p' "$stop_signals")" = 404 ] || {
		echo "$stop_service SIGKILL PID list was not processed individually" >&2
		exit 1
	}

	# Failed SIGKILL must not be reported as a successful stop.
	stop_state="$TMP_DIR/$stop_prefix-stop-failed-kill-pids"
	stop_signals="$TMP_DIR/$stop_prefix-stop-failed-kill-signals"
	printf '%s\n' 505 606 > "$stop_state"
	: > "$stop_signals"
	if run_stop_runner "$stop_runner" "$stop_service" "$stop_state" "$stop_signals" 9 0 0 0; then
		echo "$stop_service reported success while daemon PIDs survived SIGKILL failure" >&2
		exit 1
	fi
	[ -s "$stop_state" ] || { echo "$stop_service failed-kill fixture unexpectedly removed daemon PIDs" >&2; exit 1; }
	[ "$(wc -l < "$stop_signals")" -eq 2 ] || { echo "$stop_service did not attempt SIGKILL for every remaining PID" >&2; exit 1; }

	# Revalidate a fresh pidof snapshot after the graceful signal before
	# authorizing per-PID SIGKILL escalation.
	stop_state="$TMP_DIR/$stop_prefix-stop-post-invalid-pids"
	stop_signals="$TMP_DIR/$stop_prefix-stop-post-invalid-signals"
	printf '%s\n' 707 > "$stop_state"
	: > "$stop_signals"
	rm -f "$stop_killall"
	if run_stop_runner "$stop_runner" "$stop_service" "$stop_state" "$stop_signals" 0 0 0 0 0 0 0707; then
		echo "$stop_service accepted a leading-zero PID in the refreshed stop snapshot" >&2
		exit 1
	fi
	[ ! -s "$stop_signals" ] || { echo "$stop_service sent SIGKILL for an invalid refreshed snapshot" >&2; exit 1; }

	# Reject malformed and unsafe PID lists before sending any signal.
	for invalid_pids in "505 invalid" "0" "1" "0123" "   "; do
		stop_state="$TMP_DIR/$stop_prefix-stop-invalid-pids"
		stop_signals="$TMP_DIR/$stop_prefix-stop-invalid-signals"
		printf '%s\n' "$invalid_pids" > "$stop_state"
		: > "$stop_signals"
		rm -f "$stop_killall"
		if run_stop_runner "$stop_runner" "$stop_service" "$stop_state" "$stop_signals" 0 0 0 0; then
			echo "$stop_service accepted unsafe pidof output: $invalid_pids" >&2
			exit 1
		fi
		[ ! -s "$stop_signals" ] || { echo "$stop_service signaled an unsafe PID list" >&2; exit 1; }
		[ ! -e "$stop_killall" ] || { echo "$stop_service invoked killall with an invalid PID snapshot" >&2; exit 1; }
	done

	# A pidof error or a successful-but-empty response is UNKNOWN, not absent.
	stop_state="$TMP_DIR/$stop_prefix-stop-unknown-pids"
	stop_signals="$TMP_DIR/$stop_prefix-stop-unknown-signals"
	: > "$stop_state"
	: > "$stop_signals"
	rm -f "$stop_killall"
	if run_stop_runner "$stop_runner" "$stop_service" "$stop_state" "$stop_signals" 0 0 2 0; then
		echo "$stop_service treated pidof error as daemon absence" >&2
		exit 1
	fi
	[ ! -e "$stop_killall" ] || { echo "$stop_service invoked killall after pidof error" >&2; exit 1; }
	if run_stop_runner "$stop_runner" "$stop_service" "$stop_state" "$stop_signals" 0 0 0 0 1; then
		echo "$stop_service accepted an empty successful pidof response" >&2
		exit 1
	fi
	[ ! -e "$stop_killall" ] || { echo "$stop_service invoked killall after inconsistent pidof output" >&2; exit 1; }

	# Restart must not start after stop fails.
	restart_block=$(sed -n '/^restart)/,/^;;/p' "$stop_source")
	case "$restart_block" in
		*"func_stop || exit 1"*) ;;
		*) echo "$stop_service restart is not gated on successful stop" >&2; exit 1 ;;
	esac
}

# Transmission's PID/start-time stop and reload contracts are covered by
# transmission/tests/stop_pid_snapshot.sh and reload_status.sh.
# Aria2 stop/reload are covered by aria2/tests/aria2_process_identity.sh.

run_aria_config_failure()
{
	phase=$1
	cat_status=$2
	mv_status=$3
	hexdump_status=${4:-0}
	failure_root="$TMP_DIR/aria-$phase"
	mkdir -p "$failure_root/config" "$TMP_DIR/Downloads"
	: > "$failure_root/config/incomplete.lst"
	sed -e "s|SVC_PATH=\"/usr/bin/aria2c\"|SVC_PATH=\"$STUB_DIR/aria2c\"|" \
	    -e "s|DIR_LINK=\"/mnt/aria\"|DIR_LINK=\"$failure_root\"|" \
	    "$ARIA_SRC" > "$TMP_DIR/aria-$phase.sh"
	marker="$failure_root/start-invoked"
	if run_launcher "$TMP_DIR/aria-$phase.sh" 0 0 "$cat_status" "$mv_status" "$marker" "" "" "" 0 0 0 "" "" "" "" 0 1 "" 0 auto "$hexdump_status"; then
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
run_aria_config_failure "rpc-secret-generation" 0 0 9
assert_aria_rpc_secret()
{
	config_file=$1
	secret_file=$2
	actual_secret=$(sed -n 's/^rpc-secret=//p' "$config_file")
	case "$actual_secret" in
		''|*[!0123456789abcdef]*) echo "Aria RPC token is not lowercase hexadecimal" >&2; exit 1 ;;
	esac
	[ "${#actual_secret}" -eq 32 ] || { echo "Aria RPC token is not 128 bits" >&2; exit 1; }
	[ "$(wc -c < "$secret_file")" -eq 33 ] || { echo "Aria RPC token file length is invalid" >&2; exit 1; }
	[ "$(wc -l < "$secret_file")" -eq 1 ] || { echo "Aria RPC token file must contain exactly one line" >&2; exit 1; }
	[ "$(cat "$secret_file")" = "$actual_secret" ] || { echo "Aria config and persistent token differ" >&2; exit 1; }
	[ "$(stat -c '%a' "$secret_file")" = 600 ] || { echo "Persistent Aria RPC token is not mode 0600" >&2; exit 1; }
	[ "$(stat -c '%a' "$config_file")" = 600 ] || { echo "Aria RPC config is not mode 0600" >&2; exit 1; }
}
run_aria_config_migration()
{
	migration_root="$TMP_DIR/aria-legacy-rpc"
	migration_config_dir="$migration_root/config"
	mkdir -p "$migration_config_dir" "$migration_root/downloads" "$TMP_DIR/Downloads"
	: > "$migration_config_dir/incomplete.lst"
	printf '%s\n' 'rpc-listen-all=true' 'rpc-allow-origin-all=true' > "$migration_config_dir/aria2.conf"
	chmod 644 "$migration_config_dir/aria2.conf"
	sed -e "s|SVC_PATH=\"/usr/bin/aria2c\"|SVC_PATH=\"$STUB_DIR/aria2c\"|" \
	    -e "s|DIR_LINK=\"/mnt/aria\"|DIR_LINK=\"$migration_root\"|" \
	    "$ARIA_SRC" > "$TMP_DIR/aria-legacy-rpc.sh"
	marker="$migration_root/start-invoked"
	if ! run_launcher "$TMP_DIR/aria-legacy-rpc.sh" 0 0 0 0 "$marker"; then
		echo "Aria failed to migrate an existing unauthenticated all-interface RPC config" >&2
		cat "$LAST_LAUNCH_OUTPUT" >&2 || :
		exit 1
	fi
	grep -qx 'rpc-listen-all=true' "$migration_config_dir/aria2.conf" || {
		echo "Aria legacy RPC migration changed listener compatibility" >&2
		exit 1
	}
	assert_aria_rpc_secret "$migration_config_dir/aria2.conf" "$migration_config_dir/aria2.rpc-secret"
	[ "$(stat -c '%a' "$migration_config_dir/aria2.conf")" = 600 ] || {
		echo "Migrated Aria RPC config is not mode 0600" >&2
		exit 1
	}
	for tmp_secret in "$migration_config_dir"/aria2.rpc-secret.tmp.*; do
		[ ! -e "$tmp_secret" ] || { echo "Aria left an RPC token staging file" >&2; exit 1; }
	done
	for tmp_config in "$migration_config_dir"/aria2.conf.rpc-auth.tmp.*; do
		[ ! -e "$tmp_config" ] || {
			echo "Aria left a migrated RPC staging file" >&2
			exit 1
		}
	done
}
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
	assert_aria_rpc_secret "$config_dir/aria2.conf" "$config_dir/aria2.rpc-secret"
	[ "$(stat -c '%a' "$config_dir/aria2.conf")" = 600 ] || { echo "Generated RPC config mode is not 0600" >&2; exit 1; }
	for tmp_config in "$config_dir"/aria2.conf.tmp.*; do
		[ ! -e "$tmp_config" ] || {
			echo "Concurrent Aria start left a temporary config" >&2
			exit 1
		}
	done
}
run_aria_config_migration
run_aria_config_concurrency
echo "Transmission and Aria startup exit-status regression: PASS"
