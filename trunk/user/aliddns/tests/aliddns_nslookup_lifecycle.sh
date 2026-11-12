#!/bin/sh
set -eu

TEST_DIR="$(mktemp -d)"
trap 'rm -rf "$TEST_DIR"' EXIT HUP INT TERM
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
SOURCE="$SCRIPT_DIR/../aliddns.sh"
FUNCTIONS="$TEST_DIR/functions.sh"
MOCK_BIN="$TEST_DIR/bin"
mkdir -p "$MOCK_BIN"

# Load only the production address validators and DNS lookup functions.
sed -n '/^aliddns_ipv4_valid() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^aliddns_ipv6_valid() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^aliddns_nslookup() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^arNslookup() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^arNslookup6() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
. "$FUNCTIONS"

export NSLOOKUP_PID_LOG="$TEST_DIR/nslookup-pids.log"
: > "$NSLOOKUP_PID_LOG"

cat > "$MOCK_BIN/nslookup" <<'MOCK_NSLOOKUP'
#!/bin/sh
case "$1" in
	good4.example)
		printf '%s\n' 'Server: 192.0.2.53' 'Address 1: 192.0.2.53' '' \
			'Name: good4.example' 'Address 1: 203.0.113.7'
		;;
	invalid4.example)
		printf '%s\n' 'Server: 192.0.2.53' 'Address 1: 192.0.2.53' '' \
			'Name: invalid4.example' 'Address 1: 999.0.0.1' 'Address 2: 198.51.100.8'
		;;
	good6.example)
		printf '%s\n' 'Server: 192.0.2.53' 'Address 1: 192.0.2.53' '' \
			'Name: good6.example' 'Address 1: 2001:db8::7'
		;;
	invalid6.example)
		printf '%s\n' 'Server: 192.0.2.53' 'Address 1: 192.0.2.53' '' \
			'Name: invalid6.example' 'Address 1: 2001::1::2' 'Address 2: 2001:db8::8'
		;;
	empty.example)
		printf '%s\n' 'Server: 192.0.2.53' 'Address 1: 192.0.2.53' '' '*** no answer'
		;;
	failed.example)
		printf '%s\n' 'Server: 192.0.2.53' 'Address 1: 192.0.2.53' '' \
			'Name: failed.example' 'Address 1: 203.0.113.99'
		exit 7
		;;
	timeout.example)
		printf '%s %s\n' "$1" "$$" >> "$NSLOOKUP_PID_LOG"
		printf '%s\n' 'Server: 192.0.2.53' 'Address 1: 192.0.2.53' '' \
			'Name: timeout.example' 'Address 1: 203.0.113.66'
		exec /bin/sleep 30
		;;
	unrelated.example)
		printf '%s %s\n' "$1" "$$" >> "$NSLOOKUP_PID_LOG"
		exec /bin/sleep 30
		;;
	concurrent-a.example)
		/bin/sleep 0.2
		printf '%s\n' 'Server: 192.0.2.53' 'Address 1: 192.0.2.53' '' \
			'Name: concurrent-a.example' 'Address 1: 203.0.113.11'
		;;
	concurrent-b.example)
		/bin/sleep 0.2
		printf '%s\n' 'Server: 192.0.2.53' 'Address 1: 192.0.2.53' '' \
			'Name: concurrent-b.example' 'Address 1: 203.0.113.22'
		;;
	*)
		exit 91
		;;
esac
MOCK_NSLOOKUP
cat > "$MOCK_BIN/sleep" <<'MOCK_SLEEP'
#!/bin/sh
if [ "$FAST_WAIT" = "1" ]; then
	exec /bin/sleep 0.02
fi
exec /bin/sleep "$@"
MOCK_SLEEP
chmod 700 "$MOCK_BIN/nslookup" "$MOCK_BIN/sleep"
PATH="$MOCK_BIN:$PATH"
FAST_WAIT=0
export PATH FAST_WAIT

fail() {
	echo "AliDDNS bounded DNS lookup regression: FAIL: $1" >&2
	exit 1
}

[ "$(arNslookup good4.example)" = "203.0.113.7" ] ||
	fail "valid IPv4 answer extraction failed"
[ "$(arNslookup invalid4.example)" = "198.51.100.8" ] ||
	fail "invalid IPv4 candidate was not skipped"
[ "$(arNslookup6 good6.example)" = "2001:db8::7" ] ||
	fail "valid IPv6 answer extraction failed"
[ "$(arNslookup6 invalid6.example)" = "2001:db8::8" ] ||
	fail "invalid IPv6 candidate was not skipped"

# Nonzero resolver status must never publish a partial DNS answer.
for lookup_name in empty.example failed.example; do
	lookup_output=""
	if lookup_output="$(arNslookup "$lookup_name" 2>/dev/null)"; then
		fail "$lookup_name incorrectly reported success"
	fi
	[ -z "$lookup_output" ] || fail "$lookup_name leaked an untrusted partial result"
done

# A resolver timeout rejects partial results and signals only its own child.
FAST_WAIT=1
export FAST_WAIT
nslookup unrelated.example &
unrelated_pid=$!
lookup_output=""
if lookup_output="$(arNslookup timeout.example 2>/dev/null)"; then
	fail "timed-out query returned an address"
fi
[ -z "$lookup_output" ] || fail "timed-out query leaked partial output"
kill -0 "$unrelated_pid" 2>/dev/null ||
	fail "lookup timeout killed an unrelated resolver process"
timeout_pid="$(awk '$1 == "timeout.example" { print $2; exit }' "$NSLOOKUP_PID_LOG")"
[ -n "$timeout_pid" ] || fail "timed-out resolver PID was not recorded"
if kill -0 "$timeout_pid" 2>/dev/null; then
	fail "timed-out resolver child survived cleanup"
fi
kill "$unrelated_pid" 2>/dev/null || :
wait "$unrelated_pid" 2>/dev/null || :

# Concurrent calls share $$ in their shell descendants; atomic mkdir must isolate them.
FAST_WAIT=0
export FAST_WAIT
(arNslookup concurrent-a.example > "$TEST_DIR/a.out") &
pid_a=$!
(arNslookup concurrent-b.example > "$TEST_DIR/b.out") &
pid_b=$!
wait "$pid_a" || fail "first concurrent lookup failed"
wait "$pid_b" || fail "second concurrent lookup failed"
[ "$(cat "$TEST_DIR/a.out")" = "203.0.113.11" ] ||
	fail "first concurrent lookup read another call's output"
[ "$(cat "$TEST_DIR/b.out")" = "203.0.113.22" ] ||
	fail "second concurrent lookup read another call's output"

# Every success and failure path must remove its private scratch directory.
for leftover in /tmp/arNslookup.$$.*; do
	[ ! -e "$leftover" ] || fail "temporary directory leaked: $leftover"
done

echo "AliDDNS bounded, isolated DNS lookup and strict IPv4/IPv6 lifecycle regression: PASS"
