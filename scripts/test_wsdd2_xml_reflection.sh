#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
tmp_dir=$(mktemp -d "${TMPDIR:-/tmp}/wsdd2-xml-reflection.XXXXXX")
trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM

"${CC:-cc}" -std=gnu99 -Wall -Wextra -Werror \
	-fsanitize=address,undefined -fno-omit-frame-pointer \
	-I"$repo_root/trunk/user/wsdd2/src" \
	-o "$tmp_dir/wsd-xml-text-test" \
	"$repo_root/trunk/user/wsdd2/src/tests/wsd_xml_text_policy_test.c"
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1 \
	"$tmp_dir/wsd-xml-text-test"
python3 "$repo_root/scripts/check_wsdd2_xml_reflection.py"
