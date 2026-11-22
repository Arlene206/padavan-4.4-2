#!/bin/sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$HERE/../smartdns_dnsmasq_config.sh"
T=$(mktemp -d "${TMPDIR:-/tmp}/smartdns-dnsmasq.XXXXXX")
trap 'rm -rf "$T"' 0
trap 'exit 1' HUP INT TERM
fail() { echo "SmartDNS DNSMasq regression: FAIL: $*" >&2; exit 1; }
pass() { echo "SmartDNS DNSMasq regression: PASS: $*"; }
count_line() { grep -Fxc "$1" "$2" || :; }
write_managed_fixture() {
	cat > "$1" <<'CONFIG'
# user resolver settings
no-resolv
server=127.0.0.1#6053
port=0
# smartdns-managed-dnsmasq-begin
no-resolv
server=127.0.0.1#6053
port=0
# smartdns-managed-dnsmasq-end
# user tail
no-resolv-extra
http-port=0
CONFIG
}
F="$T/dnsmasq.conf"
write_managed_fixture "$F"
smartdns_dnsmasq_remove_saved_directives "$F" 1 6053 managed-v1 || fail "managed block cleanup failed"
[ "$(count_line no-resolv "$F")" -eq 1 ] || fail "user no-resolv directive was removed"
[ "$(count_line 'server=127.0.0.1#6053' "$F")" -eq 1 ] || fail "user upstream was removed"
[ "$(count_line port=0 "$F")" -eq 1 ] || fail "user port=0 was removed"
! grep -Fq 'smartdns-managed-dnsmasq-' "$F" || fail "managed markers remained"
grep -Fxq '# user tail' "$F" || fail "user tail was removed"
grep -Fxq 'no-resolv-extra' "$F" || fail "near-match directive was removed"
cp "$F" "$T/after-managed"
smartdns_dnsmasq_remove_saved_directives "$F" 1 6053 managed-v1 || fail "repeat managed cleanup failed"
cmp -s "$F" "$T/after-managed" || fail "managed cleanup is not idempotent"
pass "marked cleanup preserves user-owned exact directives and is idempotent"

cat > "$F" <<'CONFIG'
port=0
no-resolv
# smartdns-managed-dnsmasq-begin
port=0
# smartdns-managed-dnsmasq-end
server=127.0.0.1#53
CONFIG
smartdns_dnsmasq_remove_saved_directives "$F" 2 53 managed-v1 || fail "port 53 marked cleanup failed"
[ "$(count_line port=0 "$F")" -eq 1 ] || fail "user port=0 was removed for port 53"
grep -Fxq no-resolv "$F" || fail "unmanaged no-resolv was removed"
grep -Fxq 'server=127.0.0.1#53' "$F" || fail "unmanaged server directive was removed"
pass "port 53 marker cleanup leaves same-valued user directives intact"

cat > "$F" <<'CONFIG'
no-resolv
server=127.0.0.1#6053
port=0
# no-resolv
no-resolv-extra
server=127.0.0.1#60530
port=0 # user note
http-port=0
CONFIG
smartdns_dnsmasq_remove_saved_directives "$F" 1 6053 legacy || fail "legacy exact cleanup failed"
! grep -Fxq no-resolv "$F" || fail "legacy no-resolv remained"
! grep -Fxq 'server=127.0.0.1#6053' "$F" || fail "legacy upstream remained"
grep -Fxq '# no-resolv' "$F" || fail "legacy cleanup removed a comment"
grep -Fxq no-resolv-extra "$F" || fail "legacy cleanup removed a near match"
grep -Fxq 'server=127.0.0.1#60530' "$F" || fail "legacy cleanup removed another port"
grep -Fxq 'port=0' "$F" || fail "legacy cleanup removed port=0 for a non-53 port"
grep -Fxq 'port=0 # user note' "$F" || fail "legacy cleanup removed annotated port"
pass "upgrade cleanup removes only exact legacy directives"

cat > "$F" <<'CONFIG'
user-before
# smartdns-managed-dnsmasq-begin
no-resolv
user-after
CONFIG
cp "$F" "$T/bad-marker.before"
if smartdns_dnsmasq_remove_saved_directives "$F" 1 6053 managed-v1; then fail "incomplete managed block accepted"; fi
cmp -s "$F" "$T/bad-marker.before" || fail "incomplete block changed live config"
pass "malformed marker block fails closed"

