#!/bin/sh
set -eu

TEST_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/shadowsocks-lifecycle.XXXXXX")
trap 'rm -rf "$TMP_DIR"' EXIT HUP INT TERM
mkdir -p "$TMP_DIR/bin" "$TMP_DIR/state" "$TMP_DIR/proc"
export PROCESS_STATE_DIR="$TMP_DIR/state"
export SS_PROC_ROOT="$TMP_DIR/proc"
export KILL_LOG="$TMP_DIR/kill.log"
export PATH="$TMP_DIR/bin:$PATH"

cat > "$TMP_DIR/bin/pidof" <<'EOF'
#!/bin/sh
[ "$#" -eq 1 ] || exit 2
[ -f "$PROCESS_STATE_DIR/pidof_unknown" ] && exit 2
[ -f "$PROCESS_STATE_DIR/pidof_incomplete" ] && exit 1
case "$1" in
	empty_success) exit 0 ;;
	malformed) echo '4242 bad'; exit 0 ;;
	inconsistent) echo '4242'; exit 1 ;;
esac
state_file="$PROCESS_STATE_DIR/$1"
[ -f "$state_file" ] || exit 1
if [ -f "$state_file.pids" ]; then
	cat "$state_file.pids"
else
	echo 4242
fi
EOF

cat > "$TMP_DIR/bin/pgrep" <<'EOF'
#!/bin/sh
[ "$#" -eq 2 ] && [ "$1" = "-f" ] || exit 2
[ -f "$PROCESS_STATE_DIR/pgrep_unknown" ] && exit 2
[ -f "$PROCESS_STATE_DIR/pgrep_incomplete" ] && exit 1
[ "$2" = "/usr/bin/ss-monitor" ] || exit 1
[ -f "$PROCESS_STATE_DIR/pgrep_empty_success" ] && exit 0
[ -f "$PROCESS_STATE_DIR/pgrep_malformed" ] && { echo '4243 bad'; exit 0; }
[ -f "$PROCESS_STATE_DIR/pgrep_inconsistent" ] && { echo 4243; exit 1; }
[ -f "$PROCESS_STATE_DIR/monitor" ] || exit 1
if [ -f "$PROCESS_STATE_DIR/monitor.pids" ]; then
	cat "$PROCESS_STATE_DIR/monitor.pids"
else
	echo 4243
fi
EOF

cat > "$TMP_DIR/bin/sleep" <<'EOF'
#!/bin/sh
# Yield briefly so detached mock daemons can publish their PID fixture.
exec /bin/sleep 0.1
EOF

cat > "$TMP_DIR/bin/dnsproxy" <<'EOF'
#!/bin/sh
case "${DNS_START_MODE:-ready}" in
	fail) exit 1 ;;
	unknown) : > "$PROCESS_STATE_DIR/pidof_unknown" ;;
esac
printf 'exit_on_term\n' > "$PROCESS_STATE_DIR/dnsproxy"
exit 0
EOF

cat > "$TMP_DIR/bin/dns2tcp" <<'EOF'
#!/bin/sh
case "${DNS_START_MODE:-ready}" in
	fail) exit 1 ;;
	unknown) : > "$PROCESS_STATE_DIR/pidof_unknown" ;;
esac
printf 'exit_on_term\n' > "$PROCESS_STATE_DIR/dns2tcp"
exit 0
EOF

cat > "$TMP_DIR/bin/ssr-redir" <<'EOF'
#!/bin/sh
case "${RUN_BIN_MODE:-ready}" in
	fail|absent) exit 19 ;;
	unknown) : > "$PROCESS_STATE_DIR/pidof_unknown" ;;
esac
printf 'exit_on_term\n' > "$PROCESS_STATE_DIR/ssr-redir"
if [ -f "$PROCESS_STATE_DIR/ssr-redir.pids" ]; then
	printf '4243\n' >> "$PROCESS_STATE_DIR/ssr-redir.pids"
else
	printf '4243\n' > "$PROCESS_STATE_DIR/ssr-redir.pids"
fi
exit 0
EOF

cat > "$TMP_DIR/bin/lua" <<'EOF'
#!/bin/sh
case "${RUN_BIN_MODE:-ready}" in
	fail|absent) exit 19 ;;
	unknown) : > "$PROCESS_STATE_DIR/pidof_unknown" ;;
esac
printf 'exit_on_term\n' > "$PROCESS_STATE_DIR/ipt2socks"
printf '4244\n' > "$PROCESS_STATE_DIR/ipt2socks.pids"
exit 0
EOF

cat > "$TMP_DIR/bin/chinadns-ng" <<'EOF'
#!/bin/sh
printf 'launched\n' >> "$PROCESS_STATE_DIR/chinadns_launches"
case "${CHINADNS_START_MODE:-ready}" in
	fail|absent) exit 19 ;;
	unknown) : > "$PROCESS_STATE_DIR/pidof_unknown" ;;
esac
printf 'exit_on_term\n' > "$PROCESS_STATE_DIR/chinadns-ng"
printf '4247\n' > "$PROCESS_STATE_DIR/chinadns-ng.pids"
exit 0
EOF

cat > "$TMP_DIR/bin/dnsmasq" <<'EOF'
#!/bin/sh
if [ "${1:-}" = "--test" ]; then
	[ "$#" -eq 3 ] && [ "$2" = "-C" ] && [ -r "$3" ] || exit 2
	[ "${DNSMASQ_TEST_MODE:-ready}" != "fail" ] || exit 19
	grep -q '^dns-forward-max=1000$' "$3" || exit 18
	exit 0
