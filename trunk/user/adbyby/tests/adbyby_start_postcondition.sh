#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
tmp=$(mktemp -d "${TMPDIR:-/tmp}/adbyby-start.XXXXXX")
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

mkdir -p "$tmp/bin" "$tmp/adbyby"
state_file="$tmp/state"
poll_file="$tmp/polls"
events_file="$tmp/events"
: > "$events_file"
: > "$poll_file"

cat > "$tmp/bin/pidof" <<'STUB'
#!/bin/sh
case "$ADBYBY_TEST_PIDOF_MODE" in
	running)
		echo 123
		exit 0
		;;
	delayed)
		polls=$(cat "$ADBYBY_TEST_POLL_FILE")
		polls=$((polls + 1))
		echo "$polls" > "$ADBYBY_TEST_POLL_FILE"
		if [ "$polls" -le "$ADBYBY_TEST_ABSENT_POLLS" ]; then
			exit 1
		fi
		echo 123
		exit 0
		;;
	state)
		if [ -f "$ADBYBY_TEST_STATE" ] &&
		   [ "$(cat "$ADBYBY_TEST_STATE")" = running ]; then
			echo 123
			exit 0
		fi
		exit 1
		;;
	absent)
		polls=$(cat "$ADBYBY_TEST_POLL_FILE")
		polls=$((polls + 1))
		echo "$polls" > "$ADBYBY_TEST_POLL_FILE"
		exit 1
		;;
	unknown)
		exit 2
		;;
	malformed)
		echo "123 nope"
		exit 0
		;;
esac
exit 2
STUB

cat > "$tmp/bin/sleep" <<'STUB'
#!/bin/sh
printf 'sleep:%s\n' "$*" >> "$ADBYBY_TEST_EVENTS"
[ "${ADBYBY_TEST_SLEEP_MODE:-ok}" = ok ]
STUB

cat > "$tmp/bin/iptables-save" <<'STUB'
#!/bin/sh
exit 0
STUB
chmod 755 "$tmp/bin/"*

PATH="$tmp/bin:$PATH"
ADBYBY_TEST_STATE="$state_file"
ADBYBY_TEST_POLL_FILE="$poll_file"
ADBYBY_TEST_EVENTS="$events_file"
ADBYBY_TEST_PIDOF_MODE=running
ADBYBY_TEST_ABSENT_POLLS=0
ADBYBY_TEST_SLEEP_MODE=ok
ADBYBY_TEST_DAEMON_MODE=running
export PATH ADBYBY_TEST_STATE ADBYBY_TEST_POLL_FILE ADBYBY_TEST_EVENTS
export ADBYBY_TEST_PIDOF_MODE ADBYBY_TEST_ABSENT_POLLS
export ADBYBY_TEST_SLEEP_MODE ADBYBY_TEST_DAEMON_MODE

. "$repo_root/trunk/user/adbyby/adbyby.sh"

logger() { printf 'logger:%s\n' "$*" >> "$events_file"; }
addscripts() { echo addscripts >> "$events_file"; }
add_rules() { echo add_rules >> "$events_file"; return 0; }
add_dns() { echo add_dns >> "$events_file"; return "${ADBYBY_TEST_ADD_DNS_STATUS:-0}"; }
add_rule() { echo add_rule >> "$events_file"; return "${ADBYBY_TEST_ADD_RULE_STATUS:-0}"; }
hosts_ads() { echo hosts_ads >> "$events_file"; return "${ADBYBY_TEST_HOSTS_STATUS:-0}"; }
adbyby_restart_dhcpd() { echo restart_dhcpd >> "$events_file"; return "${ADBYBY_TEST_RESTART_STATUS:-0}"; }
add_cron() { echo add_cron >> "$events_file"; return "${ADBYBY_TEST_CRON_STATUS:-0}"; }
adbyby_close() { echo adbyby_close >> "$events_file"; return "${ADBYBY_TEST_ROLLBACK_STATUS:-0}"; }
nvram() { return 0; }
adbyby_refresh_nvram_state() { return 0; }

PROG_PATH="$tmp/adbyby"
mkdir -p "$PROG_PATH"
cat > "$PROG_PATH/adbyby" <<'DAEMON'
#!/bin/sh
printf 'daemon-launch\n' >> "$ADBYBY_TEST_EVENTS"
if [ "$ADBYBY_TEST_DAEMON_MODE" = running ]; then
	printf 'running\n' > "$ADBYBY_TEST_STATE"
fi
exit 0
DAEMON
chmod 755 "$PROG_PATH/adbyby"
export PROG_PATH

