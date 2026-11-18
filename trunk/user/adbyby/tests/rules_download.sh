#!/bin/sh
set -eu
script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../../.." && pwd)
source_file="$repo_root/trunk/user/adbyby/adbyby.sh"
test_root=$(mktemp -d /tmp/adbyby-rules-test.XXXXXX)
trap 'rm -rf "$test_root"' EXIT HUP INT TERM

# Source production helpers without dispatching a service action.
. "$source_file" check >/dev/null
PROG_PATH="$test_root/adbyby"
DATA_PATH="$PROG_PATH/data"
ADBYBY_CA_BUNDLE="$test_root/ca-bundle.pem"
remote="$test_root/remote"
mkdir -p "$DATA_PATH" "$remote"
printf 'test CA bundle\n' > "$ADBYBY_CA_BUNDLE"
printf 'old lazy rules\n' > "$DATA_PATH/lazy.txt"
printf 'old video rules\n' > "$DATA_PATH/video.txt"
printf 'new lazy rules\n' > "$remote/lazy.txt"
printf 'new video rules\n' > "$remote/video.txt"
printf 'partial lazy response\n' > "$remote/lazy.partial"
: > "$test_root/curl.log"
: > "$test_root/stage.log"
: > "$test_root/curl-errors"

digest_file()
{
    md5sum "$1" | awk '{print $1}'
}

write_manifest()
{
    printf '{"lazy":"%s","video":"%s"}\n' \
        "$(digest_file "$remote/lazy.txt")" "$(digest_file "$remote/video.txt")" > "$remote/md5.json"
}

write_manifest
mock_mode=normal
logger() { :; }

fail()
{
    echo "Adbyby rule download regression: FAIL: $*" >&2
    exit 1
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
    saw_max_filesize=0
    saw_limit_rate=0
    saw_writeout=0
    while [ "$#" -gt 0 ]; do
        case "$1" in
            --cacert) saw_cacert=1; [ -n "${2:-}" ] || { echo "empty CA path" >> "$test_root/curl-errors"; return 1; }; [ "$2" = "$ADBYBY_CA_BUNDLE" ] || { echo "wrong CA bundle" >> "$test_root/curl-errors"; return 1; }; shift 2 ;;
            --output) output="$2"; shift 2 ;;
            --proto) [ "$2" = '=https' ] || { echo "wrong curl protocol restriction" >> "$test_root/curl-errors"; return 1; }; saw_proto=1; shift 2 ;;
            --max-filesize) [ "$2" = 16777216 ] || { echo "wrong max file size" >> "$test_root/curl-errors"; return 1; }; saw_max_filesize=1; shift 2 ;;
            --limit-rate) [ "$2" = 512k ] || { echo "wrong transfer rate limit" >> "$test_root/curl-errors"; return 1; }; saw_limit_rate=1; shift 2 ;;
            --write-out) [ "$2" = '%{http_code}' ] || { echo "unexpected write-out" >> "$test_root/curl-errors"; return 1; }; saw_writeout=1; shift 2 ;;
            --connect-timeout) [ "$2" = 5 ] || { echo "wrong connect timeout" >> "$test_root/curl-errors"; return 1; }; saw_connect_timeout=1; shift 2 ;;
            --max-time) [ "$2" = 30 ] || { echo "wrong total timeout" >> "$test_root/curl-errors"; return 1; }; saw_max_time=1; shift 2 ;;
            --retry) [ "$2" = 3 ] || { echo "wrong retry count" >> "$test_root/curl-errors"; return 1; }; saw_retry=1; shift 2 ;;
            --fail) saw_fail=1; shift ;;
            --silent|--show-error) shift ;;
            -k|--insecure) echo "TLS verification disabled" >> "$test_root/curl-errors"; return 1 ;;
            https://*) url="$1"; shift ;;
            *) echo "unexpected curl argument: $1" >> "$test_root/curl-errors"; return 1 ;;
        esac
    done
    [ -n "$output" ] && [ -n "$url" ] || { echo "missing output or URL" >> "$test_root/curl-errors"; return 1; }
    [ "$saw_cacert" -eq 1 ] && [ "$saw_proto" -eq 1 ] && [ "$saw_fail" -eq 1 ] &&
    [ "$saw_connect_timeout" -eq 1 ] && [ "$saw_max_time" -eq 1 ] &&
    [ "$saw_retry" -eq 1 ] && [ "$saw_max_filesize" -eq 1 ] &&
    [ "$saw_limit_rate" -eq 1 ] && [ "$saw_writeout" -eq 1 ] || {
        echo "curl security/timeout policy missing" >> "$test_root/curl-errors"; return 1;
    }
    [ -d "$(dirname "$output")" ] || { echo "staging directory missing" >> "$test_root/curl-errors"; return 1; }
    [ "$(stat -c '%a' "$(dirname "$output")")" = 700 ] || { echo "staging directory not private" >> "$test_root/curl-errors"; return 1; }
    printf '%s\n' "$(dirname "$output")" > "$test_root/stage.log"
    printf '%s %s\n' "$url" "$output" >> "$test_root/curl.log"
    [ ! -e "$output" ] || { echo "staging output already exists" >> "$test_root/curl-errors"; return 1; }
    case "$url" in
        */md5.json) cp "$remote/md5.json" "$output" ;;
        */lazy.txt)
            if [ "$mock_mode" = corrupt_lazy ]; then
                cp "$remote/lazy.partial" "$output"
            else
                cp "$remote/lazy.txt" "$output"
            fi
            ;;
        */video.txt) cp "$remote/video.txt" "$output" ;;
        *) echo "unexpected download URL: $url" >> "$test_root/curl-errors"; return 1 ;;
    esac
    case "$mock_mode" in
        tls_failure) printf '000'; return 60 ;;
        http_redirect) printf '302'; return 0 ;;
        http_failure) printf '404'; return 22 ;;
        *) printf '200'; return 0 ;;
    esac
}

