#!/bin/sh
set -eu
script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../../.." && pwd)
source_file="$repo_root/trunk/user/adbyby/adbyby.sh"
test_root=$(mktemp -d /tmp/adbyby-anti-ad-test.XXXXXX)
trap 'rm -rf "$test_root"' EXIT HUP INT TERM
. "$source_file" check >/dev/null

PROG_PATH="$test_root/adbyby"
DATA_PATH="$PROG_PATH/data"
ADBYBY_CA_BUNDLE="$test_root/ca-bundle.pem"
ADBYBY_ANTI_AD_FILE="$test_root/dnsmasq.d/anti-ad-for-dnsmasq.conf"
mkdir -p "$DATA_PATH" "$(dirname "$ADBYBY_ANTI_AD_FILE")"
printf 'test CA bundle\n' > "$ADBYBY_CA_BUNDLE"
: > "$test_root/curl.log"
: > "$test_root/curl-errors"
mock_mode=normal
publish_failure=0
configured_url='https://rules.example/adblock-for-dnsmasq.conf?channel=stable&format=dnsmasq'
nvram_count=
nvram()
{
    case "$1" in
        get)
            case "$2" in
                anti_ad) printf '1\n' ;;
                anti_ad_link) printf '%s\n' "$configured_url" ;;
                *) return 1 ;;
            esac
            ;;
        set)
            [ "$2" = anti_ad_count ] || return 1
            nvram_count="$3"
            ;;
        *) return 2 ;;
    esac
}
logger() { :; }
fail() { echo "Adbyby anti-ad fragment regression: FAIL: $*" >&2; exit 1; }

valid_fragment()
{
    printf '#TITLE=anti-AD\n#VER=20261111073634\n#TOTAL_LINES=2\naddress=/ads.example/\naddress=/sub.ads.example/\n'
}

curl()
{
    output=
    url=
    saw_cacert=0
    saw_proto=0
    saw_fail=0
    saw_connect_timeout=0
    saw_max_time=0
    saw_retry=0
    saw_limit_rate=0
    saw_max_filesize=0
    saw_writeout=0
    while [ "$#" -gt 0 ]; do
        case "$1" in
            --cacert) [ "$2" = "$ADBYBY_CA_BUNDLE" ] || { echo "wrong CA bundle" >> "$test_root/curl-errors"; return 1; }; saw_cacert=1; shift 2 ;;
            --proto) [ "$2" = '=https' ] || { echo "wrong protocol" >> "$test_root/curl-errors"; return 1; }; saw_proto=1; shift 2 ;;
            --output) output="$2"; shift 2 ;;
            --max-filesize) [ "$2" = 16777216 ] || { echo "wrong max size" >> "$test_root/curl-errors"; return 1; }; saw_max_filesize=1; shift 2 ;;
            --limit-rate) [ "$2" = 512k ] || { echo "wrong rate limit" >> "$test_root/curl-errors"; return 1; }; saw_limit_rate=1; shift 2 ;;
            --write-out) [ "$2" = '%{http_code}' ] || { echo "wrong HTTP status format" >> "$test_root/curl-errors"; return 1; }; saw_writeout=1; shift 2 ;;
            --connect-timeout) [ "$2" = 5 ] || { echo "wrong connect timeout" >> "$test_root/curl-errors"; return 1; }; saw_connect_timeout=1; shift 2 ;;
            --max-time) [ "$2" = 30 ] || { echo "wrong max timeout" >> "$test_root/curl-errors"; return 1; }; saw_max_time=1; shift 2 ;;
            --retry) [ "$2" = 3 ] || { echo "wrong retry count" >> "$test_root/curl-errors"; return 1; }; saw_retry=1; shift 2 ;;
            --fail) saw_fail=1; shift ;;
            --silent|--show-error) shift ;;
            -k|--insecure|--location) echo "unsafe curl option" >> "$test_root/curl-errors"; return 1 ;;
            https://*) url="$1"; shift ;;
            *) echo "unexpected curl argument: $1" >> "$test_root/curl-errors"; return 1 ;;
        esac
    done
    [ -n "$output" ] && [ -n "$url" ] || { echo "missing output or URL" >> "$test_root/curl-errors"; return 1; }
    [ "$saw_cacert" -eq 1 ] && [ "$saw_proto" -eq 1 ] && [ "$saw_fail" -eq 1 ] &&
    [ "$saw_connect_timeout" -eq 1 ] && [ "$saw_max_time" -eq 1 ] &&
    [ "$saw_retry" -eq 1 ] && [ "$saw_limit_rate" -eq 1 ] &&
    [ "$saw_max_filesize" -eq 1 ] && [ "$saw_writeout" -eq 1 ] ||
        { echo "download policy incomplete" >> "$test_root/curl-errors"; return 1; }
    [ "$(stat -c '%a' "$(dirname "$output")")" = 700 ] || { echo "stage directory not private" >> "$test_root/curl-errors"; return 1; }
    printf '%s\n' "$url" >> "$test_root/curl.log"
    case "$mock_mode" in
        malformed) printf 'conf-file=/etc/passwd\n' > "$output" ;;
        empty) : > "$output" ;;
        oversize) dd if=/dev/zero of="$output" bs=1048576 count=16 2>/dev/null; printf x >> "$output" ;;
        *) valid_fragment > "$output" ;;
    esac
    case "$mock_mode" in
        tls_failure) printf '000'; return 60 ;;
        http_redirect) printf '302'; return 0 ;;
        http_failure) printf '404'; return 22 ;;
        *) printf '200'; return 0 ;;
    esac
}

