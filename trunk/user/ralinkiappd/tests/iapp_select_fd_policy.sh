#!/bin/sh
set -eu

test_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$test_dir/../../../.." && pwd)
src_dir="$repo_root/trunk/user/ralinkiappd/src"
tmp_dir=$(mktemp -d /tmp/iapp-select-fd.XXXXXX)
trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM

cc=cc
"$cc" -std=gnu89 -Wall -Wextra -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I "$src_dir" "$test_dir/iapp_select_fd_policy.c" \
  -o "$tmp_dir/iapp-select-fd-policy"
"$tmp_dir/iapp-select-fd-policy"
