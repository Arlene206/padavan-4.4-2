#!/bin/sh
# Host regression for FRP's private runtime directory and config publication.
set -eu
repo_root=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
test_root=$(mktemp -d /tmp/frp-config-test.XXXXXX)
trap 'rm -rf "$test_root"' EXIT HUP INT TERM

# Import only the production helpers. Do not execute the ROMFS script's
# NVRAM lookups, killall, or daemon launch in this host-only test.
tr -d '\r' < "$repo_root/trunk/user/frp/frp_script.sh" |
	sed -n \
	-e '/^frp_validate_credentials()$/,/^}/p' \
	-e '/^frp_prepare_runtime_dir()$/,/^}/p' \
	-e '/^frp_validate_config_target()$/,/^}/p' \
	-e '/^frp_stage_client_config()$/,/^}/p' \
	-e '/^frp_stage_server_config()$/,/^}/p' \
	-e '/^frp_publish_staged_config()$/,/^}/p' \
	-e '/^frp_rollback_config_pair()$/,/^}/p' \
	-e '/^frp_publish_enabled_configs()$/,/^}/p' \
	-e '/^frp_cleanup_config_stages()$/,/^}/p' > "$test_root/helpers.sh"
. "$test_root/helpers.sh"

fail() { echo "FRP config regression: FAIL: $*" >&2; exit 1; }
expect_mode() {
	actual=$(stat -c '%a' "$1") || fail "cannot stat $1"
	[ "$actual" = "$2" ] || fail "$1 mode is $actual, expected $2"
}

frpc_enable=1
frps_enable=1
frp_token='0123456789abcdef0123456789abcdef'
frp_dashboard_user='frp_operator'
frp_dashboard_password='A9b2C4d6E8f0G1h3J5k7'
frp_client_tmp=
frp_server_tmp=
frp_client_backup=
frp_server_backup=
frp_client_had_original=0
frp_server_had_original=0
frp_client_published=0
frp_server_published=0
frp_preserve_config_backups=0
frp_runtime_dir="$test_root/runtime"

# Reject the example/default credentials before creating any config.
frp_token='CHANGE_ME_TO_A_UNIQUE_RANDOM_TOKEN'
if frp_validate_credentials; then fail "example token was accepted"; fi
frp_token='0123456789abcdef0123456789abcdef'
frp_dashboard_user='admin'
if frp_validate_credentials; then fail "default Dashboard user was accepted"; fi
frp_dashboard_user='frp_operator'
frp_dashboard_password='CHANGE_ME_TO_A_UNIQUE_PASSWORD'
if frp_validate_credentials; then fail "example Dashboard password was accepted"; fi
frp_dashboard_password='A9b2C4d6E8f0G1h3J5k7'
frp_validate_credentials || fail "valid strong credentials were rejected"

# Directory must be real, root-owned in production, and private before staging.
mkdir "$frp_runtime_dir"
chmod 0777 "$frp_runtime_dir"
frp_prepare_runtime_dir "$frp_runtime_dir" "$(id -u)" ||
	fail "owned runtime directory was rejected"
expect_mode "$frp_runtime_dir" 700

mkdir "$test_root/real-dir"
ln -s "$test_root/real-dir" "$test_root/link-dir"
if frp_prepare_runtime_dir "$test_root/link-dir" "$(id -u)"; then
	fail "symlink runtime directory was accepted"
fi
[ -d "$test_root/real-dir" ] || fail "symlink referent was damaged"

# The numeric owner field is checked rather than assuming every existing
# directory under /tmp is safe just because it is a directory.
ls() {
	case "$2" in
		/tmp) printf 'drwxrwxrwt 12 0 0 4096 Jan 1 00:00 /tmp\n' ;;
		*) printf 'drwx------ 2 424242 424242 4096 Jan 1 00:00 %s\n' "$2" ;;
	esac
}
mkdir "$test_root/wrong-owner"
if frp_prepare_runtime_dir "$test_root/wrong-owner" "$(id -u)"; then
	fail "non-owner runtime directory was accepted"
fi
unset -f ls

# Stage both enabled configs before publishing either one. Each stage is 0600.
frp_prepare_runtime_dir "$frp_runtime_dir" "$(id -u)" ||
	fail "runtime directory was not reusable"
frp_stage_client_config || fail "client config staging failed"
frp_stage_server_config || fail "server config staging failed"
expect_mode "$frp_client_tmp" 600
expect_mode "$frp_server_tmp" 600
grep -Fqx "token = $frp_token" "$frp_client_tmp" ||
	fail "client token was not substituted into the staged config"
grep -Fqx "dashboard_user = $frp_dashboard_user" "$frp_server_tmp" ||
	fail "Dashboard user was not substituted into the staged config"
grep -Fqx "dashboard_pwd = $frp_dashboard_password" "$frp_server_tmp" ||
	fail "Dashboard password was not substituted into the staged config"

# Same-directory rename replaces each complete config and keeps mode 0600.
printf 'old client config\n' > "$frp_runtime_dir/myfrpc.ini"
printf 'old server config\n' > "$frp_runtime_dir/myfrps.ini"
frp_publish_staged_config "$frp_client_tmp" "$frp_runtime_dir/myfrpc.ini" ||
	fail "client config publish failed"
