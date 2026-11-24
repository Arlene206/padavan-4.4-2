#!/bin/sh
# Publish SCUTClient auth state without truncating an attacker-controlled path.
status_value=${1-}
case "$status_value" in
	0|1) ;;
	*) exit 2 ;;
esac

status_file=${SCUTCLIENT_STATUS_FILE:-/tmp/scutclient_status}
case "$status_file" in
	/*) ;;
	*) exit 2 ;;
esac
case "$status_file" in
	*/) exit 2 ;;
esac

status_dir=${status_file%/*}
[ -n "$status_dir" ] || status_dir=/
[ -d "$status_dir" ] && [ ! -L "$status_dir" ] || exit 1
[ ! -d "$status_file" ] || exit 1

old_umask=$(umask) || exit 1
umask 077 || exit 1
tmp_file=$(mktemp "${status_file}.tmp.XXXXXX") || {
	umask "$old_umask"
	exit 1
}
scut_status_cleanup()
{
	[ -n "$tmp_file" ] && rm -f "$tmp_file"
	umask "$old_umask"
}
trap 'scut_status_cleanup' 0
trap 'exit 1' HUP INT TERM

if ! printf '%s\n' "$status_value" > "$tmp_file"; then
	exit 1
fi
if ! chmod 600 "$tmp_file"; then
	exit 1
fi
[ ! -d "$status_file" ] || exit 1
if ! mv -f "$tmp_file" "$status_file"; then
	exit 1
fi
tmp_file=
trap - 0 HUP INT TERM
umask "$old_umask"
[ -f "$status_file" ] && [ ! -L "$status_file" ] || exit 1
exit 0
