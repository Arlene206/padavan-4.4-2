#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
tmp_dir=$(mktemp -d "${TMPDIR:-/tmp}/wsdd2-llmnr-question.XXXXXX")
trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM

"${CC:-cc}" -std=gnu99 -Wall -Wextra -Werror \
	-fsanitize=address,undefined -fno-omit-frame-pointer \
	-I"$repo_root/trunk/user/wsdd2/src" \
	-o "$tmp_dir/llmnr-question-test" \
	"$repo_root/trunk/user/wsdd2/src/tests/llmnr_question_policy_test.c"
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1 \
	"$tmp_dir/llmnr-question-test"
python3 "$repo_root/scripts/check_wsdd2_llmnr_question.py"