ln -s "$F" "$T/config-link"
if smartdns_dnsmasq_remove_saved_directives "$T/config-link" 1 6053 managed-v1; then fail "symlink config accepted"; fi
[ -L "$T/config-link" ] || fail "symlink replaced"
cmp -s "$F" "$T/bad-marker.before" || fail "symlink target changed"
pass "symlink configuration is refused"

smartdns_dnsmasq_remove_saved_directives "$T/missing.conf" 1 6053 managed-v1 || fail "missing file was not a no-op"
pass "missing configuration is a safe no-op"

# AWK's END block must not replace an input/read failure with a successful
# no-match result. Exercise every preliminary scan using a failing awk shim.
mkdir "$T/failing-bin"
cat > "$T/failing-bin/awk" <<'AWK_FAIL'
#!/bin/sh
exit 37
AWK_FAIL
chmod 0700 "$T/failing-bin/awk"
saved_path=$PATH
PATH="$T/failing-bin:$PATH"
export PATH

cat > "$F" <<'CONFIG'
no-resolv
server=127.0.0.1#6053
# smartdns-managed-dnsmasq-begin
no-resolv
# smartdns-managed-dnsmasq-end
user-tail
CONFIG
cp "$F" "$T/awk-failure.before"
if smartdns_dnsmasq_remove_exact_directives "$F" no-resolv; then
	scan_status=0
else
	scan_status=$?
fi
[ "$scan_status" -eq 37 ] || fail "exact-directive scan failure status was masked: $scan_status"
cmp -s "$F" "$T/awk-failure.before" || fail "exact-directive scan failure changed live config"

if smartdns_dnsmasq_remove_managed_block "$F"; then
	scan_status=0
else
	scan_status=$?
fi
[ "$scan_status" -eq 37 ] || fail "managed-block scan failure status was masked: $scan_status"
cmp -s "$F" "$T/awk-failure.before" || fail "managed-block scan failure changed live config"

if smartdns_dnsmasq_publish_managed_block "$F" no-resolv; then
	scan_status=0
else
	scan_status=$?
fi
[ "$scan_status" -eq 37 ] || fail "managed-block publish scan failure status was masked: $scan_status"
cmp -s "$F" "$T/awk-failure.before" || fail "publish scan failure changed live config"

PATH=$saved_path
export PATH
pass "all DNSMasq preliminary AWK scan failures propagate without changing live config"

cat > "$F" <<'CONFIG'
# user resolver before SmartDNS
no-resolv
server=127.0.0.1#6053
port=0
# user resolver after SmartDNS
CONFIG
cp "$F" "$T/publish.before"
smartdns_dnsmasq_publish_managed_block "$F" no-resolv 'server=127.0.0.1#6053' port=0 || fail "atomic managed-block publication failed"
grep -Fxq '# smartdns-managed-dnsmasq-begin' "$F" || fail "managed begin marker missing"
grep -Fxq '# smartdns-managed-dnsmasq-end' "$F" || fail "managed end marker missing"
[ "$(grep -Fxc no-resolv "$F")" -eq 2 ] || fail "publisher replaced or dropped user resolver"
[ "$(grep -Fxc 'server=127.0.0.1#6053' "$F")" -eq 2 ] || fail "publisher replaced or dropped user upstream"
[ "$(grep -Fxc port=0 "$F")" -eq 2 ] || fail "publisher replaced or dropped user port directive"
smartdns_dnsmasq_remove_saved_directives "$F" 1 6053 managed-v1 || fail "published managed-block cleanup failed"
cmp -s "$F" "$T/publish.before" || fail "managed block cleanup did not restore exact original file"
pass "managed-block publication is atomic and preserves same-valued user directives"

write_managed_fixture "$F"
cp "$F" "$T/publish-existing.before"
if smartdns_dnsmasq_publish_managed_block "$F" no-resolv; then fail "publisher accepted an existing marker block"; fi
cmp -s "$F" "$T/publish-existing.before" || fail "refused duplicate publication changed the file"
pass "duplicate or interrupted managed blocks fail closed"