fi
run_count=0
[ ! -f "$PROCESS_STATE_DIR/dnsmasq_runs" ] || run_count=$(cat "$PROCESS_STATE_DIR/dnsmasq_runs")
run_count=$((run_count + 1))
printf '%s\n' "$run_count" > "$PROCESS_STATE_DIR/dnsmasq_runs"
case "${DNSMASQ_START_MODE:-ready}" in
	fail) exit 19 ;;
	rollback) [ "$run_count" -eq 1 ] && exit 19 ;;
	unknown) : > "$PROCESS_STATE_DIR/pidof_unknown" ;;
esac
printf 'exit_on_term\n' > "$PROCESS_STATE_DIR/dnsmasq"
printf '4248\n' > "$PROCESS_STATE_DIR/dnsmasq.pids"
exit 0
EOF
chmod 755 "$TMP_DIR/bin/dnsmasq"
chmod 755 "$TMP_DIR/bin/ssr-redir" "$TMP_DIR/bin/lua" "$TMP_DIR/bin/chinadns-ng"

chmod 755 "$TMP_DIR/bin/dnsproxy" "$TMP_DIR/bin/dns2tcp"
chmod 755 "$TMP_DIR/bin/pidof" "$TMP_DIR/bin/pgrep" "$TMP_DIR/bin/sleep"
log() { :; }
# The stop helper must signal only the captured PID after executable and
# start_time revalidation; this shell function emulates those PID outcomes.
kill() {
	[ "$#" -eq 2 ] || return 2
	case "$1" in -TERM) ss_test_signal=TERM ;; -KILL) ss_test_signal=KILL ;; *) return 2 ;; esac
	case "$2" in ''|*[!0-9]*|0*) return 2 ;; esac
	printf 'PID-%s %s\n' "$ss_test_signal" "$2" >> "$KILL_LOG"
	ss_test_dir="$SS_PROC_ROOT/$2"
	[ -d "$ss_test_dir" ] || return 0
	ss_test_mode=$(cat "$ss_test_dir/stop_mode" 2>/dev/null || printf never_exit)
	case "$ss_test_signal:$ss_test_mode" in
		TERM:exit_on_term|KILL:exit_on_kill)
			rm -rf "$ss_test_dir"
			;;
		TERM:unknown_after_term)
			rm -f "$ss_test_dir/exe"
			;;
		TERM:reuse_as_foreign)
			rm -f "$ss_test_dir/exe"
			ln -s /usr/bin/untrusted-daemon "$ss_test_dir/exe"
			printf '%s\n' untrusted > "$ss_test_dir/comm"
			printf '%s (untrusted) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 5001\n' \
				"$2" > "$ss_test_dir/stat"
			;;
		TERM:reuse_as_same)
			printf '%s (%s) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 5001\n' \
				"$2" "$(cat "$ss_test_dir/comm")" > "$ss_test_dir/stat"
			;;
	esac
	return 0
}
nvram() {
	case "$1:$2" in
		get:pdnsd_enable) echo "${DNS_PDNSD_ENABLE:-0}" ;;
		get:tunnel_forward) echo "${DNS_UPSTREAM-8.8.8.8#53}" ;;
		get:ss_cgroups) echo "${MOCK_SS_CGROUPS:-0}" ;;
		get:ss_watchcat) echo "${WATCHCAT_ENABLED:-0}" ;;
		*) return 90 ;;
	esac
}
ipset() { return 0; }
. "$TEST_DIR/../scripts/shadowsocks_process_lifecycle.sh"
. "$TEST_DIR/../scripts/dnsmasq_config_lifecycle.sh"
sed -n \
	-e '/^run_bin() {/,/^}/p' \
	-e '/^ss_start_chinadns_daemon() {/,/^}/p' \
	-e '/^ss_restart_dhcpd() {/,/^}/p' \
	-e '/^ss_publish_and_restart_dnsmasq() {/,/^}/p' \
	-e '/^stop_dns_proxy() {/,/^}/p' \
	-e '/^ss_dns_proxy_config_read() {/,/^}/p' \
	-e '/^start_dns_proxy() {/,/^}/p' \
	-e '/^ss_launch_watchcat() {/,/^}/p' \
	-e '/^start_watchcat() {/,/^}/p' \
	"$TEST_DIR/../scripts/shadowsocks.sh" > "$TMP_DIR/dns-proxy-functions.sh"
. "$TMP_DIR/dns-proxy-functions.sh"

# Replace the firmware RC entry point with an injected process-state model.
DNSMASQ_MOCK_BIN="$TMP_DIR/bin/dnsmasq"
DNSMASQ_RESTART_CALLS=0
ss_restart_dhcpd() {
	DNSMASQ_RESTART_CALLS=$((DNSMASQ_RESTART_CALLS + 1))
	if ss_stop_named_processes dnsmasq; then
		:
	else
		return $?
	fi
	"$DNSMASQ_MOCK_BIN" || return $?
	rm -rf "$SS_PROC_ROOT/4248"
	add_stop_proc_fixture 4248 dnsmasq /user/sbin/dnsmasq /user/sbin/dnsmasq exit_on_term 4000
	ss_wait_named_process_count dnsmasq 1 5
}


reset_proc_fixture() {
	rm -rf "$SS_PROC_ROOT"
	mkdir -p "$SS_PROC_ROOT/1"
	printf '%s\n' '1 (init) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 4000' > "$SS_PROC_ROOT/1/stat"
	printf '%s\n' init > "$SS_PROC_ROOT/1/comm"
	printf '/sbin/init\000' > "$SS_PROC_ROOT/1/cmdline"
}

