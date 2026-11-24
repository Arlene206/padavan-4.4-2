#!/bin/sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$HERE/../smartdns_config_file.sh"
T=$(mktemp -d "${TMPDIR:-/tmp}/smartdns-config-file.XXXXXX")
trap 'rm -rf "$T"' 0
trap 'exit 1' HUP INT TERM
fail() { echo "SmartDNS atomic config regression: FAIL: $*" >&2; exit 1; }
pass() { echo "SmartDNS atomic config regression: PASS: $*"; }

# The full base generator must not return success after a middle append
# fails: the previous unchecked echoes let a later successful command hide it.
smartdns_nvram_read_checked() {
    [ "$#" -eq 1 ] && [ "$1" = "sdns_staticnum_x" ] || return 90
    smartdns_nvram_value=0
    return 0
}
sdns_name=router
sdns_port=6053
sdns_address=0
sdns_ns=0
sdns_ipset=0
sdns_speed=0
sdns_as=0
sdns_ipv6_server=0
sdns_tcp_server=0
sdns_cache=4096
sdns_rr_ttl=60
sdns_rr_ttl_min=1
sdns_rr_ttl_max=86400
sdns_tcp_idle_time=300
sdns_rr_ttl_reply_max=3600
sdns_max_reply_ip_num=16
sdns_exp_ttl=600
sdns_exp_ttl_max=60
sdns_exp_prefetch_time=30
sdns_force_qtype_soa=0
sdns_speed_mode=ping,tcp:80
sdns_ip_change=0
sdns_ip_change_time=0
sdns_dualstack_ip_allow_force_aaaa=0
sdns_cache_persist=0
sdns_prefetch_domain=0
sdns_ipset_timeout=0
sdns_force_aaaa_soa=0
sdns_exp=0
sdns_adblock=0
sdns_white=0
sdns_black=0
sdnse_enable=0
smartdns_echo_count=0
echo() {
    smartdns_echo_count=$((smartdns_echo_count + 1))
    [ "$smartdns_echo_count" -ne 5 ] || return 43
    command echo "$@"
}
smartdns_tmp_Conf="$T/base-config-failed.conf"
if Get_sdns_conf; then fail "base config generator hid a middle append failure"; else status=$?; fi
unset -f echo
[ "$status" -eq 43 ] || fail "base config generator changed the append error to $status"
[ "$smartdns_echo_count" -eq 5 ] || fail "base config generator continued after failed append"
[ "$(wc -l < "$smartdns_tmp_Conf")" -eq 4 ] || fail "base config generator wrote past the failed append"
! grep -Fq 'rr-ttl-min ' "$smartdns_tmp_Conf" || fail "directive after failed append was published"
pass "base config generator propagates the exact mid-file append failure"

# A full filesystem must be reported on the first base-config write too.
smartdns_tmp_Conf=/dev/full
if Get_sdns_conf; then fail "base config generator ignored /dev/full"; else status=$?; fi
[ "$status" -ne 0 ] || fail "base config /dev/full failure returned success"
pass "base config generator rejects an unwritable staged file"

# A secondary listener write error must be visible even when TCP binding is
# disabled (the false conditional used to mask the failed first write).
sed -n '/^Get_sdns_conf () {/,/^}/p' "$HERE/../smartdns.sh" > "$T/smartdns_generators.sh"
sed -n '/^Get_sdnse_conf () {/,/^}/p' "$HERE/../smartdns.sh" >> "$T/smartdns_generators.sh"
[ -s "$T/smartdns_generators.sh" ] || fail "SmartDNS generator extraction failed"
. "$T/smartdns_generators.sh"
sdnse_enable=1
sdnse_speed=0
sdnse_name=
sdnse_address=0
sdnse_ns=0
sdnse_ipset=0
sdnse_as=0
sdnse_ipc=0
sdnse_cache=0
sdnse_ipv6_server=0
sdnse_tcp_server=0
sdnse_port=5335
smartdns_tmp_Conf=/dev/full
[ -e "$smartdns_tmp_Conf" ] || fail "/dev/full is unavailable for write-failure injection"
if Get_sdnse_conf; then fail "failed secondary UDP bind write was ignored"; else status=$?; fi
[ "$status" -ne 0 ] || fail "secondary UDP bind write returned success"
pass "secondary UDP bind write failure propagates with TCP bind disabled"