write_managed_fixture "$F"
cp "$F" "$T/before-mv"
mv() { return 37; }
if smartdns_dnsmasq_remove_saved_directives "$F" 1 6053 managed-v1; then fail "rename failure ignored"; else status=$?; fi
unset -f mv
[ "$status" -eq 37 ] || fail "rename status not propagated"
cmp -s "$F" "$T/before-mv" || fail "rename failure modified live config"
for tmp in "$F".tmp.*; do [ ! -e "$tmp" ] || fail "temporary file leaked"; done
pass "rename failure preserves original config and cleans temporary file"

cat > "$F" <<'CONFIG'
user-before
user-after
CONFIG
cp "$F" "$T/publish-before-mv"
mv() { return 41; }
if smartdns_dnsmasq_publish_managed_block "$F" no-resolv; then fail "publish rename failure ignored"; else status=$?; fi
unset -f mv
[ "$status" -eq 41 ] || fail "publish rename status not propagated"
cmp -s "$F" "$T/publish-before-mv" || fail "failed publication changed live DNSMasq configuration"
for tmp in "$F".tmp.*; do [ ! -e "$tmp" ] || fail "publish leaked temporary file"; done
pass "failed managed-block publication leaves original config intact"

# Critical resolver mode/port reads must fail before any DNS state can be changed.
SMARTDNS_SOURCE="$HERE/../smartdns.sh"
sed -n '/^smartdns_nvram_read_checked()/,/^}/p' "$SMARTDNS_SOURCE" > "$T/startup-nvram-reader.sh"
[ -s "$T/startup-nvram-reader.sh" ] || fail "checked startup NVRAM reader missing"
. "$T/startup-nvram-reader.sh"
NVRAM_READ_FAIL_KEY=sdns_enable
NVRAM_READ_STATUS=51
nvram() {
    [ "${1:-}" = get_checked ] && [ "${2:-}" = "$NVRAM_READ_FAIL_KEY" ] || return 2
    return "$NVRAM_READ_STATUS"
}
logger() { printf '%s\n' "$*" >> "$T/startup-nvram.log"; }
: > "$T/startup-nvram.log"
if smartdns_nvram_read_checked sdns_enable; then
    fail "failed critical sdns_enable read was accepted"
else
    status=$?
fi
[ "$status" -eq 51 ] || fail "critical sdns_enable read status changed to $status"
grep -Fq 'Failed to read NVRAM key sdns_enable' "$T/startup-nvram.log" || fail "critical sdns_enable read failure was not logged"
NVRAM_READ_FAIL_KEY=sdns_port
NVRAM_READ_STATUS=52
if smartdns_nvram_read_checked sdns_port; then
    fail "failed critical sdns_port read was accepted"
else
    status=$?
fi
[ "$status" -eq 52 ] || fail "critical sdns_port read status changed to $status"
NVRAM_READ_FAIL_KEY=sdns_redirect
NVRAM_READ_STATUS=53
if smartdns_nvram_read_checked sdns_redirect; then
    fail "failed critical sdns_redirect read was accepted"
else
    status=$?
fi
[ "$status" -eq 53 ] || fail "critical sdns_redirect read status changed to $status"
NVRAM_READ_FAIL_KEY=sdnse_enable
NVRAM_READ_STATUS=54
if smartdns_nvram_read_checked sdnse_enable; then
    fail "failed secondary-resolver enable read was accepted"
else
    status=$?
fi
[ "$status" -eq 54 ] || fail "secondary-resolver enable read status changed to $status"
NVRAM_READ_FAIL_KEY=sdnse_port
NVRAM_READ_STATUS=55
if smartdns_nvram_read_checked sdnse_port; then
    fail "failed secondary-resolver port read was accepted"
else
    status=$?
fi
[ "$status" -eq 55 ] || fail "secondary-resolver port read status changed to $status"
pass "critical SmartDNS resolver NVRAM reads fail closed before lifecycle changes"

if grep -Fq '/usr/bin/adbyby.sh switch' "$SMARTDNS_SOURCE"; then
    fail "SmartDNS dispatched an unsupported Adbyby switch action"
