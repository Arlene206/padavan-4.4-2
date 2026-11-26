#!/bin/sh
set -eu

test_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
makefile="$test_dir/../src/Makefile"
tmp_dir=$(mktemp -d /tmp/iappd-make-deps.XXXXXX)
trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM

cc=cc
make=make

cat >"$tmp_dir/make_dependency_probe.h" <<'EOF'
#define IAPP_DEPENDENCY_PROBE 0
EOF
cat >"$tmp_dir/rtmpiapp.c" <<'EOF'
#include "make_dependency_probe.h"
int main(void)
{
	return IAPP_DEPENDENCY_PROBE;
}
EOF
cat >"$tmp_dir/mt_iapp_util.c" <<'EOF'
int mt_iapp_util(void)
{
	return 0;
}
EOF

"$make" -C "$tmp_dir" -f "$makefile" CC="$cc" >"$tmp_dir/first.log" 2>&1
test -s "$tmp_dir/rtmpiapp.d"
test -s "$tmp_dir/mt_iapp_util.d"
"$tmp_dir/ralinkiappd"

"$make" -C "$tmp_dir" -f "$makefile" CC="$cc" >"$tmp_dir/noop.log" 2>&1
if grep -q 'rtmpiapp.c' "$tmp_dir/noop.log"; then
	echo "FAIL: unchanged source was rebuilt"
	exit 1
fi
if grep -q ' -o ralinkiappd ' "$tmp_dir/noop.log"; then
	echo "FAIL: unchanged binary was relinked"
	exit 1
fi

sleep 1
cat >"$tmp_dir/make_dependency_probe.h" <<'EOF'
#define IAPP_DEPENDENCY_PROBE 1
EOF
"$make" -C "$tmp_dir" -f "$makefile" CC="$cc" >"$tmp_dir/rebuild.log" 2>&1
if ! grep -q 'rtmpiapp.c' "$tmp_dir/rebuild.log"; then
	echo "FAIL: header change did not rebuild rtmpiapp.o"
	cat "$tmp_dir/rebuild.log"
	exit 1
fi
if ! grep -q ' -o ralinkiappd ' "$tmp_dir/rebuild.log"; then
	echo "FAIL: header change did not relink ralinkiappd"
	cat "$tmp_dir/rebuild.log"
	exit 1
fi
if "$tmp_dir/ralinkiappd"; then
	echo "FAIL: relinked binary did not observe the changed header"
	exit 1
else
	status=$?
	test "$status" -eq 1
fi

"$make" -C "$tmp_dir" -f "$makefile" clean CC="$cc" >"$tmp_dir/clean.log" 2>&1
test ! -e "$tmp_dir/rtmpiapp.d"
test ! -e "$tmp_dir/mt_iapp_util.d"
test ! -e "$tmp_dir/rtmpiapp.o"
test ! -e "$tmp_dir/ralinkiappd"

echo "Ralink IAPP automatic header dependency regression: PASS"
