#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../../.." && pwd)
sandbox=$(mktemp -d "${TMPDIR:-/tmp}/reset-ss-template.XXXXXX")
real_rm=$(command -v rm)
real_mkdir=$(command -v mkdir)
trap '"$real_rm" -rf "$sandbox"' EXIT HUP INT TERM

fail()
{
	echo "reset_ss template regression: FAIL: $*" >&2
	exit 1
}

prepare_script()
{
	storage_dir=$1
	"$real_mkdir" -p "$storage_dir"
	sed "s|^script_dir=/etc/storage$|script_dir=$storage_dir|" \
		"$repo_root/trunk/user/scripts/files/sbin/reset_ss.sh" > "$sandbox/reset_ss.sh"
	sh -n "$sandbox/reset_ss.sh" || fail "reset_ss.sh has invalid shell syntax"
}

run_generator()
{
	if [ -n "${BUSYBOX_BIN:-}" ]; then
		"$BUSYBOX_BIN" ash "$sandbox/reset_ss.sh" generate
	else
		sh "$sandbox/reset_ss.sh" generate
	fi
}

# Keep runtime substitutions in the generated file.
prepare_script "$sandbox/storage"
device=must-not-be-expanded-now
export device
run_generator || fail "failed to generate start_script.sh"
generated="$sandbox/storage/start_script.sh"
[ -f "$generated" ] || fail "start_script.sh was not published"
[ ! -L "$generated" ] || fail "published start_script.sh is a symlink"
[ "$(stat -c %a "$generated")" = 755 ] || fail "published start_script.sh mode is not 755"
grep -Fqx 'for device in $(ls /sys/class/net); do' "$generated" ||
	fail "runtime interface enumeration was expanded during generation"
grep -Fqx 'echo f >/sys/class/net/$device/queues/rx-0/rps_cpus' "$generated" ||
	fail "runtime device variable was expanded during generation"
grep -Fqx 'case $1 in' "$generated" ||
	fail "runtime argument was expanded during generation"
grep -Fq '$(cat /sys/class/net/$device/queues/rx-0/rps_cpus)' "$generated" ||
	fail "runtime sysfs reads were expanded during generation"
if grep -Fq 'must-not-be-expanded-now' "$generated"; then
	fail "outer environment leaked into generated script"
fi
sh -n "$generated" || fail "generated start_script.sh has invalid shell syntax"

# Replacing a stale symlink must not write through to its target.
prepare_script "$sandbox/storage-symlink"
printf '%s\n' 'preserve symlink target' > "$sandbox/target"
ln -s "$sandbox/target" "$sandbox/storage-symlink/start_script.sh"
run_generator || fail "failed to replace stale symlink"
[ ! -L "$sandbox/storage-symlink/start_script.sh" ] ||
	fail "start_script.sh symlink was retained"
[ "$(cat "$sandbox/target")" = "preserve symlink target" ] ||
	fail "start script generation overwrote symlink target"

# A failed final rename leaves the previous startup script intact.
prepare_script "$sandbox/storage-mv-failure"
printf '%s\n' 'previous startup script' > "$sandbox/storage-mv-failure/start_script.sh"
"$real_mkdir" -p "$sandbox/bin"
cat > "$sandbox/bin/mv" <<'EOF_MV'
#!/bin/sh
exit 1
EOF_MV
chmod +x "$sandbox/bin/mv"
if PATH="$sandbox/bin:$PATH" sh "$sandbox/reset_ss.sh" generate; then
	fail "injected publication failure unexpectedly succeeded"
fi
[ "$(cat "$sandbox/storage-mv-failure/start_script.sh")" = "previous startup script" ] ||
	fail "failed publication modified the previous script"
for scratch in "$sandbox/storage-mv-failure"/.start_script.sh.*; do
	[ ! -e "$scratch" ] || fail "failed publication leaked staging file: $scratch"
done

echo "reset_ss deferred expansion and atomic publication regression: PASS"
