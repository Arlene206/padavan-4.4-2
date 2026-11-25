#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
TEST_TMP=$(mktemp -d "${TMPDIR:-/tmp}/scutclient-status.XXXXXX")
trap 'rm -rf "$TEST_TMP"' EXIT HUP INT TERM

WRITER="$ROOT/trunk/user/scutclient/scripts/scutclient_status_file.sh"
STATUS_FILE="$TEST_TMP/scutclient_status"
export SCUTCLIENT_STATUS_FILE="$STATUS_FILE"

run_writer()
{
	"$WRITER" "$1"
}

run_writer 0
[ "$(cat "$STATUS_FILE")" = 0 ]
[ "$(stat -c %a "$STATUS_FILE")" = 600 ]
[ "$(stat -c %h "$STATUS_FILE")" = 1 ]
run_writer 1
[ "$(cat "$STATUS_FILE")" = 1 ]

rm -f "$STATUS_FILE"
printf 'sentinel\n' > "$TEST_TMP/outside"
ln -s "$TEST_TMP/outside" "$STATUS_FILE"
run_writer 0
[ "$(cat "$TEST_TMP/outside")" = sentinel ]
[ ! -L "$STATUS_FILE" ] && [ "$(cat "$STATUS_FILE")" = 0 ]

rm -f "$STATUS_FILE"
printf 'hardlink-sentinel\n' > "$TEST_TMP/hardlink-outside"
ln "$TEST_TMP/hardlink-outside" "$STATUS_FILE"
run_writer 1
[ "$(cat "$TEST_TMP/hardlink-outside")" = hardlink-sentinel ]
[ "$(cat "$STATUS_FILE")" = 1 ]

rm -f "$STATUS_FILE"
mkdir "$STATUS_FILE"
if run_writer 0; then
	echo 'status writer accepted a directory destination' >&2
	exit 1
fi
[ -d "$STATUS_FILE" ]
[ -z "$(find "$STATUS_FILE" -mindepth 1 -maxdepth 1 -print -quit)" ]
rmdir "$STATUS_FILE"

if run_writer 2; then
	echo 'status writer accepted an invalid status value' >&2
	exit 1
fi
if SCUTCLIENT_STATUS_FILE=relative-status "$WRITER" 0; then
	echo 'status writer accepted a relative destination' >&2
	exit 1
fi
[ -z "$(find "$TEST_TMP" -maxdepth 1 -name 'scutclient_status.tmp.*' -print -quit)" ]

echo 'SCUTClient status hook atomic publication and symlink/hardlink safety: PASS'