# First update: all three responses are verified before publication.
adbyby_refresh_managed_rules || fail "valid update failed"
[ ! -s "$test_root/curl-errors" ] || fail "$(cat "$test_root/curl-errors")"
[ "$(cat "$DATA_PATH/lazy.txt")" = "new lazy rules" ] || fail "lazy rules not updated"
[ "$(cat "$DATA_PATH/video.txt")" = "new video rules" ] || fail "video rules not updated"
[ "$(stat -c '%a' "$DATA_PATH/lazy.txt")" = 600 ] || fail "published lazy file is not private"
stage_dir=$(cat "$test_root/stage.log")
[ ! -e "$stage_dir" ] || fail "private stage was not cleaned"
[ "$(wc -l < "$test_root/curl.log")" -eq 3 ] || fail "update did not use three unique staged requests"

# No update: only the manifest is fetched.
: > "$test_root/curl.log"
adbyby_refresh_managed_rules || fail "matching cache was rejected"
[ "$(wc -l < "$test_root/curl.log")" -eq 1 ] || fail "matching cache caused redundant downloads"

# TLS, redirect and HTTP failures preserve the complete previous cache.
printf 'next lazy rules\n' > "$remote/lazy.txt"
printf 'next video rules\n' > "$remote/video.txt"
write_manifest
printf 'old snapshot lazy\n' > "$DATA_PATH/lazy.txt"
printf 'old snapshot video\n' > "$DATA_PATH/video.txt"
mock_mode=tls_failure
: > "$test_root/curl.log"
adbyby_refresh_managed_rules || fail "TLS outage did not fall back to usable cache"
[ "$(cat "$DATA_PATH/lazy.txt")" = "old snapshot lazy" ] || fail "TLS failure changed lazy cache"
[ "$(cat "$DATA_PATH/video.txt")" = "old snapshot video" ] || fail "TLS failure changed video cache"
[ "$(wc -l < "$test_root/curl.log")" -eq 1 ] || fail "TLS failure made unexpected requests"

mock_mode=http_redirect
: > "$test_root/curl.log"
adbyby_refresh_managed_rules || fail "redirect did not preserve usable cache"
[ "$(cat "$DATA_PATH/lazy.txt")" = "old snapshot lazy" ] || fail "redirect changed lazy cache"

mock_mode=http_failure
: > "$test_root/curl.log"
adbyby_refresh_managed_rules || fail "HTTP failure did not preserve usable cache"
[ "$(cat "$DATA_PATH/video.txt")" = "old snapshot video" ] || fail "HTTP failure changed video cache"