# The production process inventory verifies executable identity and stat
# start_time, so this launcher fixture supplies a minimal synthetic procfs.
ADBYBY_PROC_ROOT="$tmp/proc"
mkdir -p "$ADBYBY_PROC_ROOT/1" "$ADBYBY_PROC_ROOT/123"
printf '1 (init) S 0 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 100\n' > "$ADBYBY_PROC_ROOT/1/stat"
ln -s "$PROG_PATH/adbyby" "$ADBYBY_PROC_ROOT/123/exe"
printf '123 (adbyby) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 1000\n' > "$ADBYBY_PROC_ROOT/123/stat"
export ADBYBY_PROC_ROOT

event_count()
{
	awk -v wanted="$1" '$0 == wanted { count++ } END { print count + 0 }' "$events_file"
}
reset_events()
{
	: > "$events_file"
	echo 0 > "$poll_file"
	rm -f "$state_file"
}
fail()
{
	echo "Adbyby start readiness regression: FAIL: $*" >&2
	exit 1
}
assert_no_publish()
{
	if grep -E '^(add_dns|add_rule|hosts_ads|restart_dhcpd|add_cron)$' "$events_file" >/dev/null; then
		fail "$1: DNS/firewall publication continued without confirmed RUNNING"
	fi
}

# The production launcher is detached. Reap the short-lived test double after
# each entrypoint returns so it cannot race reset_events() or the EXIT cleanup.
run_and_reap_detached_test_child()
{
	if "$@"; then
		adbyby_test_command_status=0
	else
		adbyby_test_command_status=$?
	fi

	adbyby_test_child_pid=$!
	case "$adbyby_test_child_pid" in
		''|*[!0-9]*) ;;
		*) wait "$adbyby_test_child_pid" 2>/dev/null || : ;;
	esac
	return "$adbyby_test_command_status"
}

# An already visible process satisfies the postcondition without sleeping.
ADBYBY_TEST_PIDOF_MODE=running
reset_events
adbyby_wait_started || fail "RUNNING was not accepted"
[ "$(event_count 'sleep:1')" -eq 0 ] || fail "RUNNING caused an unnecessary wait"

# Confirmed ABSENT may transition to RUNNING within the bounded wait.
ADBYBY_TEST_PIDOF_MODE=delayed
ADBYBY_TEST_ABSENT_POLLS=2
reset_events
adbyby_wait_started || fail "delayed RUNNING transition was rejected"
[ "$(event_count 'sleep:1')" -eq 2 ] || fail "delayed start did not use two bounded waits"

# UNKNOWN and malformed pidof output fail immediately, without another poll.
for mode in unknown malformed; do
	ADBYBY_TEST_PIDOF_MODE="$mode"
	reset_events
	if adbyby_wait_started; then fail "$mode inventory was accepted"; fi
	[ "$(event_count 'sleep:1')" -eq 0 ] || fail "$mode inventory was retried"
done

# Persistent ABSENT times out after five probes and four sleeps.
ADBYBY_TEST_PIDOF_MODE=absent
reset_events
if adbyby_wait_started; then fail "persistent ABSENT was accepted"; fi
[ "$(cat "$poll_file")" -eq 5 ] || fail "ABSENT probe count was not five"
[ "$(event_count 'sleep:1')" -eq 4 ] || fail "timeout wait count was not four"

# A failed wait primitive is not evidence that the daemon is running.
ADBYBY_TEST_PIDOF_MODE=absent
ADBYBY_TEST_SLEEP_MODE=fail
reset_events
if adbyby_wait_started; then fail "sleep failure was accepted"; fi
[ "$(event_count 'sleep:1')" -eq 1 ] || fail "sleep failure was not observed once"
ADBYBY_TEST_SLEEP_MODE=ok

# A detached launcher must be confirmed RUNNING before the caller continues.
ADBYBY_TEST_PIDOF_MODE=state
ADBYBY_TEST_DAEMON_MODE=running
reset_events
run_and_reap_detached_test_child adbyby_start || fail "successful daemon postcondition did not allow startup"
[ "$(event_count daemon-launch)" -eq 1 ] || fail "daemon was not launched once"
[ "$(event_count add_dns)" -eq 1 ] || fail "DNS setup did not follow confirmed startup"
[ "$(event_count add_rule)" -eq 1 ] || fail "redirect rule setup did not follow startup"
launch_line=$(grep -n '^daemon-launch$' "$events_file" | head -n 1 | cut -d: -f1)
dns_line=$(grep -n '^add_dns$' "$events_file" | head -n 1 | cut -d: -f1)
[ "$launch_line" -lt "$dns_line" ] || fail "DNS was published before the daemon launch"