frp_client_tmp=
frp_publish_staged_config "$frp_server_tmp" "$frp_runtime_dir/myfrps.ini" ||
	fail "server config publish failed"
frp_server_tmp=
expect_mode "$frp_runtime_dir/myfrpc.ini" 600
expect_mode "$frp_runtime_dir/myfrps.ini" 600
grep -Fqx "token = $frp_token" "$frp_runtime_dir/myfrpc.ini" ||
	fail "published client config is incomplete"
grep -Fqx "dashboard_pwd = $frp_dashboard_password" "$frp_runtime_dir/myfrps.ini" ||
	fail "published server config is incomplete"

# A staging failure must preserve the last-good config and leave no temp file.
printf 'last-good client config\n' > "$frp_runtime_dir/myfrpc.ini"
mktemp() { return 1; }
if frp_stage_client_config; then fail "mktemp failure was ignored"; fi
unset -f mktemp
[ "$(cat "$frp_runtime_dir/myfrpc.ini")" = "last-good client config" ] ||
	fail "failed mktemp damaged last-good client config"
[ -z "$frp_client_tmp" ] || fail "failed mktemp retained a temporary path"

# A chmod failure discards its stage and leaves the live config untouched.
chmod() { return 1; }
if frp_stage_client_config; then fail "staged config chmod failure was ignored"; fi
unset -f chmod
[ "$(cat "$frp_runtime_dir/myfrpc.ini")" = "last-good client config" ] ||
	fail "chmod failure damaged last-good client config"
[ -z "$frp_client_tmp" ] || fail "chmod failure retained a temporary path"

# A failed final rename preserves the existing live file.
frp_stage_client_config || fail "rename-failure fixture staging failed"
mv() { return 1; }
if frp_publish_staged_config "$frp_client_tmp" "$frp_runtime_dir/myfrpc.ini"; then
	fail "rename failure was reported as success"
fi
unset -f mv
frp_cleanup_config_stages
frp_client_tmp=
[ "$(cat "$frp_runtime_dir/myfrpc.ini")" = "last-good client config" ] ||
	fail "failed rename damaged last-good client config"

# A failure of the second publish rename must restore both last-good files.
printf 'last-good client pair\n' > "$frp_runtime_dir/myfrpc.ini"
printf 'last-good server pair\n' > "$frp_runtime_dir/myfrps.ini"
frp_client_tmp=
frp_server_tmp=
frp_stage_client_config || fail "paired client staging failed"
frp_stage_server_config || fail "paired server staging failed"
MV_CALLS=0
mv()
{
	MV_CALLS=$((MV_CALLS + 1))
	if [ "$MV_CALLS" -eq 4 ]; then
		return 1
	fi
	command mv "$@"
}
if frp_publish_enabled_configs; then
	fail "second config rename failure was reported as success"
fi
unset -f mv
[ "$(cat "$frp_runtime_dir/myfrpc.ini")" = "last-good client pair" ] ||
	fail "second rename failure did not restore the client config"
[ "$(cat "$frp_runtime_dir/myfrps.ini")" = "last-good server pair" ] ||
	fail "second rename failure did not restore the server config"
[ "$frp_preserve_config_backups" = "0" ] ||
	fail "successful rollback incorrectly retained recovery backups"

# When neither old file existed, failed publication must remove the new file.
rm -f "$frp_runtime_dir/myfrpc.ini" "$frp_runtime_dir/myfrps.ini"
frp_stage_client_config || fail "absent-pair client staging failed"
frp_stage_server_config || fail "absent-pair server staging failed"
MV_CALLS=0
mv()
{
	MV_CALLS=$((MV_CALLS + 1))
	if [ "$MV_CALLS" -eq 2 ]; then
		return 1
	fi
	command mv "$@"
}
if frp_publish_enabled_configs; then
	fail "second rename failure for absent pair was reported as success"
fi
unset -f mv
[ ! -e "$frp_runtime_dir/myfrpc.ini" ] ||
	fail "rollback left a newly-created client config"
[ ! -e "$frp_runtime_dir/myfrps.ini" ] ||
	fail "rollback left a newly-created server config"

# Refuse a symlink or non-regular live config target without touching its referent.
rm -f "$frp_runtime_dir/myfrpc.ini"
printf 'untouched referent\n' > "$test_root/referent"
ln -s "$test_root/referent" "$frp_runtime_dir/myfrpc.ini"
if frp_validate_config_target "$frp_runtime_dir/myfrpc.ini"; then
	fail "symlink config target was accepted"
fi
[ "$(cat "$test_root/referent")" = "untouched referent" ] ||
	fail "symlink target referent was modified"
rm -f "$frp_runtime_dir/myfrpc.ini"
mkdir "$frp_runtime_dir/myfrpc.ini"
if frp_validate_config_target "$frp_runtime_dir/myfrpc.ini"; then
	fail "directory config target was accepted"
fi

frp_cleanup_config_stages
echo "FRP private runtime directory and credential config publication: PASS"
echo "FRP paired second-rename failure restores existing last-good configs: PASS"
echo "FRP paired second-rename failure removes originally absent configs: PASS"
echo "FRP failed staging/rename preserves last-good config: PASS"
echo "FRP rejects symlink and non-regular runtime paths: PASS"
