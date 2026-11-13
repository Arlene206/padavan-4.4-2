#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../../.." && pwd)
source_file="$repo_root/trunk/user/adbyby/adbyby.sh"
test_root=$(mktemp -d /tmp/adbyby-hosts-config.XXXXXX)
trap 'rm -rf "$test_root"' EXIT HUP INT TERM

# Extract the production function verbatim; do not duplicate its sed policy.
policy_file="$test_root/hosts-policy.sh"
awk '
    /^adbyby_remove_generated_hosts_config\(\)$/ { capture = 1 }
    capture { print }
    capture && /^}/ { exit }
' "$source_file" > "$policy_file"
[ -s "$policy_file" ] || {
    echo "could not extract production Adbyby hosts cleanup helper" >&2
    exit 1
}

PROG_PATH=/tmp/adbyby
. "$policy_file"

config="$test_root/dnsmasq.conf"
expected="$test_root/expected.conf"
cat > "$config" <<'CONFIG'
# Operator comment mentioning hosts must remain.
addn-hosts=/etc/storage/custom-hosts
addn-hosts=/tmp/adbyby/hosts
addn-hosts=/tmp/adbyby/hosts
addn-hosts=/tmp/adbyby/hosts-backup
addn-hosts=/mnt/data/hosts
hostsdir=/etc/dnsmasq/hosts
conf-file=/etc/storage/dnsmasq.d/hosts.conf
address=/hosts.example/192.0.2.10
server=192.0.2.1
CONFIG
cat > "$expected" <<'EXPECTED'
# Operator comment mentioning hosts must remain.
addn-hosts=/etc/storage/custom-hosts
addn-hosts=/tmp/adbyby/hosts-backup
addn-hosts=/mnt/data/hosts
hostsdir=/etc/dnsmasq/hosts
conf-file=/etc/storage/dnsmasq.d/hosts.conf
address=/hosts.example/192.0.2.10
server=192.0.2.1
EXPECTED

adbyby_remove_generated_hosts_config "$config"
cmp "$expected" "$config"
# A second removal is idempotent and preserves every unrelated line.
adbyby_remove_generated_hosts_config "$config"
cmp "$expected" "$config"

# Metacharacters in the configured file name must be treated literally.
special_config="$test_root/special.conf"
special_hosts="$test_root/a[bc].$*/hosts|owned"
printf 'addn-hosts=%s\naddn-hosts=%s\n' \
    "$special_hosts" "$test_root/axbcXy/hosts|owned" > "$special_config"
adbyby_remove_generated_hosts_config "$special_config" "$special_hosts"
[ "$(wc -l < "$special_config")" -eq 1 ]
[ "$(cat "$special_config")" = "addn-hosts=$test_root/axbcXy/hosts|owned" ]

# An absent dnsmasq config is a harmless no-op.
adbyby_remove_generated_hosts_config "$test_root/missing.conf" "$special_hosts"
[ ! -e "$test_root/missing.conf" ]

echo "Adbyby exact managed addn-hosts cleanup and custom dnsmasq preservation: PASS"
