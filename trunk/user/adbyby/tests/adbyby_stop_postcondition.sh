#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
mkdir -p "$tmp/bin"
state_file="$tmp/state"
events_file="$tmp/events"
: > "$events_file"

cat > "$tmp/bin/pidof" <<'STUB'
#!/bin/sh
case "$(cat "$ADBYBY_TEST_STATE")" in
	absent) exit 1 ;;
	error) exit 2 ;;
	malformed) echo "123 nope"; exit 0 ;;
	stubborn) echo "123"; exit 0 ;;
	running) echo "123"; exit 0 ;;
	*) exit 2 ;;
esac
STUB
cat > "$tmp/bin/killall" <<'STUB'
#!/bin/sh
echo killall >> "$ADBYBY_TEST_EVENTS"
if [ "$ADBYBY_TEST_KILL_MODE" = "stop" ]; then
	echo absent > "$ADBYBY_TEST_STATE"
elif [ "$ADBYBY_TEST_KILL_MODE" = "unknown" ]; then
	echo unknown > "$ADBYBY_TEST_STATE"
fi
exit 0
STUB
cat > "$tmp/bin/sleep" <<'STUB'
#!/bin/sh
echo sleep >> "$ADBYBY_TEST_EVENTS"
[ "${ADBYBY_TEST_SLEEP_MODE:-ok}" = "ok" ] || exit 7
exit 0
STUB
cat > "$tmp/bin/logger" <<'STUB'
#!/bin/sh
exit 0
STUB
cat > "$tmp/bin/ps" <<'STUB'
#!/bin/sh
exit 0
STUB
cat > "$tmp/bin/kill" <<'STUB'
#!/bin/sh
exit 0
STUB
chmod +x "$tmp/bin/"*

PATH="$tmp/bin:$PATH"
export PATH
ADBYBY_TEST_STATE="$state_file"
ADBYBY_TEST_EVENTS="$events_file"
ADBYBY_TEST_KILL_MODE=stop
export ADBYBY_TEST_STATE ADBYBY_TEST_EVENTS ADBYBY_TEST_KILL_MODE

# Source production functions; the empty command-line action is intentionally inert.
. "$repo_root/trunk/user/adbyby/adbyby.sh"
mem_mode=0
cleanup_count=0
restart_count=0
del_rule() { echo del_rule >> "$events_file"; cleanup_count=$((cleanup_count + 1)); }
del_cron() { echo del_cron >> "$events_file"; cleanup_count=$((cleanup_count + 1)); }
del_dns() { echo del_dns >> "$events_file"; cleanup_count=$((cleanup_count + 1)); }
adbyby_restart_dhcpd() { echo restart >> "$events_file"; restart_count=$((restart_count + 1)); }
# kill is a shell builtin in ash and the CI shell; prevent signalling the test runner.
kill() { echo kill >> "$events_file"; return 0; }


assert_blocked()
{
	case "$1" in
		error|malformed|stubborn) ;;
		*) echo "invalid blocked fixture: $1" >&2; exit 1 ;;
	esac
	echo "$1" > "$state_file"
	: > "$events_file"
	ADBYBY_TEST_KILL_MODE=keep
	if adbyby_close; then
		echo "adbyby_close unexpectedly succeeded for $1" >&2
		exit 1
	fi
	if grep -E '^(del_rule|del_cron|del_dns|restart)$' "$events_file" >/dev/null; then
		echo "cleanup/restart ran while process state was $1" >&2
		exit 1
	fi
}

assert_blocked error
assert_blocked malformed
assert_blocked stubborn

[ "$(grep -c '^sleep# A confirmed-absent process is idempotently stopped without killall.
echo absent > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=keep
adbyby_close
if grep '^killall$' "$events_file" >/dev/null; then
	echo "killall ran after confirmed ABSENT state" >&2
	exit 1
fi
[ "$(grep -c '^del_' "$events_file")" -eq 3 ]
[ "$(grep -c '^restart$' "$events_file")" -eq 1 ]

# A running daemon is signalled, then cleanup occurs only after absence is seen.
echo running > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=stop
ADBYBY_TEST_SLEEP_MODE=ok
adbyby_close
[ "$(grep -c '^killall$' "$events_file")" -eq 1 ]
[ "$(grep -c '^del_' "$events_file")" -eq 3 ]
[ "$(grep -c '^restart$' "$events_file")" -eq 1 ]

echo "Adbyby stop postcondition regression: PASS"
 "$events_file")" -eq 5 ] ||
	echo "stubborn daemon was not checked through five bounded polls" >&2