add_proc_fixture() {
	ss_test_pid=$1
	ss_test_comm=$2
	ss_test_argv0=$3
	shift 3
	mkdir -p "$SS_PROC_ROOT/$ss_test_pid"
	printf '%s (%s) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 4000\n' \
		"$ss_test_pid" "$ss_test_comm" > "$SS_PROC_ROOT/$ss_test_pid/stat"
	printf '%s\n' "$ss_test_comm" > "$SS_PROC_ROOT/$ss_test_pid/comm"
	printf '%s\000' "$ss_test_argv0" > "$SS_PROC_ROOT/$ss_test_pid/cmdline"
	for ss_test_arg in "$@"; do printf '%s\000' "$ss_test_arg" >> "$SS_PROC_ROOT/$ss_test_pid/cmdline"; done
}

add_stop_proc_fixture() {
	ss_test_pid=$1
	ss_test_comm=$2
	ss_test_argv0=$3
	ss_test_exe=$4
	ss_test_mode=$5
	ss_test_start=$6
	shift 6
	add_proc_fixture "$ss_test_pid" "$ss_test_comm" "$ss_test_argv0" "$@"
	printf '%s (%s) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 %s\n' \
		"$ss_test_pid" "$ss_test_comm" "$ss_test_start" > "$SS_PROC_ROOT/$ss_test_pid/stat"
	ln -s "$ss_test_exe" "$SS_PROC_ROOT/$ss_test_pid/exe"
	printf '%s\n' "$ss_test_mode" > "$SS_PROC_ROOT/$ss_test_pid/stop_mode"
}

fail() { echo "FAIL: $*" >&2; exit 1; }
expect_status() {
	expected=$1
	label=$2
	shift 2
	if "$@"; then actual=0; else actual=$?; fi
	[ "$actual" -eq "$expected" ] || fail "$label status=$actual expected=$expected"
}

# Status 1 is only ABSENT after a complete procfs scan.
reset_proc_fixture

# BusyBox pidof/pgrep can return status 1 after procps_scan skips a live entry.
reset_proc_fixture
add_proc_fixture 4242 dnsproxy /usr/bin/dnsproxy --foreground
: > "$PROCESS_STATE_DIR/pidof_incomplete"
expect_status 0 "pidof incomplete procfs scan finds a live daemon" ss_process_pid_state dnsproxy
recovered_pid_count=$(ss_process_pid_count dnsproxy)
[ "$recovered_pid_count" = "1" ] || fail "procfs pidof fallback count was $recovered_pid_count"
rm -f "$PROCESS_STATE_DIR/pidof_incomplete"

reset_proc_fixture
add_proc_fixture 4243 sh /bin/sh /usr/bin/ss-monitor
: > "$PROCESS_STATE_DIR/pgrep_incomplete"
expect_status 0 "pgrep incomplete procfs scan finds monitor" ss_process_pattern_state "/usr/bin/ss-monitor"
recovered_pattern_count=$(ss_process_pattern_count "/usr/bin/ss-monitor")
[ "$recovered_pattern_count" = "1" ] || fail "procfs pgrep fallback count was $recovered_pattern_count"
rm -f "$PROCESS_STATE_DIR/pgrep_incomplete"

# A malformed visible task record must make an incomplete inventory UNKNOWN.
reset_proc_fixture
mkdir -p "$SS_PROC_ROOT/4244"
printf '%s\n' 'truncated stat record' > "$SS_PROC_ROOT/4244/stat"
printf '/usr/bin/unrelated\000' > "$SS_PROC_ROOT/4244/cmdline"
: > "$PROCESS_STATE_DIR/pidof_incomplete"
expect_status 2 "pidof incomplete scan with malformed procfs is UNKNOWN" ss_process_pid_state dnsproxy
rm -f "$PROCESS_STATE_DIR/pidof_incomplete"
: > "$PROCESS_STATE_DIR/pgrep_incomplete"
expect_status 2 "pgrep incomplete scan with malformed procfs is UNKNOWN" ss_process_pattern_state "/usr/bin/ss-monitor"
rm -f "$PROCESS_STATE_DIR/pgrep_incomplete"
reset_proc_fixture
echo "Shadowsocks BusyBox incomplete-procfs absence rejection: PASS"

if absent_count=$(ss_process_pid_count absent_startup); then
	[ "$absent_count" = "0" ] || fail "ABSENT pidof count was $absent_count, expected 0"
else
	absent_status=$?
	fail "ABSENT pidof lookup failed under set -e (status=$absent_status)"
fi
if absent_pattern_count=$(ss_process_pattern_count "/usr/bin/absent-monitor"); then
	[ "$absent_pattern_count" = "0" ] || fail "ABSENT pgrep count was $absent_pattern_count, expected 0"
else
	absent_pattern_status=$?
	fail "ABSENT pgrep lookup failed under set -e (status=$absent_pattern_status)"
fi


# A successful command must provide a non-empty, canonical PID list.
expect_status 2 "pidof empty success is unknown" ss_process_pid_state empty_success
expect_status 2 "pidof malformed PID output is unknown" ss_process_pid_state malformed
expect_status 2 "pidof failure with output is unknown" ss_process_pid_state inconsistent

# A candidate whose executable metadata is unreadable is UNKNOWN: no signal.
: > "$KILL_LOG"
reset_proc_fixture
add_stop_proc_fixture 5100 dnsproxy /usr/bin/dnsproxy /usr/bin/dnsproxy never_exit 4000
printf '%s\n' 'truncated stat record' > "$SS_PROC_ROOT/5100/stat"
expect_status 2 "incomplete executable identity blocks named stop" ss_stop_named_processes dnsproxy
[ ! -s "$KILL_LOG" ] || fail "incomplete executable identity caused a signal"

