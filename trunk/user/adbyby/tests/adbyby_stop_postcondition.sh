#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
tmp=$(mktemp -d "${TMPDIR:-/tmp}/adbyby-stop.XXXXXX")
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

mkdir -p "$tmp/bin"
state_file="$tmp/state"
events_file="$tmp/events"
: > "$events_file"

cat > "$tmp/bin/pidof" <<'STUB'
#!/bin/sh
case "$(cat "$ADBYBY_TEST_STATE")" in
	absent) exit 1 ;;
	error|unknown) exit 2 ;;
	malformed) echo "123 nope"; exit 0 ;;
	running|stubborn) echo "${ADBYBY_TEST_PID_OUTPUT:-123}"; exit 0 ;;
	*) exit 2 ;;
esac
STUB
cat > "$tmp/bin/killall" <<'STUB'
#!/bin/sh
printf 'killall:%s\n' "$*" >> "$ADBYBY_TEST_EVENTS"
exit 0
STUB
kill()
{
	printf 'kill:%s\n' "$*" >> "$ADBYBY_TEST_EVENTS"
	case "$ADBYBY_TEST_KILL_MODE" in
		stop|stop-fail) echo absent > "$ADBYBY_TEST_STATE" ;;
		unknown) echo unknown > "$ADBYBY_TEST_STATE" ;;
	esac
	[ "${ADBYBY_TEST_KILL_STATUS:-0}" -eq 0 ] && return 0
	return "$ADBYBY_TEST_KILL_STATUS"
}

cat > "$tmp/bin/sleep" <<'STUB'
#!/bin/sh
printf 'sleep:%s\n' "$*" >> "$ADBYBY_TEST_EVENTS"
[ "${ADBYBY_TEST_SLEEP_MODE:-ok}" = ok ]
STUB
cat > "$tmp/bin/ps" <<'STUB'
#!/bin/sh
exit 0
STUB
chmod 755 "$tmp/bin/"*
PATH="$tmp/bin:$PATH"
ADBYBY_TEST_STATE="$state_file"
ADBYBY_TEST_EVENTS="$events_file"
ADBYBY_TEST_KILL_MODE=keep
ADBYBY_TEST_SLEEP_MODE=ok
export PATH ADBYBY_TEST_STATE ADBYBY_TEST_EVENTS
export ADBYBY_TEST_KILL_MODE ADBYBY_TEST_SLEEP_MODE

. "$repo_root/trunk/user/adbyby/adbyby.sh"

logger() { :; }
mem_mode=0
del_rule() { echo del_rule >> "$events_file"; }
del_cron() { echo del_cron >> "$events_file"; }
del_dns() { echo del_dns >> "$events_file"; }
adbyby_restart_dhcpd() { echo restart_dhcpd >> "$events_file"; }
addscripts() { echo addscripts >> "$events_file"; }
add_rules() { echo add_rules >> "$events_file"; return 0; }
add_dns() { echo add_dns >> "$events_file"; }
add_rule() { echo add_rule >> "$events_file"; }
hosts_ads() { echo hosts_ads >> "$events_file"; }
PROG_PATH="$tmp/adbyby"
mkdir -p "$PROG_PATH"
printf '%s\n' '#!/bin/sh' 'echo launch >> "$ADBYBY_TEST_EVENTS"' > "$PROG_PATH/adbyby"
chmod 755 "$PROG_PATH/adbyby"
cat > "$tmp/bin/iptables-save" <<'STUB'
#!/bin/sh
echo ADBYBY
STUB
chmod 755 "$tmp/bin/iptables-save"
# The shell builtin takes precedence over PATH's kill stub.
kill() { echo mem_helper_kill >> "$events_file"; return 0; }

event_count()
{
	awk -v wanted="$1" '$0 == wanted { count++ } END { print count + 0 }' "$events_file"
}
fail()
{
	echo "Adbyby stop postcondition regression: FAIL: $*" >&2
	exit 1
}
assert_no_cleanup()
{
	if grep -E '^(del_rule|del_cron|del_dns|restart_dhcpd)$' "$events_file" >/dev/null; then
		fail "$1: cleanup/restart ran before confirmed daemon absence"
	fi
}

# Initial inventory errors and malformed PID output cannot authorize a signal.
for blocked_state in error malformed; do
	echo "$blocked_state" > "$state_file"
	: > "$events_file"
	ADBYBY_TEST_KILL_MODE=keep
	if adbyby_close; then fail "initial $blocked_state inventory was accepted"; fi
	[ "$(event_count 'killall:-q adbyby')" -eq 0 ] || fail "signal sent for $blocked_state"
	assert_no_cleanup "$blocked_state"
done

