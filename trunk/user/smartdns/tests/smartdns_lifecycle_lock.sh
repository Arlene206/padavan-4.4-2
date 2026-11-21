#!/bin/sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$HERE/../smartdns_lifecycle_lock.sh"
T=$(mktemp -d "${TMPDIR:-/tmp}/smartdns-lifecycle-lock.XXXXXX")
LOCK="$T/lock"
holder=
cleanup() {
	[ -z "$holder" ] || kill "$holder" 2>/dev/null || :
	rm -rf "$T"
}
trap cleanup 0
trap 'exit 1' HUP INT TERM
fail() { echo "SmartDNS lifecycle lock regression: FAIL: $*" >&2; exit 1; }
pass() { echo "SmartDNS lifecycle lock regression: PASS: $*"; }

smartdns_lifecycle_lock_acquire "$LOCK" 2 || fail "initial lock acquisition failed"
[ "$(cat "$LOCK/pid")" = "$$" ] || fail "lock owner PID mismatch"
if smartdns_lifecycle_lock_acquire "$LOCK" 1; then fail "active lock was acquired twice"; fi
[ "$(cat "$LOCK/pid")" = "$$" ] || fail "active lock owner was overwritten"
smartdns_lifecycle_lock_release || fail "owner could not release lock"
[ ! -e "$LOCK" ] || fail "released lock directory remains"
pass "exclusive acquire, owner validation and release"

sleep 30 &
holder=$!
mkdir -m 0700 "$LOCK"
printf '%s\n' "$holder" > "$LOCK/pid"
if smartdns_lifecycle_lock_acquire "$LOCK" 1; then fail "live foreign owner was stolen"; fi
[ "$(cat "$LOCK/pid")" = "$holder" ] || fail "live owner metadata changed"
kill "$holder" 2>/dev/null || :
wait "$holder" 2>/dev/null || :
holder=
smartdns_lifecycle_lock_acquire "$LOCK" 2 || fail "stale owner lock was not reclaimed"
[ "$(cat "$LOCK/pid")" = "$$" ] || fail "reclaimed lock has wrong owner"
smartdns_lifecycle_lock_release || fail "reclaimed lock could not be released"
pass "live-owner exclusion and dead-owner reclamation"

mkdir -m 0700 "$LOCK"
printf 'not-a-pid\n' > "$LOCK/pid"
if smartdns_lifecycle_lock_acquire "$LOCK" 1; then fail "malformed owner metadata was accepted"; fi
[ -d "$LOCK" ] || fail "malformed lock was destructively reclaimed"
rm -rf "$LOCK"
mkdir "$T/real-lock"
ln -s "$T/real-lock" "$LOCK"
if smartdns_lifecycle_lock_acquire "$LOCK" 1; then fail "lock symlink was accepted"; fi
[ -d "$T/real-lock" ] || fail "symlink target was modified"
[ -L "$LOCK" ] || fail "lock symlink was replaced"
pass "malformed owner and symlink locks fail closed"
echo "SmartDNS lifecycle lock regression: PASS"