# A valid response with a partial/corrupt body must never be published.
mock_mode=corrupt_lazy
: > "$test_root/curl.log"
adbyby_refresh_managed_rules || fail "digest mismatch should retain a valid cache"
[ "$(cat "$DATA_PATH/lazy.txt")" = "old snapshot lazy" ] || fail "partial lazy response was published"
[ "$(cat "$DATA_PATH/video.txt")" = "old snapshot video" ] || fail "partial update changed video cache"

# The second rename failure must roll back the first file.
mock_mode=normal
mv()
{
    adbyby_mv_source="$1"
    [ "$adbyby_mv_source" = "-f" ] && adbyby_mv_source="$2"
    case "$adbyby_mv_source" in */video.next) return 1 ;; esac
    command mv "$@"
}
if adbyby_refresh_managed_rules; then
    fail "second rename failure was ignored"
fi
unset -f mv
[ "$(cat "$DATA_PATH/lazy.txt")" = "old snapshot lazy" ] || fail "first file was not rolled back"
[ "$(cat "$DATA_PATH/video.txt")" = "old snapshot video" ] || fail "second file changed on failed publication"
[ ! -e "$DATA_PATH/.rules-transaction" ] || fail "failed publication left an unrecovered transaction marker"

# Simulate abrupt shell termination after lazy.txt was replaced but before video.txt.
printf 'recovery old lazy rules\n' > "$DATA_PATH/lazy.txt"
printf 'recovery old video rules\n' > "$DATA_PATH/video.txt"
recovery_stage=$(mktemp -d /tmp/adbyby-rules.XXXXXX)
cp -p "$DATA_PATH/lazy.txt" "$recovery_stage/lazy.previous"
cp -p "$DATA_PATH/video.txt" "$recovery_stage/video.previous"
( umask 077
  printf 'present\n' > "$recovery_stage/lazy.state"
  printf 'present\n' > "$recovery_stage/video.state"
  printf '%s\n' "$recovery_stage" > "$recovery_stage/transaction.next"
)
mv -f "$recovery_stage/transaction.next" "$DATA_PATH/.rules-transaction"
printf 'interrupted new lazy rules\n' > "$DATA_PATH/lazy.txt"
cp "$recovery_stage/lazy.previous" "$remote/lazy.txt"
cp "$recovery_stage/video.previous" "$remote/video.txt"
write_manifest
: > "$test_root/curl.log"
adbyby_refresh_managed_rules || fail "refresh did not recover an interrupted pair before reading the cache"
[ "$(cat "$DATA_PATH/lazy.txt")" = "recovery old lazy rules" ] || fail "interrupted lazy publication was not rolled back"
[ "$(cat "$DATA_PATH/video.txt")" = "recovery old video rules" ] || fail "interrupted video publication was not restored"
[ ! -e "$DATA_PATH/.rules-transaction" ] || fail "successful recovery left the transaction marker"
[ ! -e "$recovery_stage" ] || fail "successful recovery left its private journal"
[ "$(wc -l < "$test_root/curl.log")" -eq 1 ] || fail "recovered cache caused unnecessary rule downloads"

# A journal restores files that did not exist before the transaction.
rm -f "$DATA_PATH/lazy.txt"
printf 'original video only\n' > "$DATA_PATH/video.txt"
recovery_stage=$(mktemp -d /tmp/adbyby-rules.XXXXXX)
cp -p "$DATA_PATH/video.txt" "$recovery_stage/video.previous"
( umask 077
  printf 'absent\n' > "$recovery_stage/lazy.state"
  printf 'present\n' > "$recovery_stage/video.state"
  printf '%s\n' "$recovery_stage" > "$recovery_stage/transaction.next"
)
mv -f "$recovery_stage/transaction.next" "$DATA_PATH/.rules-transaction"
printf 'uncommitted lazy file\n' > "$DATA_PATH/lazy.txt"
adbyby_recover_managed_rules || fail "absent-file recovery failed"
[ ! -e "$DATA_PATH/lazy.txt" ] || fail "recovery did not restore the previously absent lazy file"
[ "$(cat "$DATA_PATH/video.txt")" = "original video only" ] || fail "recovery did not restore the existing video file"
[ ! -e "$DATA_PATH/.rules-transaction" ] || fail "absent-file recovery left a journal"