# UNKNOWN after TERM and a failed wait primitive both preserve active rules.
echo running > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=unknown
if adbyby_close; then
	echo "adbyby_close succeeded after post-signal process state became UNKNOWN" >&2
	exit 1
fi
[ "$(grep -c '^killall# A confirmed-absent process is idempotently stopped without killall.
echo absent > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=keep
adbyby_close
if grep '^killall$' "$events_file" >/dev/null; then
	echo "killall ran after confirmed ABSENT state" >&2
	exit 1
fi
[ "$(grep -c '^del_' "$events_file")" -eq 3 ]
[ "$(grep -c '^restart$' "$events_file")" -eq 1 ]

# A running daemon is signalled, then cleanup occurs only after absence is seen.
echo running > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=stop
adbyby_close
[ "$(grep -c '^killall$' "$events_file")" -eq 1 ]
[ "$(grep -c '^del_' "$events_file")" -eq 3 ]
[ "$(grep -c '^restart$' "$events_file")" -eq 1 ]

echo "Adbyby stop postcondition regression: PASS"
 "$events_file")" -eq 1 ]
if grep -E '^(del_rule|del_cron|del_dns|restart)# A confirmed-absent process is idempotently stopped without killall.
echo absent > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=keep
adbyby_close
if grep '^killall$' "$events_file" >/dev/null; then
	echo "killall ran after confirmed ABSENT state" >&2
	exit 1
fi
[ "$(grep -c '^del_' "$events_file")" -eq 3 ]
[ "$(grep -c '^restart$' "$events_file")" -eq 1 ]

# A running daemon is signalled, then cleanup occurs only after absence is seen.
echo running > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=stop
adbyby_close
[ "$(grep -c '^killall$' "$events_file")" -eq 1 ]
[ "$(grep -c '^del_' "$events_file")" -eq 3 ]
[ "$(grep -c '^restart$' "$events_file")" -eq 1 ]

echo "Adbyby stop postcondition regression: PASS"
 "$events_file" >/dev/null; then
	echo "cleanup/restart ran after post-signal UNKNOWN state" >&2
	exit 1
fi

echo running > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=keep
ADBYBY_TEST_SLEEP_MODE=fail
if adbyby_close; then
	echo "adbyby_close succeeded after sleep failure" >&2
	exit 1
fi
if grep -E '^(del_rule|del_cron|del_dns|restart)# A confirmed-absent process is idempotently stopped without killall.
echo absent > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=keep
adbyby_close
if grep '^killall$' "$events_file" >/dev/null; then
	echo "killall ran after confirmed ABSENT state" >&2
	exit 1
fi
[ "$(grep -c '^del_' "$events_file")" -eq 3 ]
[ "$(grep -c '^restart$' "$events_file")" -eq 1 ]

# A running daemon is signalled, then cleanup occurs only after absence is seen.
echo running > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=stop
adbyby_close
[ "$(grep -c '^killall$' "$events_file")" -eq 1 ]
[ "$(grep -c '^del_' "$events_file")" -eq 3 ]
[ "$(grep -c '^restart$' "$events_file")" -eq 1 ]

echo "Adbyby stop postcondition regression: PASS"
 "$events_file" >/dev/null; then
	echo "cleanup/restart ran after sleep failure" >&2
	exit 1
fi
ADBYBY_TEST_SLEEP_MODE=ok

# A confirmed-absent process is idempotently stopped without killall.
echo absent > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=keep
adbyby_close
if grep '^killall$' "$events_file" >/dev/null; then
	echo "killall ran after confirmed ABSENT state" >&2
	exit 1
fi
[ "$(grep -c '^del_' "$events_file")" -eq 3 ]
[ "$(grep -c '^restart$' "$events_file")" -eq 1 ]

# A running daemon is signalled, then cleanup occurs only after absence is seen.
echo running > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=stop
adbyby_close
[ "$(grep -c '^killall$' "$events_file")" -eq 1 ]
[ "$(grep -c '^del_' "$events_file")" -eq 3 ]
[ "$(grep -c '^restart$' "$events_file")" -eq 1 ]

echo "Adbyby stop postcondition regression: PASS"