# Every post-launch setup failure must be surfaced and trigger rollback.
for failed_stage in add_dns add_rule hosts_ads adbyby_restart_dhcpd add_cron; do
	ADBYBY_TEST_ADD_DNS_STATUS=0
	ADBYBY_TEST_ADD_RULE_STATUS=0
	ADBYBY_TEST_HOSTS_STATUS=0
	ADBYBY_TEST_RESTART_STATUS=0
	ADBYBY_TEST_CRON_STATUS=0
	ADBYBY_TEST_ROLLBACK_STATUS=0
	case "$failed_stage" in
		add_dns) ADBYBY_TEST_ADD_DNS_STATUS=31 ;;
		add_rule) ADBYBY_TEST_ADD_RULE_STATUS=32 ;;
		hosts_ads) ADBYBY_TEST_HOSTS_STATUS=33 ;;
		adbyby_restart_dhcpd) ADBYBY_TEST_RESTART_STATUS=34 ;;
		add_cron) ADBYBY_TEST_CRON_STATUS=35; ADBYBY_TEST_ROLLBACK_STATUS=46 ;;
	esac
	ADBYBY_TEST_PIDOF_MODE=state
	ADBYBY_TEST_DAEMON_MODE=running
	reset_events
	if run_and_reap_detached_test_child adbyby_start; then
		fail "$failed_stage failure was reported as successful startup"
	else
		publish_status=$?
	fi
	case "$failed_stage" in
		add_dns) expected_status=31 ;;
		add_rule) expected_status=32 ;;
		hosts_ads) expected_status=33 ;;
		adbyby_restart_dhcpd) expected_status=34 ;;
		add_cron) expected_status=35 ;;
	esac
	[ "$publish_status" -eq "$expected_status" ] || fail "$failed_stage status was not preserved"
	[ "$(event_count adbyby_close)" -eq 1 ] || fail "$failed_stage did not request rollback"
	case "$failed_stage" in
		add_dns) [ "$(event_count add_rule)" -eq 0 ] || fail "firewall setup ran after DNS failure" ;;
		add_rule) [ "$(event_count hosts_ads)" -eq 0 ] || fail "hosts setup ran after firewall failure" ;;
		hosts_ads) [ "$(event_count restart_dhcpd)" -eq 0 ] || fail "dnsmasq restarted after hosts failure" ;;
		adbyby_restart_dhcpd) [ "$(event_count add_cron)" -eq 0 ] || fail "cron changed after restart failure" ;;
	esac
done

# Rule-update launch shares the checked publisher but intentionally skips cron.
ADBYBY_TEST_ADD_DNS_STATUS=43
ADBYBY_TEST_ADD_RULE_STATUS=0
ADBYBY_TEST_HOSTS_STATUS=0
ADBYBY_TEST_RESTART_STATUS=0
ADBYBY_TEST_CRON_STATUS=0
ADBYBY_TEST_ROLLBACK_STATUS=0
ADBYBY_TEST_PIDOF_MODE=state
ADBYBY_TEST_DAEMON_MODE=running
reset_events
if run_and_reap_detached_test_child adbyby_uprules; then
	fail "rule update hid DNS publication failure"
else
	update_status=$?
fi
[ "$update_status" -eq 43 ] || fail "rule update did not preserve DNS publication status"
[ "$(event_count adbyby_close)" -eq 2 ] || fail "rule update did not roll back its partial new service"
[ "$(event_count add_cron)" -eq 0 ] || fail "rule update unexpectedly modified the cron schedule"
ADBYBY_TEST_ADD_DNS_STATUS=0

# UNKNOWN startup must stop adbyby_start before DNS, firewall or cron changes.
ADBYBY_TEST_PIDOF_MODE=unknown
ADBYBY_TEST_DAEMON_MODE=running
reset_events
if run_and_reap_detached_test_child adbyby_start; then fail "Adbyby start accepted UNKNOWN process state"; fi
assert_no_publish "start UNKNOWN"

# A child that never appears must not publish a redirect or DNS configuration.
ADBYBY_TEST_PIDOF_MODE=state
ADBYBY_TEST_DAEMON_MODE=absent
reset_events
if run_and_reap_detached_test_child adbyby_start; then fail "Adbyby start accepted a missing child"; fi
assert_no_publish "start timeout"
[ "$(event_count 'sleep:1')" -eq 4 ] || fail "start timeout did not wait four times"

# Updating rules uses the same startup postcondition and fails before reconfiguring DNS.
ADBYBY_TEST_PIDOF_MODE=unknown
ADBYBY_TEST_DAEMON_MODE=running
reset_events
if run_and_reap_detached_test_child adbyby_uprules; then fail "rule update accepted UNKNOWN daemon state"; fi
assert_no_publish "rule update UNKNOWN"

# The lifecycle restart wrapper must preserve the daemon launch failure.
ADBYBY_TEST_PIDOF_MODE=running
adbyby_enable=1
adbyby_close() { return 0; }
adbyby_start() { return 37; }
if adbyby_lifecycle_locked_action restart; then
	fail "restart converted a failed Adbyby start to success"
else
	restart_status=$?
fi
[ "$restart_status" -eq 37 ] || fail "restart did not preserve start status 37"

echo "Adbyby detached-start readiness and restart propagation regression: PASS"
