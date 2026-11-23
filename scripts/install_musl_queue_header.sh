#!/bin/sh
set -eu

if [ "$#" -ne 2 ] || [ -z "$1" ] || [ -z "$2" ]; then
	echo "Usage: $0 <header-url> <destination>" >&2
	exit 2
fi

url=$1
target=$2
case "$target" in
	*/*) target_dir=${target%/*}; [ -n "$target_dir" ] || target_dir=/ ;;
	*) target_dir=. ;;
esac
if [ ! -d "$target_dir" ]; then
	echo "queue.h destination directory does not exist: $target_dir" >&2
	exit 1
fi

tmp=$(mktemp "$target.tmp.XXXXXX") || {
	echo "cannot create temporary queue.h beside $target" >&2
	exit 1
}
cleanup()
{
	rm -f "$tmp"
}
trap cleanup 0
trap 'exit 1' HUP INT TERM

if curl -fL --retry 5 --retry-all-errors --retry-delay 1 \
	--retry-max-time 20 --connect-timeout 15 --max-time 60 \
	--progress-bar "$url" -o "$tmp"; then
	:
else
	status=$?
	echo "queue.h download failed (curl exit $status; URL: $url)" >&2
	exit "$status"
fi

if [ ! -s "$tmp" ]; then
	echo "queue.h download returned an empty file (URL: $url)" >&2
	exit 1
fi
if ! chmod 0644 "$tmp"; then
	echo "cannot set permissions on downloaded queue.h" >&2
	exit 1
fi

# Publish only a complete, non-empty file; the rename is atomic on this filesystem.
mv -f "$tmp" "$target"