# If the executable link becomes unavailable after TERM, do not escalate.
: > "$KILL_LOG"
reset_proc_fixture
add_stop_proc_fixture 5101 dnsproxy /usr/bin/dnsproxy /usr/bin/dnsproxy unknown_after_term 4000
expect_status 2 "unknown after TERM blocks KILL" ss_stop_named_processes dnsproxy
grep -Fqx 'PID-TERM 5101' "$KILL_LOG" || fail "named stop did not signal the captured PID"
if grep -Fqx 'PID-KILL 5101' "$KILL_LOG"; then fail "named KILL sent after identity became unknown"; fi

# A deleted executable at the expected path is UNKNOWN, never a safe target.
: > "$KILL_LOG"
reset_proc_fixture
add_stop_proc_fixture 5107 dnsproxy /usr/bin/dnsproxy '/usr/bin/dnsproxy (deleted)' never_exit 4000
expect_status 2 "deleted executable path blocks named stop" ss_stop_named_processes dnsproxy
[ ! -s "$KILL_LOG" ] || fail "deleted executable received a signal"

# A same-comm process running another executable is an impostor, not a target.
: > "$KILL_LOG"
reset_proc_fixture
add_stop_proc_fixture 5102 dnsproxy /usr/bin/dnsproxy /usr/bin/untrusted-daemon exit_on_term 4000
expect_status 2 "same-comm foreign executable blocks stop" ss_stop_named_processes dnsproxy
[ ! -s "$KILL_LOG" ] || fail "same-comm impostor received a signal"

# TERM is sent only to the exact executable and is not escalated after exit.
: > "$KILL_LOG"
reset_proc_fixture
add_stop_proc_fixture 5103 dnsproxy /usr/bin/dnsproxy /usr/bin/dnsproxy exit_on_term 4000
ss_stop_named_processes dnsproxy
[ ! -d "$SS_PROC_ROOT/5103" ] || fail "TERM process was not stopped"
grep -Fqx 'PID-TERM 5103' "$KILL_LOG" || fail "TERM was not scoped to the captured PID"
if grep -Fqx 'PID-KILL 5103' "$KILL_LOG"; then fail "named stop escalated after TERM exit"; fi

# PID reuse to a different executable after TERM must not receive KILL.
: > "$KILL_LOG"
reset_proc_fixture
add_stop_proc_fixture 5104 dnsproxy /usr/bin/dnsproxy /usr/bin/dnsproxy reuse_as_foreign 4000
ss_stop_named_processes dnsproxy
grep -Fqx 'PID-TERM 5104' "$KILL_LOG" || fail "PID-reuse fixture did not receive its initial TERM"
if grep -Fqx 'PID-KILL 5104' "$KILL_LOG"; then fail "reused PID with a foreign executable received KILL"; fi
[ -L "$SS_PROC_ROOT/5104/exe" ] || fail "PID-reuse impostor fixture disappeared"

# Reuse with the same executable is still a different process generation.
: > "$KILL_LOG"
reset_proc_fixture
add_stop_proc_fixture 5108 dnsproxy /usr/bin/dnsproxy /usr/bin/dnsproxy reuse_as_same 4000
expect_status 1 "same-executable PID reuse remains an incomplete stop" ss_stop_named_processes dnsproxy
grep -Fqx 'PID-TERM 5108' "$KILL_LOG" || fail "same-executable reuse fixture did not receive TERM"
if grep -Fqx 'PID-KILL 5108' "$KILL_LOG"; then fail "new process generation inherited old PID KILL authorization"; fi
grep -Fq '18 5001' "$SS_PROC_ROOT/5108/stat" || fail "same-executable PID reuse fixture did not change start_time"

# Preserve graceful TERM, bounded KILL escalation, and stuck-process failure.
: > "$KILL_LOG"
reset_proc_fixture
add_stop_proc_fixture 5105 dnsproxy /usr/bin/dnsproxy /usr/bin/dnsproxy exit_on_kill 4000
ss_stop_named_processes dnsproxy
[ ! -d "$SS_PROC_ROOT/5105" ] || fail "KILL escalation did not stop the daemon"
grep -Fqx 'PID-TERM 5105' "$KILL_LOG" || fail "daemon did not receive TERM"
grep -Fqx 'PID-KILL 5105' "$KILL_LOG" || fail "daemon did not receive KILL"

: > "$KILL_LOG"
reset_proc_fixture
add_stop_proc_fixture 5106 dnsproxy /usr/bin/dnsproxy /usr/bin/dnsproxy never_exit 4000
expect_status 1 "stuck named process remains a stop failure" ss_stop_named_processes dnsproxy
[ -d "$SS_PROC_ROOT/5106" ] || fail "stuck named process fixture disappeared"
grep -Fqx 'PID-TERM 5106' "$KILL_LOG" || fail "stuck daemon did not receive TERM"
grep -Fqx 'PID-KILL 5106' "$KILL_LOG" || fail "stuck daemon did not receive KILL"

# Keep the legacy pidof/pgrep status and procfs-readiness coverage.
: > "$PROCESS_STATE_DIR/pgrep_empty_success"
expect_status 2 "pgrep empty success is unknown" ss_process_pattern_state '/usr/bin/ss-monitor'
rm -f "$PROCESS_STATE_DIR/pgrep_empty_success"
: > "$PROCESS_STATE_DIR/pgrep_malformed"
expect_status 2 "pgrep malformed PID output is unknown" ss_process_pattern_state '/usr/bin/ss-monitor'
rm -f "$PROCESS_STATE_DIR/pgrep_malformed"
: > "$PROCESS_STATE_DIR/pgrep_inconsistent"
expect_status 2 "pgrep failure with output is unknown" ss_process_pattern_state '/usr/bin/ss-monitor'
rm -f "$PROCESS_STATE_DIR/pgrep_inconsistent"