# Symlinked markers are rejected before any live configuration is modified.
printf 'safe lazy snapshot\n' > "$DATA_PATH/lazy.txt"
printf 'safe video snapshot\n' > "$DATA_PATH/video.txt"
ln -s "$remote" "$DATA_PATH/.rules-transaction"
if adbyby_refresh_managed_rules; then fail "symlinked recovery marker was accepted"; fi
[ -L "$DATA_PATH/.rules-transaction" ] || fail "symlinked marker was replaced or followed"
[ "$(cat "$DATA_PATH/lazy.txt")" = "safe lazy snapshot" ] || fail "symlink marker changed lazy cache"
[ "$(cat "$DATA_PATH/video.txt")" = "safe video snapshot" ] || fail "symlink marker changed video cache"
rm -f "$DATA_PATH/.rules-transaction"

# Reject a journal path outside the dedicated private staging namespace.
( umask 077
  printf '%s\n' '/tmp/not-adbyby-rules.abcdef' > "$DATA_PATH/.rules-transaction"
)
if adbyby_refresh_managed_rules; then fail "invalid transaction stage path was accepted"; fi
[ -f "$DATA_PATH/.rules-transaction" ] || fail "invalid transaction journal was discarded"
[ "$(cat "$DATA_PATH/lazy.txt")" = "safe lazy snapshot" ] || fail "invalid journal changed lazy"
[ "$(cat "$DATA_PATH/video.txt")" = "safe video snapshot" ] || fail "invalid journal changed video"
rm -f "$DATA_PATH/.rules-transaction"

# A missing backup keeps the journal and last-good files intact for retry.
retry_stage=$(mktemp -d /tmp/adbyby-rules.XXXXXX)
cp -p "$DATA_PATH/video.txt" "$retry_stage/video.previous"
( umask 077
  printf '%s\n' present > "$retry_stage/lazy.state"
  printf '%s\n' present > "$retry_stage/video.state"
  printf '%s\n' "$retry_stage" > "$retry_stage/transaction.next"
)
mv -f "$retry_stage/transaction.next" "$DATA_PATH/.rules-transaction"
if adbyby_refresh_managed_rules; then fail "incomplete recovery used the cached rule pair"; fi
[ -f "$DATA_PATH/.rules-transaction" ] || fail "incomplete recovery journal was discarded"
[ "$(cat "$DATA_PATH/lazy.txt")" = "safe lazy snapshot" ] || fail "incomplete recovery changed the live lazy file"
[ "$(cat "$DATA_PATH/video.txt")" = "safe video snapshot" ] || fail "incomplete recovery changed the live video file"
cp -p "$DATA_PATH/lazy.txt" "$retry_stage/lazy.previous"
adbyby_recover_managed_rules || fail "repaired recovery journal retry failed"
[ "$(cat "$DATA_PATH/lazy.txt")" = "safe lazy snapshot" ] || fail "repaired recovery did not restore lazy"
[ "$(cat "$DATA_PATH/video.txt")" = "safe video snapshot" ] || fail "repaired recovery did not preserve video"
[ ! -e "$DATA_PATH/.rules-transaction" ] || fail "successful recovery retry left the marker"
[ ! -e "$retry_stage" ] || fail "successful recovery retry left its stage"

# Missing CA: no request is sent and a valid cache remains usable.
ADBYBY_CA_BUNDLE="$test_root/missing-ca"
: > "$test_root/curl.log"
adbyby_refresh_managed_rules || fail "missing CA should retain valid local rules"
[ "$(wc -l < "$test_root/curl.log")" -eq 0 ] || fail "curl called without a CA bundle"

# Missing CA plus absent cache is a hard failure, never an empty rule install.
rm -f "$DATA_PATH/lazy.txt" "$DATA_PATH/video.txt"
if adbyby_refresh_managed_rules; then
    fail "missing cache/CA failure was accepted"
fi

[ ! -s "$test_root/curl-errors" ] || fail "$(cat "$test_root/curl-errors")"
echo "Adbyby private HTTPS staging, digest validation, cache fallback and publication rollback: PASS"
