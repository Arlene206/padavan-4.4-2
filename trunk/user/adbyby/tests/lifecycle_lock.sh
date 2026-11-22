#!/bin/sh
set -eu
script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../../.." && pwd)
source_file="$repo_root/trunk/user/adbyby/adbyby.sh"
test_root=$(mktemp -d /tmp/adbyby-lifecycle.XXXXXX)
trap 'rm -rf "$test_root"' EXIT HUP INT TERM
policy_file="$test_root/policy.sh"
awk '
    /^adbyby_(reset_transient_nvram_state|lifecycle_(lock_path|lock_prepare|lock_execute|locked_action|lock_run))\(\) \{$/ {
        capture = 1
        found++
    }
    capture { print }
    capture && /^}$/ { capture = 0; print "" }
    END { if (found != 6) exit 1 }
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
nvram_fail_entry=""
nvram_fail_status=37
nvram_get_status=0
nvram() {
    case "$1" in
        get)
            [ "$nvram_get_status" -eq 0 ] || return "$nvram_get_status"
            case "$2" in adbyby_enable) printf '%s\n' "$test_enable" ;; *) printf '\n' ;; esac
            ;;
        set)
            printf '%s\n' "$*" >> "$nvram_log"
            if [ -n "$nvram_fail_entry" ] && [ "$2" = "$nvram_fail_entry" ]; then
                return "$nvram_fail_status"
            fi
            ;;
        *) return 2 ;;
    esac
}
adbyby_refresh_nvram_state() {
    adbyby_enable=$(nvram get adbyby_enable) || return $?
}
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
: > "$nvram_log"
nvram_fail_entry="adbybyip_ip_x_0="
if adbyby_lifecycle_lock_run start 2; then
    fail "start ignored a failed transient NVRAM reset"
else
    status=$?
    [ "$status" -eq 37 ] || fail "start changed NVRAM write status to $status"
fi
[ ! -s "$events" ] || fail "start launched after transient NVRAM reset failure"
[ "$(wc -l < "$nvram_log")" -eq 3 ] || fail "NVRAM reset continued after first failed write"
grep -Fqx 'set adbybyip_ip_x_0=' "$nvram_log" || fail "failed NVRAM key was not recorded"

: > "$events"
: > "$nvram_log"
if adbyby_lifecycle_lock_run stop 2; then
    fail "stop hid a transient NVRAM reset failure"
else
    status=$?
    [ "$status" -eq 37 ] || fail "stop changed NVRAM write status to $status"
fi
[ "$(cat "$events")" = "$(printf 'close-begin\nclose-end')" ] || fail "stop skipped cleanup after transient NVRAM reset failure"

: > "$events"
: > "$nvram_log"
nvram_fail_entry=""
nvram_get_status=39
if adbyby_lifecycle_lock_run stop 2; then
    fail "stop hid an NVRAM read failure"
else
    status=$?
    [ "$status" -eq 39 ] || fail "stop changed NVRAM read status to $status"
fi
[ "$(cat "$events")" = "$(printf 'close-begin\nclose-end')" ] || fail "stop skipped cleanup after NVRAM read failure"
[ ! -s "$nvram_log" ] || fail "stop attempted resets after NVRAM read failure"
nvram_get_status=0
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
# del_dns is a best-effort cleanup transaction: attempt every independent
# removal, report the first failure, and never hide it behind a later rm.
del_dns_file="$test_root/del-dns.sh"
awk '
    /^del_dns\(\)$/ { capture = 1; found++ }
    capture { print }
    capture && /^}$/ { capture = 0; print "" }
    END { if (found != 1) exit 1 }