# A malformed script identity must fail closed before sending a signal.
: > "$KILL_LOG"
reset_proc_fixture
add_stop_proc_fixture 5110 sh /bin/sh /bin/busybox exit_on_term 4000 /usr/bin/ss-monitor
printf '%s\n' 'truncated stat record' > "$SS_PROC_ROOT/5110/stat"
expect_status 2 "malformed monitor identity blocks stop" ss_stop_process_pattern '/usr/bin/ss-monitor'
[ ! -s "$KILL_LOG" ] || fail "malformed monitor identity caused a signal"

# Another shell argument containing the script path is not a script owner.
: > "$KILL_LOG"
reset_proc_fixture
add_stop_proc_fixture 5111 sh /bin/sh /bin/busybox exit_on_term 4000 /tmp/other.sh /usr/bin/ss-monitor
expect_status 0 "monitor path in argv[2] is ignored" ss_stop_process_pattern '/usr/bin/ss-monitor'
[ ! -s "$KILL_LOG" ] || fail "unrelated shell mentioning monitor path was signalled"

# If the script process loses its executable identity after TERM, do not KILL.
: > "$KILL_LOG"
reset_proc_fixture
add_stop_proc_fixture 5112 sh /bin/sh /bin/busybox unknown_after_term 4000 /usr/bin/ss-monitor
expect_status 2 "unknown after monitor TERM blocks KILL" ss_stop_process_pattern '/usr/bin/ss-monitor'
grep -Fqx 'PID-TERM 5112' "$KILL_LOG" || fail "monitor stop did not target the captured PID"
if grep -Fqx 'PID-KILL 5112' "$KILL_LOG"; then fail "monitor KILL sent after identity became unknown"; fi

: > "$KILL_LOG"
reset_proc_fixture
add_stop_proc_fixture 5113 sh /bin/sh /bin/busybox exit_on_term 4000 /usr/bin/ss-monitor
ss_stop_process_pattern '/usr/bin/ss-monitor'
[ ! -d "$SS_PROC_ROOT/5113" ] || fail "monitor was not stopped"
grep -Fqx 'PID-TERM 5113' "$KILL_LOG" || fail "monitor TERM was not PID-scoped"
if grep -Fqx 'PID-KILL 5113' "$KILL_LOG"; then fail "monitor escalated after TERM exit"; fi

: > "$KILL_LOG"
reset_proc_fixture
add_stop_proc_fixture 5114 sh /bin/sh /bin/busybox exit_on_kill 4000 /usr/bin/ss-monitor
ss_stop_process_pattern '/usr/bin/ss-monitor'
[ ! -d "$SS_PROC_ROOT/5114" ] || fail "monitor KILL escalation failed"
grep -Fqx 'PID-TERM 5114' "$KILL_LOG" || fail "monitor did not receive TERM"
grep -Fqx 'PID-KILL 5114' "$KILL_LOG" || fail "monitor did not receive KILL"

: > "$KILL_LOG"
reset_proc_fixture
add_stop_proc_fixture 5115 sh /bin/sh /bin/busybox never_exit 4000 /usr/bin/ss-monitor
expect_status 1 "stuck monitor remains a stop failure" ss_stop_process_pattern '/usr/bin/ss-monitor'
[ -d "$SS_PROC_ROOT/5115" ] || fail "stuck monitor fixture disappeared"
grep -Fqx 'PID-TERM 5115' "$KILL_LOG" || fail "stuck monitor did not receive TERM"
grep -Fqx 'PID-KILL 5115' "$KILL_LOG" || fail "stuck monitor did not receive KILL"

# Count delta prevents an existing sibling from masking a failed worker launch.
: > "$PROCESS_STATE_DIR/ssr-redir"
printf '4242\n' > "$PROCESS_STATE_DIR/ssr-redir.pids"
export RUN_BIN_MODE=ready
expect_status 0 "new detached worker readiness" run_bin ssr-redir -c /tmp/test.json
[ "$(wc -w < "$PROCESS_STATE_DIR/ssr-redir.pids")" -eq 2 ] || fail "run_bin accepted only the pre-existing sibling"
rm -f "$PROCESS_STATE_DIR/ssr-redir" "$PROCESS_STATE_DIR/ssr-redir.pids"

# gensocks.lua starts ipt2socks and exits; monitor the actual long-lived child.
RUN_BIN_MODE=ready
expect_status 0 "Lua-generated ipt2socks readiness" run_bin --wait-for ipt2socks lua /etc_ro/ss/gensocks.lua node1 1080
[ -f "$PROCESS_STATE_DIR/ipt2socks" ] || fail "Lua-generated ipt2socks child was not observed"
rm -f "$PROCESS_STATE_DIR/ipt2socks" "$PROCESS_STATE_DIR/ipt2socks.pids"