fi
{
    sed -n '/^smartdns_nvram_read_checked()/,/^}/p' "$SMARTDNS_SOURCE"
    sed -n '/^Change_adbyby () {$/,/^}/p' "$SMARTDNS_SOURCE"
} > "$T/change-adbyby.sh"
[ -s "$T/change-adbyby.sh" ] || fail "Change_adbyby helper missing"
. "$T/change-adbyby.sh"
NVRAM_ADDBYBY_VALUE=1
NVRAM_SET_STATUS=37
NVRAM_READ_FAIL_KEY=
NVRAM_READ_STATUS=41
NVRAM_WRITE_LOG="$T/adbyby-writes.log"
: > "$NVRAM_WRITE_LOG"
pidof() { [ "${1:-}" = adbyby ] || return 1; printf '123\n'; }
nvram() {
    case "${1:-} ${2:-}" in
        "get_checked adbyby_enable")
            [ "$NVRAM_READ_FAIL_KEY" != adbyby_enable ] || return "$NVRAM_READ_STATUS"
            printf '1\n'
            ;;
        "get_checked adbyby_add")
            [ "$NVRAM_READ_FAIL_KEY" != adbyby_add ] || return "$NVRAM_READ_STATUS"
            printf '%s\n' "$NVRAM_ADDBYBY_VALUE"
            ;;
        "set adbyby_add=0"|"set adbyby_add=1")
            printf '%s\n' "$1 $2" >> "$NVRAM_WRITE_LOG"
            return "$NVRAM_SET_STATUS"
            ;;
        *) return 2 ;;
    esac
}
logger() { printf '%s\n' "$*" >> "$T/adbyby.log"; }

# Failed service-enabled reads must abort both ownership transitions.
: > "$T/adbyby.log"
NVRAM_SET_STATUS=0
NVRAM_READ_FAIL_KEY=adbyby_enable
NVRAM_READ_STATUS=41
sdns_enable=0
hosts_type=SmartDNS
action=start
if Change_adbyby; then
    fail "failed adbyby_enable read was accepted"
else
    status=$?
fi
[ "$status" -eq 41 ] || fail "adbyby_enable read error changed status to $status"
[ "$hosts_type" = SmartDNS ] || fail "adbyby_enable read failure changed active host mode"
[ "$NVRAM_ADDBYBY_VALUE" = 1 ] || fail "adbyby_enable read failure changed persistent state"
[ ! -s "$NVRAM_WRITE_LOG" ] || fail "adbyby_enable read failure attempted a persistent write"
grep -Fq 'Failed to read NVRAM key adbyby_enable' "$T/adbyby.log" || fail "adbyby_enable read failure was not logged"

# SmartDNS -> DNSMasq must not treat an unreadable adbyby_add as disabled.
: > "$T/adbyby.log"
: > "$NVRAM_WRITE_LOG"
NVRAM_READ_FAIL_KEY=adbyby_add
NVRAM_READ_STATUS=42
sdns_enable=0
hosts_type=SmartDNS
action=start
if Change_adbyby; then
    fail "failed adbyby_add read was accepted for DNSMasq transition"
else
    status=$?
fi
[ "$status" -eq 42 ] || fail "SmartDNS-to-DNSMasq read error changed status to $status"
[ "$hosts_type" = SmartDNS ] || fail "adbyby_add read failure changed active host mode"
[ "$NVRAM_ADDBYBY_VALUE" = 1 ] || fail "adbyby_add read failure changed persistent state"
[ ! -s "$NVRAM_WRITE_LOG" ] || fail "adbyby_add read failure attempted a persistent write"

# DNSMasq -> SmartDNS must also refuse an unreadable adbyby_add decision.
: > "$T/adbyby.log"
: > "$NVRAM_WRITE_LOG"
NVRAM_READ_STATUS=43
sdns_enable=1
hosts_type=dnsmasq
sdns_port=6053
sdns_redirect=0
action=start
if Change_adbyby; then
    fail "failed adbyby_add read was accepted for SmartDNS transition"
else
    status=$?
fi
[ "$status" -eq 43 ] || fail "DNSMasq-to-SmartDNS read error changed status to $status"
[ "$hosts_type" = dnsmasq ] || fail "reverse adbyby_add read failure changed active host mode"
[ "$NVRAM_ADDBYBY_VALUE" = 1 ] || fail "reverse adbyby_add read failure changed persistent state"
[ ! -s "$NVRAM_WRITE_LOG" ] || fail "reverse adbyby_add read failure attempted a persistent write"
pass "Adbyby NVRAM read failures preserve both DNS ownership targets and propagate status"

