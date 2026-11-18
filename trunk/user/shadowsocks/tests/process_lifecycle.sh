#!/bin/sh
set -eu

TEST_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/shadowsocks-lifecycle.XXXXXX")
trap 'rm -rf "$TMP_DIR"' EXIT HUP INT TERM
mkdir -p "$TMP_DIR/bin" "$TMP_DIR/state"
export PROCESS_STATE_DIR="$TMP_DIR/state"
export KILL_LOG="$TMP_DIR/kill.log"
export PATH="$TMP_DIR/bin:$PATH"

cat > "$TMP_DIR/bin/pidof" <<'EOF'
#!/bin/sh
[ "$#" -eq 1 ] || exit 2
[ -f "$PROCESS_STATE_DIR/pidof_unknown" ] && exit 2
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

cat > "$TMP_DIR/bin/killall" <<'EOF'
#!/bin/sh
signal=TERM
process_name=
while [ "$#" -gt 0 ]; do
	case "$1" in
		-q) shift ;;
		-KILL|-9) signal=KILL; shift ;;
		-*) exit 2 ;;
		*) process_name=$1; shift ;;
	esac
done
[ -n "$process_name" ] || exit 2
printf 'NAME-%s %s\n' "$signal" "$process_name" >> "$KILL_LOG"
state_file="$PROCESS_STATE_DIR/$process_name"
[ -f "$state_file" ] || exit 1
mode=$(cat "$state_file")
case "$signal:$mode" in
	TERM:exit_on_term|KILL:exit_on_kill)
		rm -f "$state_file"
		;;
	TERM:unknown_after_term)
		: > "$PROCESS_STATE_DIR/pidof_unknown"
		;;
esac
exit 0
EOF

cat > "$TMP_DIR/bin/pgrep" <<'EOF'
#!/bin/sh
[ "$#" -eq 2 ] && [ "$1" = "-f" ] || exit 2
[ -f "$PROCESS_STATE_DIR/pgrep_unknown" ] && exit 2
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

cat > "$TMP_DIR/bin/pkill" <<'EOF'
#!/bin/sh
signal=TERM
[ "$#" -ge 1 ] || exit 2
case "$1" in
	-TERM) signal=TERM; shift ;;
	-KILL|-9) signal=KILL; shift ;;
esac
[ "$#" -eq 2 ] && [ "$1" = "-f" ] && [ "$2" = "/usr/bin/ss-monitor" ] || exit 2
printf 'PATTERN-%s %s\n' "$signal" "$2" >> "$KILL_LOG"
state_file="$PROCESS_STATE_DIR/monitor"
[ -f "$state_file" ] || exit 1
mode=$(cat "$state_file")
case "$signal:$mode" in
	TERM:exit_on_term|KILL:exit_on_kill)
		rm -f "$state_file"
		;;
	TERM:unknown_after_term)
		: > "$PROCESS_STATE_DIR/pgrep_unknown"
		;;
esac
exit 0
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
chmod 755 "$TMP_DIR/bin/ssr-redir" "$TMP_DIR/bin/lua" "$TMP_DIR/bin/chinadns-ng"

chmod 755 "$TMP_DIR/bin/dnsproxy" "$TMP_DIR/bin/dns2tcp"
chmod 755 "$TMP_DIR/bin/pidof" "$TMP_DIR/bin/killall" "$TMP_DIR/bin/pgrep" "$TMP_DIR/bin/pkill" "$TMP_DIR/bin/sleep"
log() { :; }
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
sed -n \
	-e '/^run_bin() {/,/^}/p' \
	-e '/^ss_start_chinadns_daemon() {/,/^}/p' \
	-e '/^stop_dns_proxy() {/,/^}/p' \
	-e '/^start_dns_proxy() {/,/^}/p' \
	-e '/^ss_launch_watchcat() {/,/^}/p' \
	-e '/^start_watchcat() {/,/^}/p' \
	"$TEST_DIR/../scripts/shadowsocks.sh" > "$TMP_DIR/dns-proxy-functions.sh"
. "$TMP_DIR/dns-proxy-functions.sh"