' "$source_file" > "$del_dns_file"
[ -s "$del_dns_file" ] || fail "could not extract del_dns"
(
    . "$del_dns_file"
    ADBYBY_DNSMASQ_AD_DIR=/etc/storage/dnsmasq-adbyby.d
    events="$test_root/del-dns-events"
    fail_stages=""
    fail() { echo "Adbyby del_dns regression: FAIL: $*" >&2; exit 1; }
    sed() {
        printf '%s\n' sed >> "$events"
        case ",$fail_stages," in *,sed,*) return 31 ;; esac
        return 0
    }
    adbyby_remove_generated_hosts_config() {
        printf '%s\n' generated_hosts >> "$events"
        case ",$fail_stages," in *,generated_hosts,*) return 32 ;; esac
        return 0
    }
    rm() {
        case "$2" in
            /tmp/dnsmasq.d/dnsmasq-adbyby.conf) step=rm_dns; rc=33 ;;
            /tmp/adbyby_host.conf) step=rm_hosts; rc=35 ;;
            *) return 2 ;;
        esac
        printf '%s\n' "$step" >> "$events"
        case ",$fail_stages," in *,"$step",*) return "$rc" ;; esac
        return 0
    }
    adbyby_cleanup_dnsmasq_ad_dir() {
        printf '%s\n' cleanup_dir >> "$events"
        case ",$fail_stages," in *,cleanup_dir,*) return 34 ;; esac
        return 0
    }
    logger() { printf 'logger:%s\n' "$*" >> "$events"; }
    run_del_dns_case() {
        fail_stages=$1
        expected_status=$2
        : > "$events"
        if del_dns; then actual_status=0; else actual_status=$?; fi
        [ "$actual_status" -eq "$expected_status" ] ||
            fail "failures [$fail_stages] returned $actual_status, expected $expected_status"
        for step in sed generated_hosts rm_dns cleanup_dir rm_hosts; do
            grep -Fqx "$step" "$events" || fail "[$fail_stages] skipped cleanup step $step"
        done
    }
    run_del_dns_case "" 0
    run_del_dns_case "sed" 31
    run_del_dns_case "generated_hosts" 32
    run_del_dns_case "rm_dns" 33
    run_del_dns_case "cleanup_dir" 34
    run_del_dns_case "rm_hosts" 35
    run_del_dns_case "sed,cleanup_dir" 31
    echo "Adbyby DNS cleanup preserves first error and attempts all stages: PASS"
) || fail "del_dns cleanup regression failed"

# Once daemon absence is confirmed, failure in one cleanup subsystem must
# not suppress independent firewall, cron, DNS or helper-process cleanup.
close_file="$test_root/adbyby-close.sh"
awk '
    /^adbyby_close\(\)$/ { capture = 1; found++ }
    capture { print }
    capture && /^}$/ { capture = 0; print "" }
    END { if (found != 1) exit 1 }
' "$source_file" > "$close_file"
[ -s "$close_file" ] || fail "could not extract adbyby_close"
(
    . "$close_file"
    events="$test_root/close-events"
    logs="$test_root/close-logs"
    fail_stages=""
    fail() { echo "Adbyby close regression: FAIL: $*" >&2; exit 1; }
    adbyby_stop_daemon() {
        printf '%s\n' stop >> "$events"
        case ",$fail_stages," in *,stop,*) return 40 ;; esac
        return 0
    }
    del_rule() {
        printf '%s\n' del_rule >> "$events"
        case ",$fail_stages," in *,del_rule,*) return 41 ;; esac
        return 0
    }
    del_cron() {
        printf '%s\n' del_cron >> "$events"
        case ",$fail_stages," in *,del_cron,*) return 42 ;; esac
        return 0
    }
    del_dns() {
        printf '%s\n' del_dns >> "$events"
        case ",$fail_stages," in *,del_dns,*) return 43 ;; esac
        return 0
    }
    adbyby_restart_dhcpd() {
        printf '%s\n' restart_dhcpd >> "$events"
        case ",$fail_stages," in *,restart_dhcpd,*) return 44 ;; esac
        return 0
    }
    ps() { :; }
    kill() { printf '%s\n' kill_admem >> "$events"; return 0; }
    logger() { printf '%s\n' "$*" >> "$logs"; }
    run_close_case() {
        fail_stages=$1
        expected_status=$2
        expect_restart=$3
        : > "$events"
        : > "$logs"
        if adbyby_close; then actual_status=0; else actual_status=$?; fi
        [ "$actual_status" -eq "$expected_status" ] ||
            fail "failures [$fail_stages] returned $actual_status, expected $expected_status"
        if [ "$fail_stages" = "stop" ]; then
            expected_events=$(printf 'stop')
        else
            expected_events=$(printf 'stop\ndel_rule\ndel_cron\ndel_dns\nkill_admem')
            if [ "$expect_restart" = "1" ]; then
                expected_events=$(printf '%s\nrestart_dhcpd' "$expected_events")
            fi
        fi
        [ "$(cat "$events")" = "$expected_events" ] ||
            fail "failures [$fail_stages] produced unexpected cleanup order: $(cat "$events")"
        if [ "$fail_stages" = "del_dns" ] ||
           [ "$fail_stages" = "del_rule,del_dns" ]; then
            grep -Fq "Skipping DHCP service restart" "$logs" ||
                fail "DNS cleanup failure did not explain skipped dnsmasq restart"
        fi
    }
    run_close_case "" 0 1
    run_close_case "del_rule" 41 1
    run_close_case "del_cron" 42 1
    run_close_case "del_dns" 43 0
    run_close_case "restart_dhcpd" 44 1
    run_close_case "del_rule,del_cron" 41 1
    run_close_case "del_rule,del_dns" 41 0
    run_close_case "stop" 1 0
    echo "Adbyby close attempts independent cleanup and preserves first error: PASS"
) || fail "Adbyby close cleanup regression failed"

echo "Adbyby lifecycle serialization, bounded lock, update routing and FD hygiene: PASS"
