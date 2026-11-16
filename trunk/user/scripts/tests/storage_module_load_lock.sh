#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
automount="$repo_root/trunk/user/scripts/files/sbin/automount.sh"
tmpdir=$(mktemp -d /tmp/padavan-module-load-lock.XXXXXX)
trap 'rm -rf "$tmpdir"' 0 HUP INT TERM
mkdir -p "$tmpdir/bin"

sed -n '/^func_load_module()/,/^}/p' "$automount" > "$tmpdir/module-function.sh"
[ -s "$tmpdir/module-function.sh" ]
. "$tmpdir/module-function.sh"

cat > "$tmpdir/bin/flock" <<'MOCK'
#!/bin/sh
printf 'flock %s\n' "$*" >> "$MOCK_CALLS"
if [ "$MOCK_RACE_LOAD" = 1 ]; then
	printf '%s 1 0 - Live 0x0\n' "$MOCK_RACE_MODULE" >> "$MOCK_MODULES_FILE"
fi
exit "$MOCK_FLOCK_STATUS"
MOCK
cat > "$tmpdir/bin/modprobe" <<'MOCK'
#!/bin/sh
printf 'modprobe %s\n' "$*" >> "$MOCK_CALLS"
exit "$MOCK_MODPROBE_STATUS"
MOCK
cat > "$tmpdir/bin/usleep" <<'MOCK'
#!/bin/sh
printf 'usleep %s\n' "$*" >> "$MOCK_CALLS"
exit 0
MOCK
cat > "$tmpdir/bin/logger" <<'MOCK'
#!/bin/sh
printf 'logger %s\n' "$*" >> "$MOCK_LOG"
exit 0
MOCK
chmod +x "$tmpdir/bin/"*
PATH="$tmpdir/bin:$PATH"
export PATH
MOCK_CALLS="$tmpdir/calls"
MOCK_LOG="$tmpdir/log"
MOCK_MODULES_FILE="$tmpdir/modules"
MOCK_FLOCK_STATUS=0
MOCK_MODPROBE_STATUS=0
MOCK_RACE_LOAD=0
MOCK_RACE_MODULE=
export MOCK_CALLS MOCK_LOG MOCK_MODULES_FILE
export MOCK_FLOCK_STATUS MOCK_MODPROBE_STATUS MOCK_RACE_LOAD MOCK_RACE_MODULE
lock_file="$tmpdir/fs_module.lock"

# An already-loaded module needs neither the lock nor a second modprobe.
: > "$MOCK_CALLS"
: > "$MOCK_LOG"
printf 'ext4 123 1 - Live 0x0\n' > "$MOCK_MODULES_FILE"
MOCK_FLOCK_STATUS=19
export MOCK_FLOCK_STATUS
func_load_module ext4 "$MOCK_MODULES_FILE" "$lock_file"
[ ! -s "$MOCK_CALLS" ]

# A failed lock must stop before modprobe.
: > "$MOCK_CALLS"
: > "$MOCK_MODULES_FILE"
MOCK_FLOCK_STATUS=23
MOCK_RACE_LOAD=0
export MOCK_FLOCK_STATUS MOCK_RACE_LOAD
if func_load_module ext4 "$MOCK_MODULES_FILE" "$lock_file"; then
	echo "module loader ignored a failed lock" >&2
	exit 1
fi
grep -qx 'flock 9' "$MOCK_CALLS"
if grep -q '^modprobe ' "$MOCK_CALLS"; then
	echo "module loader ran modprobe without its lock" >&2
	exit 1
fi

# Successful load has strict lock -> modprobe -> settle ordering.
: > "$MOCK_CALLS"
MOCK_FLOCK_STATUS=0
MOCK_MODPROBE_STATUS=0
export MOCK_FLOCK_STATUS MOCK_MODPROBE_STATUS
func_load_module ext4 "$MOCK_MODULES_FILE" "$lock_file"
printf 'flock 9\nmodprobe -q ext4\nusleep 300000\n' > "$tmpdir/expected"
cmp "$tmpdir/expected" "$MOCK_CALLS"

# A competing worker may load the module while this process waits for the lock.
: > "$MOCK_CALLS"
: > "$MOCK_MODULES_FILE"
MOCK_RACE_LOAD=1
MOCK_RACE_MODULE=xfs
export MOCK_RACE_LOAD MOCK_RACE_MODULE
func_load_module xfs "$MOCK_MODULES_FILE" "$lock_file"
grep -qx 'flock 9' "$MOCK_CALLS"
if grep -q '^modprobe ' "$MOCK_CALLS"; then
	echo "module loader did not recheck after acquiring the lock" >&2
	exit 1
fi

# A real modprobe failure must propagate and must not be followed by a delay.
: > "$MOCK_CALLS"
: > "$MOCK_MODULES_FILE"
MOCK_RACE_LOAD=0
MOCK_MODPROBE_STATUS=7
export MOCK_RACE_LOAD MOCK_MODPROBE_STATUS
if func_load_module ntfs "$MOCK_MODULES_FILE" "$lock_file"; then
	echo "module loader hid a modprobe failure" >&2
	exit 1
fi
if grep -q '^usleep ' "$MOCK_CALLS"; then
	echo "module loader delayed after failed modprobe" >&2
	exit 1
fi

# Invalid names must not reach any external command.
: > "$MOCK_CALLS"
if func_load_module ../ext4 "$MOCK_MODULES_FILE" "$lock_file"; then
	echo "module loader accepted an unsafe module name" >&2
	exit 1
fi
[ ! -s "$MOCK_CALLS" ]

# An unreadable module inventory is UNKNOWN, not evidence that the module is absent.
: > "$MOCK_CALLS"
mkdir "$tmpdir/not-a-module-file"
if func_load_module ext4 "$tmpdir/not-a-module-file" "$lock_file"; then
	echo "module loader treated an inventory error as module absence" >&2
	exit 1
fi
[ ! -s "$MOCK_CALLS" ]

echo "Filesystem module loader lock/recheck policy: PASS"