# A failed persistent write must likewise leave the active in-memory owner unchanged.
: > "$T/adbyby.log"
: > "$NVRAM_WRITE_LOG"
NVRAM_READ_FAIL_KEY=
NVRAM_SET_STATUS=37
NVRAM_ADDBYBY_VALUE=1
sdns_enable=0
hosts_type=SmartDNS
action=start
if Change_adbyby; then
    fail "failed SmartDNS-to-DNSMasq NVRAM transition was accepted"
else
    status=$?
fi
[ "$status" -eq 37 ] || fail "SmartDNS-to-DNSMasq NVRAM status changed to $status"
[ "$hosts_type" = SmartDNS ] || fail "failed DNSMasq transition changed active host mode"
[ "$NVRAM_ADDBYBY_VALUE" = 1 ] || fail "failed DNSMasq transition changed mock persistent value"
grep -Fq 'Failed to persist adbyby_add=0' "$T/adbyby.log" || fail "DNSMasq transition failure was not logged"
! grep -Fq 'DNS 去广告规则:' "$T/adbyby.log" || fail "failed DNSMasq transition was reported as successful"

: > "$T/adbyby.log"
: > "$NVRAM_WRITE_LOG"
NVRAM_ADDBYBY_VALUE=0
sdns_enable=1
hosts_type=dnsmasq
sdns_port=6053
sdns_redirect=2
if Change_adbyby; then
    fail "failed DNSMasq-to-SmartDNS NVRAM transition was accepted"
else
    status=$?
fi
[ "$status" -eq 37 ] || fail "SmartDNS transition NVRAM status changed to $status"
[ "$hosts_type" = dnsmasq ] || fail "failed SmartDNS transition changed active host mode"
[ "$NVRAM_ADDBYBY_VALUE" = 0 ] || fail "failed SmartDNS transition changed mock persistent value"
grep -Fq 'Failed to persist adbyby_add=1' "$T/adbyby.log" || fail "SmartDNS transition failure was not logged"
! grep -Fq 'DNS 去广告规则:' "$T/adbyby.log" || fail "failed SmartDNS transition was reported as successful"
pass "Adbyby NVRAM write failures preserve the active DNS target and propagate status"
pass "SmartDNS does not dispatch unsupported Adbyby switch action"


# Contract-test the actual SmartDNS and Adbyby helpers against one DNSMasq file.
# Adbyby's conf-dir/addn-hosts outputs remain DNSMasq-owned; SmartDNS's own
# adblock input is a separate SmartDNS config file, not an import of Adbyby data.
SMARTDNS_SOURCE="$HERE/../smartdns.sh"
ADBYBY_SOURCE="$HERE/../../adbyby/adbyby.sh"
{
    sed -n '/^smartdns_nvram_read_checked()/,/^}/p' "$SMARTDNS_SOURCE"
    sed -n '/^Change_adbyby () {$/,/^}/p' "$SMARTDNS_SOURCE"
    sed -n '/^Change_dnsmasq () {$/,/^}/p' "$SMARTDNS_SOURCE"
    sed -n '/^Get_sdnse_conf () {$/,/^}/p' "$SMARTDNS_SOURCE"
    sed -n '/^Get_sdns_conf () {$/,/^}/p' "$SMARTDNS_SOURCE"
} > "$T/runtime-owner-functions.sh"
[ -s "$T/runtime-owner-functions.sh" ] || fail "SmartDNS runtime ownership functions missing"
{
    sed -n '/^adbyby_remove_generated_hosts_config()/,/^}/p' "$ADBYBY_SOURCE"
    sed -n '/^hosts_ads()/,/^}/p' "$ADBYBY_SOURCE"
} > "$T/adbyby-runtime-functions.sh"
[ -s "$T/adbyby-runtime-functions.sh" ] || fail "Adbyby runtime host functions missing"
. "$T/runtime-owner-functions.sh"
. "$T/adbyby-runtime-functions.sh"

