#!/bin/sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$HERE/../smartdns_config_file.sh"
T=$(mktemp -d "${TMPDIR:-/tmp}/smartdns-config-file.XXXXXX")
trap 'rm -rf "$T"' 0
trap 'exit 1' HUP INT TERM
fail() { echo "SmartDNS atomic config regression: FAIL: $*" >&2; exit 1; }
pass() { echo "SmartDNS atomic config regression: PASS: $*"; }
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