# ChinaDNS-ng is successful only when its named daemon is actually observable.
rm -f "$PROCESS_STATE_DIR/chinadns-ng" "$PROCESS_STATE_DIR/chinadns-ng.pids" "$PROCESS_STATE_DIR/chinadns_launches" "$PROCESS_STATE_DIR/pidof_unknown"
CHINADNS_START_MODE=ready
export CHINADNS_START_MODE
expect_status 1 "ChinaDNS-ng rejects empty upstream" ss_start_chinadns_daemon "" ""
[ ! -s "$PROCESS_STATE_DIR/chinadns_launches" ] || fail "empty ChinaDNS upstream launched a child"
expect_status 0 "ChinaDNS-ng detached readiness" ss_start_chinadns_daemon "223.5.5.5" ""
[ -f "$PROCESS_STATE_DIR/chinadns-ng" ] || fail "ChinaDNS-ng child was not observed"
[ "$(wc -l < "$PROCESS_STATE_DIR/chinadns_launches")" -eq 1 ] || fail "ChinaDNS-ng was not launched exactly once"
CHINADNS_START_MODE=fail
expect_status 0 "ChinaDNS-ng duplicate startup is idempotent" ss_start_chinadns_daemon "223.5.5.5" ""
[ "$(wc -l < "$PROCESS_STATE_DIR/chinadns_launches")" -eq 1 ] || fail "existing ChinaDNS-ng instance was relaunched"
rm -f "$PROCESS_STATE_DIR/chinadns-ng" "$PROCESS_STATE_DIR/chinadns-ng.pids"
CHINADNS_START_MODE=fail
expect_status 1 "ChinaDNS-ng immediate child exit" ss_start_chinadns_daemon "223.5.5.5" ""
[ ! -f "$PROCESS_STATE_DIR/chinadns-ng" ] || fail "failed ChinaDNS-ng child was reported as running"
CHINADNS_START_MODE=unknown
expect_status 2 "ChinaDNS-ng UNKNOWN startup inventory" ss_start_chinadns_daemon "223.5.5.5" ""
rm -f "$PROCESS_STATE_DIR/pidof_unknown" "$PROCESS_STATE_DIR/chinadns-ng" "$PROCESS_STATE_DIR/chinadns-ng.pids"

# dnsmasq config is staged, validated and atomically published before service restart.
dnsmasq_config="$TMP_DIR/dnsmasq.conf"
cat > "$dnsmasq_config" <<'EOF'
# valid-test baseline; preserve comments and non-owned DNS rules.
# no-resolv and min-cache-ttl-extra in this comment must not be treated as directives.
server=1.1.1.1
no-resolv
no-resolv-extra=yes
server=127.0.0.1
server=127.0.0.1#5353
server=127.0.0.1#6053
server=127.0.0.1#65353
server=/domain.example/127.0.0.1#5353
min-cache-ttl=60
min-cache-ttl-extra=7
dns-forward-max=20
dns-forward-max-extra=30
EOF
DNSMASQ_TEST_MODE=ready
DNSMASQ_START_MODE=ready
export DNSMASQ_TEST_MODE DNSMASQ_START_MODE
printf 'exit_on_term\n' > "$PROCESS_STATE_DIR/dnsmasq"
printf '4246\n' > "$PROCESS_STATE_DIR/dnsmasq.pids"
reset_proc_fixture
add_stop_proc_fixture 4246 dnsmasq /user/sbin/dnsmasq /user/sbin/dnsmasq exit_on_term 4000
# dnsmasq is not part of the Shadowsocks cgroup, even when that option is enabled.
MOCK_SS_CGROUPS=1
export MOCK_SS_CGROUPS
expect_status 0 "dnsmasq atomic config and restart" ss_publish_and_restart_dnsmasq "$dnsmasq_config" "$TMP_DIR/bin/dnsmasq" 1
MOCK_SS_CGROUPS=0
if ! grep -Fqx 'server=127.0.0.1#65353' "$dnsmasq_config"; then
	echo "dnsmasq config after successful ChinaDNS transaction:" >&2
	sed -n 'l' "$dnsmasq_config" >&2
	fail "ChinaDNS upstream was not published"
fi
[ "$DNSMASQ_RESTART_CALLS" -eq 1 ] || fail "successful transaction did not use the RC restart entry point"
[ "$(grep -c '^no-resolv$' "$dnsmasq_config")" -eq 1 ] || fail "no-resolv directive was not normalized"
[ "$(grep -c '^min-cache-ttl=1800$' "$dnsmasq_config")" -eq 1 ] || fail "min-cache-ttl was not normalized"
[ "$(grep -c '^dns-forward-max=1000$' "$dnsmasq_config")" -eq 1 ] || fail "dns-forward-max was not normalized"
grep -Fqx "# no-resolv and min-cache-ttl-extra in this comment must not be treated as directives." "$dnsmasq_config" || fail "ChinaDNS staging removed a comment"
grep -Fqx 'no-resolv-extra=yes' "$dnsmasq_config" || fail "ChinaDNS staging removed a similarly named no-resolv option"
if grep -Fqx 'server=127.0.0.1' "$dnsmasq_config"; then fail "ChinaDNS staging retained the unmanaged default loopback server"; fi
[ "$(grep -c '^server=127.0.0.1#65353$' "$dnsmasq_config")" -eq 1 ] || fail "ChinaDNS upstream was duplicated"
grep -Fqx 'server=127.0.0.1#5353' "$dnsmasq_config" || fail "ChinaDNS staging removed SmartDNS loopback port 5353"
grep -Fqx 'server=127.0.0.1#6053' "$dnsmasq_config" || fail "ChinaDNS staging removed SmartDNS loopback port 6053"
grep -Fqx 'server=/domain.example/127.0.0.1#5353' "$dnsmasq_config" || fail "ChinaDNS staging removed a domain-scoped server rule"
grep -Fqx 'min-cache-ttl-extra=7' "$dnsmasq_config" || fail "ChinaDNS staging removed a similarly named cache option"
grep -Fqx 'dns-forward-max-extra=30' "$dnsmasq_config" || fail "ChinaDNS staging removed a similarly named forward option"
[ -f "$PROCESS_STATE_DIR/dnsmasq" ] || fail "dnsmasq replacement process was not observed"

