#!/bin/sh
set -eu
script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../../.." && pwd)
source_file="$repo_root/trunk/user/adbyby/adbyby.sh"
test_root=$(mktemp -d /tmp/adbyby-lifecycle.XXXXXX)
trap 'rm -rf "$test_root"' EXIT HUP INT TERM
policy_file="$test_root/policy.sh"
awk '
    /^adbyby_lifecycle_(lock_path|lock_prepare|lock_execute|locked_action|lock_run)\(\) \{$/ {
        capture = 1
        found++
    }
    capture { print }
    capture && /^}$/ { capture = 0; print "" }
    END { if (found != 5) exit 1 }
' "$source_file" > "$policy_file"
[ -s "$policy_file" ] || { echo "could not extract lifecycle functions" >&2; exit 1; }
. "$policy_file"
lock_file="$test_root/adbyby.lifecycle.lock"
events="$test_root/events"
nvram_log="$test_root/nvram.log"
marker="$test_root/held"
: > "$events"
: > "$nvram_log"
adbyby_lifecycle_lock_path() { printf '%s\n' "$lock_file"; }
logger() { :; }
nvram() {
    case "$1" in
        get) case "$2" in adbyby_enable) printf '%s\n' "$test_enable" ;; *) printf '\n' ;; esac ;;
        set) printf '%s\n' "$*" >> "$nvram_log" ;;
        *) return 2 ;;
    esac
}
adbyby_refresh_nvram_state() { adbyby_enable=$(nvram get adbyby_enable); }
adbyby_start() { printf 'start\n' >> "$events"; }
adbyby_close() {
    printf 'close-begin\n' >> "$events"
    if [ "$slow_close" = 1 ]; then : > "$marker"; sleep 2; fi
    printf 'close-end\n' >> "$events"
    [ "$fail_close" != 1 ]
}
adbyby_uprules() { printf 'update\n' >> "$events"; }
adbyby_refresh_blockip_set() { printf 'blockip\n' >> "$events"; }
add_rules() { printf 'rules\n' >> "$events"; }
add_rule() { printf 'firewall\n' >> "$events"; }
add_dns() { printf 'dns\n' >> "$events"; }
addscripts() { printf 'scripts\n' >> "$events"; }
hosts_ads() { printf 'hosts\n' >> "$events"; }
fail_action() { return 7; }
fail() { echo "Adbyby lifecycle regression: FAIL: $*" >&2; exit 1; }
test_enable=1
slow_close=0
fail_close=0
adbyby_lifecycle_lock_run B 2 || fail "B action failed"
[ "$(cat "$events")" = "blockip" ] || fail "B action did not dispatch narrowly"
[ ! -s "$nvram_log" ] || fail "B action reset NVRAM state"
[ "$(stat -c '%a' "$lock_file")" = 600 ] || fail "lock mode is not 0600"
[ "$(stat -c '%h' "$lock_file")" = 1 ] || fail "lock has extra hard links"
lock_inode=$(stat -c '%i' "$lock_file")
if ( : >&9 ) 2>/dev/null; then fail "FD 9 leaked after success"; fi
: > "$events"
: > "$nvram_log"
adbyby_lifecycle_lock_run updateadb 2 || fail "updateadb action failed"
[ "$(cat "$events")" = "update" ] || fail "updateadb did not dispatch updater"
[ -s "$nvram_log" ] || fail "non-B action missed legacy NVRAM reset"
: > "$events"
test_enable=1
adbyby_lifecycle_lock_run restart 2 || fail "restart failed"
[ "$(cat "$events")" = "$(printf 'close-begin\nclose-end\nstart')" ] || fail "restart was not one stop/start transaction"
: > "$events"
test_enable=0
adbyby_lifecycle_lock_run restart 2 || fail "disabled restart failed"
[ "$(cat "$events")" = "$(printf 'close-begin\nclose-end')" ] || fail "disabled service started"
: > "$events"
fail_close=1
if adbyby_lifecycle_lock_run restart 2; then fail "restart continued after stop failure"; fi
[ "$(cat "$events")" = "$(printf 'close-begin\nclose-end')" ] || fail "replacement started after stop failure"
fail_close=0
: > "$events"
rm -f "$marker"
test_enable=1
slow_close=1
adbyby_lifecycle_lock_run restart 5 &
first_pid=$!
attempt=0
while [ ! -e "$marker" ]; do
    attempt=$((attempt + 1))
    [ "$attempt" -lt 100 ] || fail "first launcher did not lock"
    sleep 0.05
done
slow_close=0
adbyby_lifecycle_lock_run stop 5 || fail "contending stop failed"
wait "$first_pid" || fail "first restart failed"
expected=$(printf 'close-begin\nclose-end\nstart\nclose-begin\nclose-end')
[ "$(cat "$events")" = "$expected" ] || fail "lifecycle transitions interleaved"
[ "$(stat -c '%i' "$lock_file")" = "$lock_inode" ] || fail "lock inode changed"
[ -f "$lock_file" ] || fail "lock inode unlinked"
if adbyby_lifecycle_lock_execute "$lock_file" 1 fail_action; then
    fail "failure status lost"
else
    status=$?
    [ "$status" -eq 7 ] || fail "failure status changed to $status"
fi
if ( : >&9 ) 2>/dev/null; then fail "FD 9 leaked after failure"; fi
rm -f "$lock_file"
sentinel="$test_root/sentinel"
printf 'preserve\n' > "$sentinel"
ln -s "$sentinel" "$lock_file"
if adbyby_lifecycle_lock_prepare "$lock_file"; then fail "symlink lock accepted"; fi
[ "$(cat "$sentinel")" = "preserve" ] || fail "symlink target changed"
rm -f "$lock_file"
: > "$lock_file"
chmod 644 "$lock_file"
if adbyby_lifecycle_lock_prepare "$lock_file"; then fail "permissive lock accepted"; fi
rm -f "$lock_file"
: > "$lock_file"
chmod 600 "$lock_file"
ln "$lock_file" "$test_root/extra-link"
if adbyby_lifecycle_lock_prepare "$lock_file"; then fail "hard-linked lock accepted"; fi
echo "Adbyby lifecycle serialization, bounded lock, update routing and FD hygiene: PASS"