fail() { echo "FAIL: $*" >&2; exit 1; }
expect_status() {
	expected=$1
	label=$2
	shift 2
	if "$@"; then actual=0; else actual=$?; fi
	[ "$actual" -eq "$expected" ] || fail "$label status=$actual expected=$expected"
}

# pidof/pgrep status 1 means confirmed ABSENT, and must normalize to a zero count.
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
: > "$KILL_LOG"
: > "$PROCESS_STATE_DIR/pidof_unknown"
printf 'never_exit\n' > "$PROCESS_STATE_DIR/unknown"
expect_status 2 "pidof error blocks named stop" ss_stop_named_processes unknown
[ ! -s "$KILL_LOG" ] || fail "pidof error caused a signal"
rm -f "$PROCESS_STATE_DIR/pidof_unknown" "$PROCESS_STATE_DIR/unknown"

# If inventory becomes unknown after TERM, do not escalate to KILL.
: > "$KILL_LOG"
printf 'unknown_after_term\n' > "$PROCESS_STATE_DIR/term_unknown"
expect_status 2 "unknown after TERM blocks KILL" ss_stop_named_processes term_unknown
if grep -q '^NAME-KILL term_unknown$' "$KILL_LOG"; then
	fail "named KILL sent after inventory became unknown"
fi
[ -e "$PROCESS_STATE_DIR/pidof_unknown" ] || fail "TERM fixture did not inject the unknown state"
rm -f "$PROCESS_STATE_DIR/pidof_unknown" "$PROCESS_STATE_DIR/term_unknown"

# Preserve the existing graceful / forced / timeout behavior.
: > "$KILL_LOG"
printf 'exit_on_term\n' > "$PROCESS_STATE_DIR/term_exit"
ss_stop_named_processes term_exit
[ ! -e "$PROCESS_STATE_DIR/term_exit" ] || fail "TERM process was not stopped"
grep -q '^NAME-TERM term_exit$' "$KILL_LOG"
if grep -q '^NAME-KILL term_exit$' "$KILL_LOG"; then fail "named stop escalated after TERM exit"; fi

: > "$KILL_LOG"
printf 'exit_on_kill\n' > "$PROCESS_STATE_DIR/force_exit"
ss_stop_named_processes force_exit
[ ! -e "$PROCESS_STATE_DIR/force_exit" ] || fail "KILL escalation did not stop process"
grep -q '^NAME-TERM force_exit$' "$KILL_LOG"
grep -q '^NAME-KILL force_exit$' "$KILL_LOG"

: > "$KILL_LOG"
printf 'never_exit\n' > "$PROCESS_STATE_DIR/stuck"
expect_status 1 "stuck named process remains a stop failure" ss_stop_named_processes stuck
[ -e "$PROCESS_STATE_DIR/stuck" ] || fail "stuck named process fixture disappeared"
grep -q '^NAME-TERM stuck$' "$KILL_LOG"
grep -q '^NAME-KILL stuck$' "$KILL_LOG"

# pgrep has the same strict status/output contract as pidof.
: > "$PROCESS_STATE_DIR/pgrep_empty_success"
expect_status 2 "pgrep empty success is unknown" ss_process_pattern_state '/usr/bin/ss-monitor'
rm -f "$PROCESS_STATE_DIR/pgrep_empty_success"
: > "$PROCESS_STATE_DIR/pgrep_malformed"
expect_status 2 "pgrep malformed PID output is unknown" ss_process_pattern_state '/usr/bin/ss-monitor'
rm -f "$PROCESS_STATE_DIR/pgrep_malformed"
: > "$PROCESS_STATE_DIR/pgrep_inconsistent"
expect_status 2 "pgrep failure with output is unknown" ss_process_pattern_state '/usr/bin/ss-monitor'
rm -f "$PROCESS_STATE_DIR/pgrep_inconsistent"

: > "$KILL_LOG"
: > "$PROCESS_STATE_DIR/pgrep_unknown"
printf 'never_exit\n' > "$PROCESS_STATE_DIR/monitor"
expect_status 2 "pgrep error blocks monitor stop" ss_stop_process_pattern '/usr/bin/ss-monitor'
[ ! -s "$KILL_LOG" ] || fail "pgrep error caused a monitor signal"
rm -f "$PROCESS_STATE_DIR/pgrep_unknown" "$PROCESS_STATE_DIR/monitor"

