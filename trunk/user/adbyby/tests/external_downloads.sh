#!/bin/sh
set -eu
script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../../.." && pwd)
source_file="$repo_root/trunk/user/adbyby/adbyby.sh"
test_root=$(mktemp -d /tmp/adbyby-external-test.XXXXXX)
trap 'rm -rf "$test_root"' EXIT HUP INT TERM

. "$source_file" check >/dev/null
PROG_PATH="$test_root/adbyby"
DATA_PATH="$PROG_PATH/data"
ADBYBY_CA_BUNDLE="$test_root/ca-bundle.pem"
remote="$test_root/remote"
mkdir -p "$DATA_PATH" "$remote"
printf 'test CA bundle\n' > "$ADBYBY_CA_BUNDLE"
printf '||remote.example^\n@@||allow.example^\n' > "$remote/custom.txt"
printf '||new-remote.example^\n' > "$remote/custom.next"
printf '||local.example^\n' > "$PROG_PATH/rules.txt"
printf 'previous complete custom rules\n' > "$DATA_PATH/user.txt"
printf 'stale compiled cache\n' > "$DATA_PATH/user.bin"
: > "$test_root/curl.log"
: > "$test_root/curl-errors"

mock_mode=normal
custom_url='https://rules.example/list.txt?source=a%26b&tag=stable'
nvram()
{
    case "$1" in
        get)
            case "$2" in
                adbybyrules_staticnum_x) printf '1\n' ;;
                adbybyrules_x0) printf '%s\n' "$custom_url" ;;
                adbybyrules_road_x0) printf '1\n' ;;
                *) return 1 ;;
            esac
            ;;
        set) return 0 ;;
        *) return 2 ;;
    esac
}
logger() { :; }

fail()
{
    echo "Adbyby external download regression: FAIL: $*" >&2
    exit 1
}

curl()
{
    output=
    url=
    max_bytes=
    saw_cacert=0
    saw_proto=0
    saw_fail=0
    saw_connect_timeout=0
    saw_max_time=0
    saw_retry=0
    saw_limit_rate=0
    saw_writeout=0
    while [ "$#" -gt 0 ]; do
        case "$1" in
            --cacert) [ "$2" = "$ADBYBY_CA_BUNDLE" ] || { echo "wrong CA bundle" >> "$test_root/curl-errors"; return 1; }; saw_cacert=1; shift 2 ;;
            --proto) [ "$2" = '=https' ] || { echo "wrong protocol" >> "$test_root/curl-errors"; return 1; }; saw_proto=1; shift 2 ;;
            --output) output="$2"; shift 2 ;;
            --max-filesize) max_bytes="$2"; shift 2 ;;
            --limit-rate) [ "$2" = 512k ] || { echo "wrong rate bound" >> "$test_root/curl-errors"; return 1; }; saw_limit_rate=1; shift 2 ;;
            --write-out) [ "$2" = '%{http_code}' ] || { echo "wrong HTTP status format" >> "$test_root/curl-errors"; return 1; }; saw_writeout=1; shift 2 ;;
            --connect-timeout) [ "$2" = 5 ] || { echo "wrong connect timeout" >> "$test_root/curl-errors"; return 1; }; saw_connect_timeout=1; shift 2 ;;
            --max-time) [ "$2" = 30 ] || { echo "wrong total timeout" >> "$test_root/curl-errors"; return 1; }; saw_max_time=1; shift 2 ;;
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
    [ "$saw_retry" -eq 1 ] && [ "$saw_limit_rate" -eq 1 ] && [ "$saw_writeout" -eq 1 ] &&
    [ -n "$max_bytes" ] || { echo "download policy incomplete" >> "$test_root/curl-errors"; return 1; }
    [ -d "$(dirname "$output")" ] || { echo "stage directory missing" >> "$test_root/curl-errors"; return 1; }
    [ "$(stat -c '%a' "$(dirname "$output")")" = 700 ] || { echo "stage directory not private" >> "$test_root/curl-errors"; return 1; }
    [ ! -e "$output" ] || { echo "staging output was reused" >> "$test_root/curl-errors"; return 1; }
    printf '%s\n' "$url" >> "$test_root/curl.log"
    case "$mock_mode" in
        oversize) printf '0123456789012345678901234567890123456789' > "$output" ;;
        *) cp "$remote/custom.txt" "$output" ;;
    esac
    case "$mock_mode" in
        tls_failure) printf '000'; return 60 ;;
        http_redirect) printf '302'; return 0 ;;
        http_failure) printf '404'; return 22 ;;
        *) printf '200'; return 0 ;;
    esac
}

adbyby_rules_x=1
# Query delimiters remain part of one quoted URL argument.
adbyby_prepare_user_rules || fail "valid third-party list failed"
[ "$(cat "$test_root/curl.log")" = "$custom_url" ] || fail "configured URL was split or rewritten"
grep -q '||remote.example\^' "$DATA_PATH/user.txt" || fail "remote rules missing"
grep -q '||local.example\^' "$DATA_PATH/user.txt" || fail "local rules were not preserved"
[ "$(stat -c '%a' "$DATA_PATH/user.txt")" = 644 ] || fail "published user rules mode changed"
[ ! -e "$DATA_PATH/user.bin" ] || fail "stale compiled cache was not invalidated"
before=$(cat "$DATA_PATH/user.txt")

# Invalid schemes and whitespace are rejected before invoking curl; last-good survives.
for invalid_url in 'http://rules.example/list.txt' 'https://rules.example/bad path.txt'; do
    custom_url="$invalid_url"
    : > "$test_root/curl.log"
    adbyby_prepare_user_rules || fail "invalid URL did not fall back to last-good rules"
    [ -z "$(cat "$test_root/curl.log")" ] || fail "invalid URL reached curl"
    [ "$(cat "$DATA_PATH/user.txt")" = "$before" ] || fail "invalid URL replaced last-good rules"
done

# TLS, redirect and HTTP failures cannot truncate the published cache.
for mode in tls_failure http_redirect http_failure; do
    custom_url='https://rules.example/list.txt?source=a%26b&tag=stable'
    mock_mode="$mode"
    : > "$test_root/curl.log"
    adbyby_prepare_user_rules || fail "$mode did not fall back to last-good rules"
    [ "$(cat "$DATA_PATH/user.txt")" = "$before" ] || fail "$mode changed last-good rules"
done

# Response size is checked even if a server omits/misreports Content-Length.
mock_mode=oversize
oversize_dir=$(mktemp -d "$test_root/oversize.XXXXXX")
if adbyby_download_https_200 "$custom_url" "$oversize_dir/body" 16; then
    fail "oversized response was accepted"
fi

# A bad source without a prior complete user.txt fails closed.
rm -f "$DATA_PATH/user.txt"
custom_url='http://rules.example/list.txt'
if adbyby_prepare_user_rules; then
    fail "invalid URL without last-good rules was accepted"
fi

[ ! -s "$test_root/curl-errors" ] || fail "$(cat "$test_root/curl-errors")"
echo "Adbyby HTTPS-only third-party URL validation, bounded staging and last-good publication: PASS"
