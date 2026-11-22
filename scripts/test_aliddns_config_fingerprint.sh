#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
SOURCE="$ROOT/trunk/user/aliddns/aliddns.sh"
TEMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/aliddns-fingerprint.XXXXXX")
trap 'rm -rf "$TEMP_DIR"' EXIT HUP INT TERM
HELPER=$(sed -n '/^aliddns_config_fingerprint () {/,/^}/p' "$SOURCE")
[ -n "$HELPER" ] || { echo 'FAIL: missing production fingerprint helper' >&2; exit 1; }
eval "$HELPER"
TEST_SCRIPT="$TEMP_DIR/ddns_script.sh"
printf '%s\n' '# custom DDNS configuration' 'echo update' > "$TEST_SCRIPT"
aliddns_enable=1
aliddns_interval=600
aliddns_ak=access-key
aliddns_sk=secret-key
aliddns_domain=example.com
aliddns_name=ab
aliddns_domain2=example.net
aliddns_name2=www
aliddns_domain6=example.org
aliddns_name6=ipv6
aliddns_ttl=600
first=$(aliddns_config_fingerprint "$TEST_SCRIPT")
again=$(aliddns_config_fingerprint "$TEST_SCRIPT")
[ "$first" = "$again" ] || { echo 'FAIL: stable settings produced different fingerprints' >&2; exit 1; }

# These two adjacent-field tuples collided under the former plain concatenation:
# example.com + ab == example.coma + b.
aliddns_domain=example.coma
aliddns_name=b
second=$(aliddns_config_fingerprint "$TEST_SCRIPT")
[ "$first" != "$second" ] || { echo 'FAIL: adjacent NVRAM fields are not delimited' >&2; exit 1; }

printf '%s\n' '# changed config' >> "$TEST_SCRIPT"
third=$(aliddns_config_fingerprint "$TEST_SCRIPT")
[ "$second" != "$third" ] || { echo 'FAIL: custom script content is not fingerprinted' >&2; exit 1; }

ln -s "$TEST_SCRIPT" "$TEMP_DIR/script-link.sh"
if aliddns_config_fingerprint "$TEMP_DIR/script-link.sh"; then
  echo 'FAIL: symlink custom script was accepted' >&2
  exit 1
fi
if aliddns_config_fingerprint "$TEMP_DIR/missing.sh"; then
  echo 'FAIL: missing custom script was accepted' >&2
  exit 1
fi
echo 'AliDDNS unambiguous configuration fingerprint regression: PASS'
