#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
tmp_dir=$(mktemp -d "${TMPDIR:-/tmp}/wsdd2-netlink.XXXXXX")
trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM

"${CC:-cc}" -std=gnu89 -Wall -Wextra -Werror \
	-fsanitize=address,undefined -fno-omit-frame-pointer \
	-I"$repo_root/trunk/user/wsdd2/src" \
	-o "$tmp_dir/wsd-netlink-recv-test" \
	"$repo_root/trunk/user/wsdd2/src/tests/wsd_netlink_recv_policy_test.c"
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1 \
	"$tmp_dir/wsd-netlink-recv-test"
python3 "$repo_root/scripts/check_wsdd2_netlink_recv.py"