dnsmasq_Conf="$T/runtime-dnsmasq.conf"
smartdns_tmp_Conf="$T/runtime-smartdns.conf"
PROG_PATH="$T/adbyby"
ADBYBY_HOSTS_CONFIG_FILE="$dnsmasq_Conf"
mkdir -p "$PROG_PATH"
cat > "$dnsmasq_Conf" <<'CONFIG'
# operator-owned resolver configuration
conf-dir=/etc/storage/dnsmasq-adbyby.d
addn-hosts=/etc/storage/custom-hosts
CONFIG
adbyby_prepare_hosts_transaction() {
    mkdir -p "$PROG_PATH" || return 1
    printf '%s\n' '192.0.2.44 ads.example.test' > "$PROG_PATH/hosts" || return 1
}
adbyby_hosts_cache_is_usable() { return 1; }

NVRAM_ADDBYBY_VALUE=1
NVRAM_SET_STATUS=0
pidof() { [ "${1:-}" = adbyby ] || return 1; printf '123\n'; }
nvram() {
    case "${1:-} ${2:-}" in
        "get_checked adbyby_enable"|"get adbyby_enable") printf '1\n' ;;
        "get_checked adbyby_add"|"get adbyby_add") printf '%s\n' "$NVRAM_ADDBYBY_VALUE" ;;
        "get hosts_ad") printf '1\n' ;;
        "get sdns_staticnum_x") printf '0\n' ;;
        "set adbyby_add=0")
            [ "$NVRAM_SET_STATUS" -eq 0 ] || return "$NVRAM_SET_STATUS"
            NVRAM_ADDBYBY_VALUE=0
            ;;
        "set adbyby_add=1")
            [ "$NVRAM_SET_STATUS" -eq 0 ] || return "$NVRAM_SET_STATUS"
            NVRAM_ADDBYBY_VALUE=1
            ;;
        set\ adbyby_hostsad=*) return 0 ;;
        *) return 2 ;;
    esac
}
logger() { :; }

# Produce the same host-file reference used by Adbyby's real hosts_ads()
# function, leaving the Adbyby DNSMasq conf-dir fragment in place.
hosts_ads || fail "Adbyby did not publish its generated hosts file reference"
grep -Fxq "conf-dir=/etc/storage/dnsmasq-adbyby.d" "$dnsmasq_Conf" ||
    fail "Adbyby DNSMasq conf-dir reference missing"
grep -Fxq "addn-hosts=$PROG_PATH/hosts" "$dnsmasq_Conf" ||
    fail "Adbyby generated addn-hosts reference missing"
grep -Fxq '192.0.2.44 ads.example.test' "$PROG_PATH/hosts" ||
    fail "Adbyby generated hosts file was not materialized"

# SmartDNS -> DNSMasq: stop removes only SmartDNS's marked port=0 override,
# then Change_adbyby changes the saved target flag. Adbyby-owned DNSMasq
# files and host references must remain available to the restored resolver.
smartdns_dnsmasq_publish_managed_block "$dnsmasq_Conf" port=0 ||
    fail "could not install SmartDNS port-53 ownership marker"
smartdns_state_saved=1
smartdns_state_version=managed-v1
sdns_redirected=0
sdns_ported=53
sdns_enable=0
sdns_port=53
sdns_redirect=0
sdnse_enable=0
hosts_type=SmartDNS
action=stop
Change_dnsmasq stop || fail "SmartDNS -> DNSMasq resolver restoration failed"
Change_adbyby || fail "SmartDNS -> DNSMasq Adbyby target transition failed"
[ "$hosts_type" = dnsmasq ] || fail "SmartDNS -> DNSMasq transition did not update hosts_type"
[ "$NVRAM_ADDBYBY_VALUE" = 0 ] || fail "SmartDNS -> DNSMasq transition did not persist adbyby_add=0"
! grep -Fxq 'port=0' "$dnsmasq_Conf" ||
    fail "SmartDNS port=0 override remained after returning ownership to DNSMasq"
! grep -Fq 'smartdns-managed-dnsmasq-' "$dnsmasq_Conf" ||
    fail "SmartDNS managed markers remained after resolver restoration"