# Also inject a failure on the second (TCP) line after UDP was written.
smartdns_tmp_Conf="$T/secondary.conf"
: > "$smartdns_tmp_Conf"
sdnse_tcp_server=1
printf() {
    case "${2:-}" in
        "bind-tcp "*) return 39 ;;
    esac
    command printf "$@"
}
if Get_sdnse_conf; then fail "failed secondary TCP bind write was ignored"; else status=$?; fi
unset -f printf
[ "$status" -eq 39 ] || fail "secondary TCP bind status changed to $status"
grep -Fq 'bind :5335' "$smartdns_tmp_Conf" || fail "successful UDP bind was not retained"
! grep -Fq 'bind-tcp ' "$smartdns_tmp_Conf" || fail "failed TCP bind was published"
pass "secondary TCP bind failure propagates without a false directive"
SRC="$T/input.conf"
DST="$T/smartdns.conf"
printf 'server-name router\ncache-size 4096\nserver-name router\n' > "$SRC"
printf 'old-good-config\n' > "$DST"
smartdns_config_publish_deduplicated "$SRC" "$DST" || fail "valid deduplicated publication failed"
[ "$(cat "$DST")" = "$(printf 'server-name router\ncache-size 4096')" ] || fail "deduplicated content mismatch"
[ "$(stat -c '%a' "$DST")" = 644 ] || fail "published config mode is not 0644"
pass "successful publication is complete, deduplicated and mode-safe"

printf 'last-good-config\n' > "$DST"
cp "$DST" "$T/before-awk-failure"
mkdir "$T/failing-bin"
cat > "$T/failing-bin/awk" <<'AWK_FAIL'
#!/bin/sh
printf 'partial-output\n'
exit 37
AWK_FAIL
chmod 0700 "$T/failing-bin/awk"
old_path=$PATH
PATH="$T/failing-bin:$PATH"
export PATH
if smartdns_config_publish_deduplicated "$SRC" "$DST"; then fail "AWK failure was ignored"; else status=$?; fi
PATH=$old_path
export PATH
[ "$status" -eq 37 ] || fail "AWK status not propagated: $status"
cmp -s "$DST" "$T/before-awk-failure" || fail "AWK failure truncated the live config"
for tmp in "$DST".tmp.*; do [ ! -e "$tmp" ] || fail "AWK failure leaked staging file"; done
pass "generator failure preserves last-good config and removes stage"

cp "$DST" "$T/before-mv-failure"
mv() { return 41; }
if smartdns_config_publish_deduplicated "$SRC" "$DST"; then fail "rename failure was ignored"; else status=$?; fi
unset -f mv
[ "$status" -eq 41 ] || fail "rename status not propagated: $status"
cmp -s "$DST" "$T/before-mv-failure" || fail "rename failure changed live config"
for tmp in "$DST".tmp.*; do [ ! -e "$tmp" ] || fail "rename failure leaked staging file"; done
pass "rename failure preserves last-good config and cleans stage"

ln -s "$DST" "$T/source-link"
if smartdns_config_publish_deduplicated "$T/source-link" "$DST"; then fail "source symlink accepted"; fi
ln -s "$DST" "$T/destination-link"
if smartdns_config_publish_deduplicated "$SRC" "$T/destination-link"; then fail "destination symlink accepted"; fi
[ -L "$T/destination-link" ] || fail "destination symlink replaced"
mkdir "$T/directory-target"
if smartdns_config_publish_deduplicated "$SRC" "$T/directory-target"; then fail "directory destination accepted"; fi
if smartdns_config_publish_deduplicated "$T/missing-source" "$DST"; then fail "missing source accepted"; fi
pass "symlink, special-file and missing-source targets fail closed"

printf '%s\n' 'base-config' > "$DST"
printf '%s\n' '#comment' '' 'plain-rule' '  ' > "$T/extra.conf"
smartdns_config_append_filtered "$T/extra.conf" "$DST" || fail "optional config append failed"
[ "$(cat "$DST")" = "$(printf '%s\n' 'base-config' 'plain-rule' '  ')" ] || fail "filtered append content mismatch"
smartdns_config_append_filtered "$T/missing.conf" "$DST" || fail "missing optional config was not ignored"
pass "optional config append filters comments and empty lines"

printf '%s\n' 'staging-config' > "$T/stage.conf"
cp "$DST" "$T/live-before-append-error"
if smartdns_config_append_filtered "$T" "$T/stage.conf"; then fail "directory source was accepted"; fi
[ "$(cat "$T/stage.conf")" = "staging-config" ] || fail "invalid source changed staging file"
mkdir "$T/append-bin"
cat > "$T/append-bin/awk" <<'AWK_APPEND_FAIL'
#!/bin/sh
printf '%s\n' 'partial-rule'
exit 43
AWK_APPEND_FAIL
chmod 0700 "$T/append-bin/awk"
old_path=$PATH
PATH="$T/append-bin:$PATH"
export PATH
if smartdns_config_append_filtered "$T/extra.conf" "$T/stage.conf"; then fail "append AWK failure was ignored"; else status=$?; fi
PATH=$old_path
export PATH
[ "$status" -eq 43 ] || fail "append AWK failure status not propagated"
cmp -s "$DST" "$T/live-before-append-error" || fail "failed append changed the active config"
pass "append generator failure is propagated before publication"
echo "SmartDNS atomic configuration publication regression: PASS"