cp "$dnsmasq_config" "$TMP_DIR/dnsmasq.before-validation-failure"
DNSMASQ_TEST_MODE=fail
expect_status 1 "invalid dnsmasq staged config rejected" ss_publish_and_restart_dnsmasq "$dnsmasq_config" "$TMP_DIR/bin/dnsmasq" 0
cmp -s "$dnsmasq_config" "$TMP_DIR/dnsmasq.before-validation-failure" || fail "invalid config changed live file"
[ "$DNSMASQ_RESTART_CALLS" -eq 1 ] || fail "invalid staged config invoked the RC restart entry point"
[ -f "$PROCESS_STATE_DIR/dnsmasq" ] || fail "invalid staged config stopped active daemon"
DNSMASQ_TEST_MODE=ready

# An unverified stop must not replace the live config or launch a new daemon.
cp "$dnsmasq_config" "$TMP_DIR/dnsmasq.before-stop-failure"
echo never_exit > "$PROCESS_STATE_DIR/dnsmasq"
echo 4246 > "$PROCESS_STATE_DIR/dnsmasq.pids"
printf '%s\n' never_exit > "$SS_PROC_ROOT/4248/stop_mode"
launches_before_stop_failure=$(cat "$PROCESS_STATE_DIR/dnsmasq_runs")
expect_status 1 "dnsmasq stop failure preserves live config" ss_publish_and_restart_dnsmasq "$dnsmasq_config" "$TMP_DIR/bin/dnsmasq" 0
cmp -s "$dnsmasq_config" "$TMP_DIR/dnsmasq.before-stop-failure" || fail "failed stop changed live dnsmasq config"
[ "$DNSMASQ_RESTART_CALLS" -eq 3 ] || fail "stop failure did not restore through a second RC restart attempt"
[ -f "$PROCESS_STATE_DIR/dnsmasq" ] || fail "failed stop removed the existing daemon"
[ "$(cat "$PROCESS_STATE_DIR/dnsmasq_runs")" -eq "$launches_before_stop_failure" ] || fail "replacement launched after stop failure"
rm -f "$PROCESS_STATE_DIR/dnsmasq" "$PROCESS_STATE_DIR/dnsmasq.pids"
rm -rf "$SS_PROC_ROOT/4248"

cp "$dnsmasq_config" "$TMP_DIR/dnsmasq.before-rollback"
DNSMASQ_RESTART_CALLS=0
rm -f "$PROCESS_STATE_DIR/dnsmasq_runs"
printf 'exit_on_term\n' > "$PROCESS_STATE_DIR/dnsmasq"
printf '4246\n' > "$PROCESS_STATE_DIR/dnsmasq.pids"
add_stop_proc_fixture 4246 dnsmasq /user/sbin/dnsmasq /user/sbin/dnsmasq exit_on_term 4000
DNSMASQ_START_MODE=rollback
expect_status 19 "dnsmasq restart failure preserves original status" ss_publish_and_restart_dnsmasq "$dnsmasq_config" "$TMP_DIR/bin/dnsmasq" 0
cmp -s "$dnsmasq_config" "$TMP_DIR/dnsmasq.before-rollback" || fail "dnsmasq restart did not restore original config"
[ "$DNSMASQ_RESTART_CALLS" -eq 2 ] || fail "failed replacement did not perform the RC rollback restart"
[ -f "$PROCESS_STATE_DIR/dnsmasq" ] || fail "dnsmasq rollback did not restart old service"
[ "$(cat "$PROCESS_STATE_DIR/dnsmasq_runs")" -eq 2 ] || fail "dnsmasq rollback launch count was not two"
ss_stop_named_processes dnsmasq
rm -f "$PROCESS_STATE_DIR/dnsmasq" "$PROCESS_STATE_DIR/dnsmasq.pids" "$PROCESS_STATE_DIR/dnsmasq_runs"

# An immediate child exit is not reported as a successful service start.
RUN_BIN_MODE=fail
expect_status 1 "run_bin child exits before readiness" run_bin ssr-redir -c /tmp/test.json
[ ! -f "$PROCESS_STATE_DIR/ssr-redir" ] || fail "failed child was reported as present"

# UNKNOWN inventory fails closed without signalling an unverified process.
: > "$KILL_LOG"
RUN_BIN_MODE=unknown
expect_status 2 "run_bin unknown process inventory" run_bin ssr-redir -c /tmp/test.json
[ ! -s "$KILL_LOG" ] || fail "run_bin signalled after unknown inventory"
rm -f "$PROCESS_STATE_DIR/pidof_unknown" "$PROCESS_STATE_DIR/ssr-redir" "$PROCESS_STATE_DIR/ssr-redir.pids" "$PROCESS_STATE_DIR/ipt2socks" "$PROCESS_STATE_DIR/ipt2socks.pids"