grep -Fxq "conf-dir=/etc/storage/dnsmasq-adbyby.d" "$dnsmasq_Conf" ||
    fail "SmartDNS cleanup removed Adbyby's DNSMasq conf-dir"
grep -Fxq "addn-hosts=$PROG_PATH/hosts" "$dnsmasq_Conf" ||
    fail "SmartDNS cleanup removed Adbyby's generated hosts reference"

# DNSMasq -> SmartDNS: the inverse transition persists adbyby_add=1 and adds
# port=0 when SmartDNS owns port 53. The DNSMasq-only Adbyby directives remain
# stored but are not imported into SmartDNS's generated configuration.
NVRAM_ADDBYBY_VALUE=0
hosts_type=dnsmasq
sdns_enable=1
sdns_port=53
sdns_redirect=0
sdnse_enable=0
action=start
Change_adbyby || fail "DNSMasq -> SmartDNS Adbyby target transition failed"
[ "$hosts_type" = SmartDNS ] || fail "DNSMasq -> SmartDNS transition did not update hosts_type"
[ "$NVRAM_ADDBYBY_VALUE" = 1 ] || fail "DNSMasq -> SmartDNS transition did not persist adbyby_add=1"
Change_dnsmasq start || fail "DNSMasq -> SmartDNS resolver takeover failed"
grep -Fxq 'port=0' "$dnsmasq_Conf" ||
    fail "DNSMasq did not disable its DNS listener for SmartDNS port 53"
grep -Fxq "conf-dir=/etc/storage/dnsmasq-adbyby.d" "$dnsmasq_Conf" ||
    fail "SmartDNS takeover unexpectedly removed DNSMasq Adbyby fragments"
grep -Fxq "addn-hosts=$PROG_PATH/hosts" "$dnsmasq_Conf" ||
    fail "SmartDNS takeover unexpectedly removed Adbyby host reference"

# Exercise the real SmartDNS config generator: its optional anti-ad input is
# /tmp/anti-ad-for-smartdns.conf, not the Adbyby hosts/DNSMasq fragment paths.
sdns_name=contract-test
sdns_address=0
sdns_ns=0
sdns_ipset=0
sdns_speed=0
sdns_as=0
sdns_ipv6_server=0
sdns_tcp_server=0
sdns_port=53
sdns_cache=128
sdns_rr_ttl=60
sdns_rr_ttl_min=10
sdns_rr_ttl_max=86400
sdns_tcp_idle_time=60
sdns_rr_ttl_reply_max=60
sdns_max_reply_ip_num=16
sdns_exp_ttl=60
sdns_exp_ttl_max=60
sdns_exp_prefetch_time=10
sdns_force_qtype_soa=0
sdns_speed_mode=ping
sdns_ip_change=0
sdns_force_aaaa_soa=0
sdns_dualstack_ip_allow_force_aaaa=0
sdns_cache_persist=0
sdns_prefetch_domain=0
sdns_ipset_timeout=0
sdns_exp=0
sdns_adblock=1
sdns_white=0
sdns_black=0
sdnse_enable=0
chn_Route="$T/absent-route"
nvram get sdns_staticnum_x >/dev/null 2>&1
Get_sdns_conf || fail "SmartDNS runtime config generation failed"
grep -Fxq 'conf-file /tmp/anti-ad-for-smartdns.conf' "$smartdns_tmp_Conf" ||
    fail "SmartDNS adblock config input was not emitted"
! grep -Fq '/etc/storage/dnsmasq-adbyby.d' "$smartdns_tmp_Conf" ||
    fail "SmartDNS config imported DNSMasq-only Adbyby fragments"
! grep -Fq "$PROG_PATH/hosts" "$smartdns_tmp_Conf" ||
    fail "SmartDNS config imported the DNSMasq-only Adbyby hosts file"
pass "SmartDNS/DNSMasq ownership transitions preserve Adbyby DNSMasq inputs and use separate SmartDNS adblock config"

[ "$(smartdns_dnsmasq_normalize_port 00053)" = 53 ] || fail "port normalization failed"
for bad in '' 0 65536 999999 abc 000000; do
	if smartdns_dnsmasq_normalize_port "$bad" >/dev/null; then fail "invalid port accepted"; fi
done
echo "SmartDNS DNSMasq configuration preservation regression: PASS"