# A valid but stubborn daemon times out after five bounded polls.
echo stubborn > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=keep
if adbyby_close; then fail "stubborn daemon was accepted as stopped"; fi
[ "$(event_count 'kill:-TERM 123')" -eq 1 ] || fail "TERM count was not one"
[ "$(event_count 'sleep:1')" -eq 5 ] || fail "stop did not make five bounded polls"
assert_no_cleanup stubborn

# UNKNOWN after TERM blocks destructive cleanup.
echo running > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=unknown
if adbyby_close; then fail "post-signal UNKNOWN was accepted"; fi
[ "$(event_count 'kill:-TERM 123')" -eq 1 ] || fail "unexpected signal count after UNKNOWN"
[ "$(event_count 'sleep:1')" -eq 1 ] || fail "UNKNOWN did not stop at the first poll"
assert_no_cleanup post-signal-UNKNOWN

# A failed sleep/wait primitive is not evidence of process exit.
echo running > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=keep
ADBYBY_TEST_SLEEP_MODE=fail
if adbyby_close; then fail "sleep failure was accepted as a stop"; fi
[ "$(event_count 'kill:-TERM 123')" -eq 1 ] || fail "sleep-failure path did not send TERM"
[ "$(event_count 'sleep:1')" -eq 1 ] || fail "sleep failure was not observed once"
assert_no_cleanup sleep-failure
ADBYBY_TEST_SLEEP_MODE=ok

# Multiple validated PIDs are signalled individually; killall stays unused.
echo running > "$state_file"
ADBYBY_TEST_PID_OUTPUT='123 456'
: > "$events_file"
ADBYBY_TEST_KILL_MODE=stop
adbyby_close || fail "multi-PID stop failed"
[ "$(event_count 'kill:-TERM 123')" -eq 1 ] || fail "first validated PID was not signalled"
[ "$(event_count 'kill:-TERM 456')" -eq 1 ] || fail "second validated PID was not signalled"
[ "$(event_count 'killall:-q adbyby')" -eq 0 ] || fail "stop rescanned process name with killall"
ADBYBY_TEST_PID_OUTPUT=
export ADBYBY_TEST_PID_OUTPUT

# Confirmed ABSENT is idempotent and permits cleanup without signalling.
echo absent > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=keep
adbyby_close || fail "confirmed-absent daemon did not stop idempotently"
[ "$(event_count 'killall:-q adbyby')" -eq 0 ] || fail "absent daemon was signalled"
[ "$(event_count del_rule)" -eq 1 ] || fail "firewall cleanup count was not one"
[ "$(event_count del_cron)" -eq 1 ] || fail "cron cleanup count was not one"
[ "$(event_count del_dns)" -eq 1 ] || fail "DNS cleanup count was not one"
[ "$(event_count mem_helper_kill)" -eq 1 ] || fail "mem helper cleanup was not reached"
[ "$(event_count restart_dhcpd)" -eq 1 ] || fail "dnsmasq restart count was not one"

# RUNNING followed by a confirmed ABSENT postcondition may safely clean up.
echo running > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=stop
adbyby_close || fail "RUNNING-to-ABSENT stop failed"
[ "$(event_count 'kill:-TERM 123')" -eq 1 ] || fail "successful stop did not send one TERM"
[ "$(event_count 'sleep:1')" -eq 1 ] || fail "successful stop did not verify its postcondition"
[ "$(event_count del_rule)" -eq 1 ] || fail "successful firewall cleanup count was not one"
[ "$(event_count del_cron)" -eq 1 ] || fail "successful cron cleanup count was not one"
[ "$(event_count del_dns)" -eq 1 ] || fail "successful DNS cleanup count was not one"
[ "$(event_count restart_dhcpd)" -eq 1 ] || fail "successful dnsmasq restart count was not one"

# A nonzero killall result is not decisive when the fresh inventory proves ABSENT.
echo running > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=stop-fail
ADBYBY_TEST_KILL_STATUS=29
export ADBYBY_TEST_KILL_STATUS
adbyby_close || fail "failed killall status overrode confirmed ABSENT postcondition"
[ "$(event_count 'kill:-TERM 123')" -eq 1 ] || fail "failed-signal path did not send one TERM"
[ "$(event_count del_rule)" -eq 1 ] || fail "failed-signal path did not complete firewall cleanup"
[ "$(event_count restart_dhcpd)" -eq 1 ] || fail "failed-signal path did not restart dnsmasq"

# updateadb must stop before staging rules or launching a replacement when
# the current daemon remains alive after the bounded stop window.
echo stubborn > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=keep
if adbyby_uprules; then fail "update succeeded while the old daemon remained alive"; fi
if grep -E '^(addscripts|add_rules|launch|add_dns|add_rule|hosts_ads|restart_dhcpd)$' "$events_file" >/dev/null; then
	fail "rule update continued after Adbyby stop failure"
fi

echo "Adbyby stop postcondition and fail-closed cleanup regression: PASS"