# Watchcat launch must be bounded, tri-state and idempotent.
rm -f "$PROCESS_STATE_DIR/monitor" "$PROCESS_STATE_DIR/monitor.pids"
server_count=1
redir_tcp=1
redir_udp=0
tunnel_enable=0
v2ray_enable=0
local_enable=0
pdnsd_enable_flag=0
chinadnsng_enable_flag=0
WATCHCAT_LOG="$TMP_DIR/watchcat.log"
: > "$WATCHCAT_LOG"
ss_launch_watchcat() {
	printf '%s\n' "$*" >> "$WATCHCAT_LOG"
	case "${WATCHCAT_LAUNCH_MODE:-ready}" in
		fail) return 17 ;;
		absent) return 0 ;;
		ready)
			: > "$PROCESS_STATE_DIR/monitor"
			printf '4245\n' > "$PROCESS_STATE_DIR/monitor.pids"
			;;
	esac
	return 0
}
export WATCHCAT_LAUNCH_MODE=ready
WATCHCAT_ENABLED=1
expect_status 0 "watchcat detached monitor readiness" start_watchcat
[ -f "$PROCESS_STATE_DIR/monitor" ] || fail "watchcat launcher did not publish monitor readiness"
[ "$(wc -l < "$WATCHCAT_LOG")" -eq 1 ] || fail "watchcat was not launched exactly once"
rm -f "$PROCESS_STATE_DIR/monitor" "$PROCESS_STATE_DIR/monitor.pids"

# A monitor process that never appears must not be reported as started.
WATCHCAT_LAUNCH_MODE=absent
expect_status 1 "watchcat readiness timeout" start_watchcat
[ "$(wc -l < "$WATCHCAT_LOG")" -eq 2 ] || fail "watchcat timeout did not invoke launcher"

# A scheduling failure must be preserved.
WATCHCAT_LAUNCH_MODE=fail
expect_status 17 "watchcat scheduling failure" start_watchcat
[ "$(wc -l < "$WATCHCAT_LOG")" -eq 3 ] || fail "watchcat scheduling failure was not recorded"

# UNKNOWN pgrep inventory blocks a duplicate launch.
: > "$PROCESS_STATE_DIR/pgrep_unknown"
WATCHCAT_LAUNCH_MODE=ready
expect_status 2 "watchcat UNKNOWN preflight" start_watchcat
[ "$(wc -l < "$WATCHCAT_LOG")" -eq 3 ] || fail "watchcat launched with UNKNOWN process inventory"
rm -f "$PROCESS_STATE_DIR/pgrep_unknown"

# An already-running monitor is accepted without starting a duplicate.
: > "$PROCESS_STATE_DIR/monitor"
printf '4246\n' > "$PROCESS_STATE_DIR/monitor.pids"
WATCHCAT_LAUNCH_MODE=fail
expect_status 0 "watchcat duplicate prevention" start_watchcat
[ "$(wc -l < "$WATCHCAT_LOG")" -eq 3 ] || fail "watchcat relaunched an existing monitor"
rm -f "$PROCESS_STATE_DIR/monitor" "$PROCESS_STATE_DIR/monitor.pids"

WATCHCAT_ENABLED=0
expect_status 0 "watchcat disabled" start_watchcat
[ "$(wc -l < "$WATCHCAT_LOG")" -eq 3 ] || fail "disabled watchcat was started"
WATCHCAT_ENABLED=1
server_count=0
redir_tcp=0
expect_status 0 "watchcat no active services" start_watchcat
[ "$(wc -l < "$WATCHCAT_LOG")" -eq 3 ] || fail "watchcat started without any active service"

# The required DNS proxy stage must observe the selected daemon after launch.
export DNS_PDNSD_ENABLE=0 DNS_UPSTREAM='8.8.8.8#53' DNS_START_MODE=ready
rm -f "$PROCESS_STATE_DIR/dnsproxy" "$PROCESS_STATE_DIR/dns2tcp" "$PROCESS_STATE_DIR/pidof_unknown"
ss_dns_proxy_config_read || fail "dnsproxy config preflight failed"
start_dns_proxy
[ -f "$PROCESS_STATE_DIR/dnsproxy" ] || fail "dnsproxy launch was not observed"
stop_dns_proxy
[ ! -f "$PROCESS_STATE_DIR/dnsproxy" ] || fail "DNS proxy stop did not remove dnsproxy"

# dns2tcp is an alternate backend and is subject to the same readiness gate.
DNS_PDNSD_ENABLE=1
ss_dns_proxy_config_read || fail "dns2tcp config preflight failed"
start_dns_proxy
[ -f "$PROCESS_STATE_DIR/dns2tcp" ] || fail "dns2tcp launch was not observed"
stop_dns_proxy

# Immediate child exit is not reported as a healthy DNS service.
DNS_PDNSD_ENABLE=0
DNS_START_MODE=fail
ss_dns_proxy_config_read || fail "failed-child config preflight failed"
expect_status 1 "DNS proxy child launch failure" start_dns_proxy
[ ! -f "$PROCESS_STATE_DIR/dnsproxy" ] || fail "failed dnsproxy fixture became visible"

# Unknown inventory is distinct from confirmed absence and must not signal.
: > "$KILL_LOG"
DNS_START_MODE=unknown
ss_dns_proxy_config_read || fail "unknown-inventory config preflight failed"
expect_status 2 "DNS proxy unknown startup inventory" start_dns_proxy
[ ! -s "$KILL_LOG" ] || fail "unknown DNS proxy inventory caused a signal"
rm -f "$PROCESS_STATE_DIR/pidof_unknown" "$PROCESS_STATE_DIR/dnsproxy"

DNS_UPSTREAM=''
DNS_START_MODE=ready
expect_status 1 "empty DNS upstream rejected before launch" ss_dns_proxy_config_read
DNS_UPSTREAM='8.8.8.8#53'
DNS_PDNSD_ENABLE=9
expect_status 1 "unsupported DNS proxy backend rejected before launch" ss_dns_proxy_config_read
echo "Shadowsocks process lifecycle regression: PASS"