expected=$(printf '#TITLE=anti-AD\n#VER=20261111073634\n#TOTAL_LINES=2\naddress=/ads.example/\naddress=/sub.ads.example/\n')
count=$(adbyby_prepare_anti_ad "$configured_url" "$ADBYBY_ANTI_AD_FILE") || fail "valid dnsmasq blocklist rejected"
[ "$count" = 2 ] || fail "valid rule count is wrong: $count"
[ "$(cat "$ADBYBY_ANTI_AD_FILE")" = "$expected" ] || fail "valid complete list was not published"
[ "$(stat -c '%a' "$ADBYBY_ANTI_AD_FILE")" = 644 ] || fail "published fragment mode changed"
[ "$(cat "$test_root/curl.log")" = "$configured_url" ] || fail "query URL was split or rewritten"
before=$(cat "$ADBYBY_ANTI_AD_FILE")

for invalid_url in 'http://rules.example/list.conf' 'https://rules.example/bad path.conf'; do
    configured_url="$invalid_url"
    : > "$test_root/curl.log"
    if adbyby_prepare_anti_ad "$configured_url" "$ADBYBY_ANTI_AD_FILE"; then fail "invalid URL was accepted"; fi
    [ -z "$(cat "$test_root/curl.log")" ] || fail "invalid URL reached curl"
    [ "$(cat "$ADBYBY_ANTI_AD_FILE")" = "$before" ] || fail "invalid URL changed last-good"
done

for mode in tls_failure http_redirect http_failure malformed empty; do
    configured_url='https://rules.example/adblock-for-dnsmasq.conf?channel=stable&format=dnsmasq'
    mock_mode="$mode"
    if adbyby_prepare_anti_ad "$configured_url" "$ADBYBY_ANTI_AD_FILE"; then fail "$mode response was accepted"; fi
    [ "$(cat "$ADBYBY_ANTI_AD_FILE")" = "$before" ] || fail "$mode changed last-good"
done

rm -f "$ADBYBY_CA_BUNDLE"
: > "$test_root/curl.log"
if adbyby_prepare_anti_ad "$configured_url" "$ADBYBY_ANTI_AD_FILE"; then fail "missing CA bundle was accepted"; fi
[ -z "$(cat "$test_root/curl.log")" ] || fail "missing CA bundle reached curl"
adbyby_anti_ad_cache_is_usable "$ADBYBY_ANTI_AD_FILE" || fail "missing-CA fallback is not usable"
[ "$(adbyby_validate_dnsmasq_fragment "$ADBYBY_ANTI_AD_FILE")" = 2 ] || fail "last-good count was not retained"
printf 'test CA bundle\n' > "$ADBYBY_CA_BUNDLE"

mock_mode=oversize
if adbyby_prepare_anti_ad "$configured_url" "$ADBYBY_ANTI_AD_FILE"; then fail "oversized fragment was accepted"; fi
[ "$(cat "$ADBYBY_ANTI_AD_FILE")" = "$before" ] || fail "oversized response changed last-good"

sentinel="$test_root/sentinel"
printf 'sentinel\n' > "$sentinel"
unsafe="$test_root/dnsmasq.d/unsafe.conf"
ln -s "$sentinel" "$unsafe"
if adbyby_prepare_anti_ad "$configured_url" "$unsafe"; then fail "destination symlink was accepted"; fi
[ "$(cat "$sentinel")" = sentinel ] || fail "destination symlink referent changed"
rm -f "$unsafe"
mkdir "$unsafe"
if adbyby_prepare_anti_ad "$configured_url" "$unsafe"; then fail "non-regular destination was accepted"; fi
rmdir "$unsafe"

mock_mode=normal
publish_failure=1
mv()
{
    if [ "$publish_failure" = 1 ] && [ "$3" = "$ADBYBY_ANTI_AD_FILE" ]; then return 1; fi
    command mv "$@"
}
if adbyby_prepare_anti_ad "$configured_url" "$ADBYBY_ANTI_AD_FILE"; then fail "publication failure was accepted"; fi
publish_failure=0
unset -f mv

configured_url='http://rules.example/invalid.conf'
nvram_count=
anti_ad || fail "anti_ad wrapper unexpectedly failed"
[ "$nvram_count" = 2 ] || fail "anti_ad wrapper lost the last-good count: $nvram_count"
[ "$(cat "$ADBYBY_ANTI_AD_FILE")" = "$before" ] || fail "anti_ad wrapper changed last-good"

rm -f "$ADBYBY_ANTI_AD_FILE"
if adbyby_prepare_anti_ad "$configured_url" "$ADBYBY_ANTI_AD_FILE"; then fail "invalid source without last-good cache was accepted"; fi
[ ! -e "$ADBYBY_ANTI_AD_FILE" ] || fail "invalid first-time update left an active file"

[ ! -s "$test_root/curl-errors" ] || fail "$(cat "$test_root/curl-errors")"
echo "Adbyby anti-ad HTTPS fragment validation, private staging, atomic publication and last-good fallback: PASS"