: > "$KILL_LOG"
printf 'unknown_after_term\n' > "$PROCESS_STATE_DIR/monitor"
expect_status 2 "unknown after monitor TERM blocks KILL" ss_stop_process_pattern '/usr/bin/ss-monitor'
if grep -q '^PATTERN-KILL /usr/bin/ss-monitor$' "$KILL_LOG"; then
	fail "monitor KILL sent after inventory became unknown"
fi
[ -e "$PROCESS_STATE_DIR/pgrep_unknown" ] || fail "monitor TERM fixture did not inject unknown state"
rm -f "$PROCESS_STATE_DIR/pgrep_unknown" "$PROCESS_STATE_DIR/monitor"

: > "$KILL_LOG"
printf 'exit_on_term\n' > "$PROCESS_STATE_DIR/monitor"
ss_stop_process_pattern '/usr/bin/ss-monitor'
[ ! -e "$PROCESS_STATE_DIR/monitor" ] || fail "monitor was not stopped"
grep -q '^PATTERN-TERM /usr/bin/ss-monitor$' "$KILL_LOG"
if grep -q '^PATTERN-KILL /usr/bin/ss-monitor$' "$KILL_LOG"; then fail "monitor escalated after TERM exit"; fi

: > "$KILL_LOG"
printf 'exit_on_kill\n' > "$PROCESS_STATE_DIR/monitor"
ss_stop_process_pattern '/usr/bin/ss-monitor'
[ ! -e "$PROCESS_STATE_DIR/monitor" ] || fail "monitor KILL escalation failed"
grep -q '^PATTERN-TERM /usr/bin/ss-monitor$' "$KILL_LOG"
grep -q '^PATTERN-KILL /usr/bin/ss-monitor$' "$KILL_LOG"

: > "$KILL_LOG"
printf 'never_exit\n' > "$PROCESS_STATE_DIR/monitor"
expect_status 1 "stuck monitor remains a stop failure" ss_stop_process_pattern '/usr/bin/ss-monitor'
[ -e "$PROCESS_STATE_DIR/monitor" ] || fail "stuck monitor fixture disappeared"
grep -q '^PATTERN-TERM /usr/bin/ss-monitor$' "$KILL_LOG"
grep -q '^PATTERN-KILL /usr/bin/ss-monitor$' "$KILL_LOG"

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
start_dns_proxy
[ -f "$PROCESS_STATE_DIR/dnsproxy" ] || fail "dnsproxy launch was not observed"
stop_dns_proxy
[ ! -f "$PROCESS_STATE_DIR/dnsproxy" ] || fail "DNS proxy stop did not remove dnsproxy"

# dns2tcp is an alternate backend and is subject to the same readiness gate.
DNS_PDNSD_ENABLE=1
start_dns_proxy
[ -f "$PROCESS_STATE_DIR/dns2tcp" ] || fail "dns2tcp launch was not observed"
stop_dns_proxy

# Immediate child exit is not reported as a healthy DNS service.
DNS_PDNSD_ENABLE=0
DNS_START_MODE=fail
expect_status 1 "DNS proxy child launch failure" start_dns_proxy
[ ! -f "$PROCESS_STATE_DIR/dnsproxy" ] || fail "failed dnsproxy fixture became visible"

# Unknown inventory is distinct from confirmed absence and must not signal.
: > "$KILL_LOG"
DNS_START_MODE=unknown
expect_status 2 "DNS proxy unknown startup inventory" start_dns_proxy
[ ! -s "$KILL_LOG" ] || fail "unknown DNS proxy inventory caused a signal"
rm -f "$PROCESS_STATE_DIR/pidof_unknown" "$PROCESS_STATE_DIR/dnsproxy"

DNS_UPSTREAM=''
DNS_START_MODE=ready
expect_status 1 "empty DNS upstream rejected" start_dns_proxy
DNS_UPSTREAM='8.8.8.8#53'
DNS_PDNSD_ENABLE=9
expect_status 1 "unsupported DNS proxy backend rejected" start_dns_proxy
echo "Shadowsocks process lifecycle regression: PASS"
