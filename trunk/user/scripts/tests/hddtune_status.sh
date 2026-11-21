#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../../.." && pwd)
tmp_dir=$(mktemp -d "${TMPDIR:-/tmp}/hddtune-status.XXXXXX")
trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM

mkdir -p "$tmp_dir/bin" "$tmp_dir/sys/block/sda" "$tmp_dir/sys/block/sdb" "$tmp_dir/sys/block/sdc"
printf '0\n' > "$tmp_dir/sys/block/sda/removable"
printf '0\n' > "$tmp_dir/sys/block/sdb/removable"
printf '1\n' > "$tmp_dir/sys/block/sdc/removable"

# Rewrite only fixed system paths in a temporary test copy.
sed -e "s|/sbin/hdparm|$tmp_dir/bin/hdparm|g" \
    -e "s|/sys/block|$tmp_dir/sys/block|g" \
    "$repo_root/trunk/user/scripts/files/sbin/hddtune.sh" > "$tmp_dir/hddtune.sh"

cat > "$tmp_dir/bin/nvram" <<'EOF_NVRAM'
#!/bin/sh
case "$2" in
    hdd_spindt) printf '%s\n' "${HDD_SPINDT:-0}" ;;
    hdd_apmoff) printf '%s\n' "${HDD_APMOFF:-0}" ;;
    *) exit 1 ;;
esac
EOF_NVRAM
cat > "$tmp_dir/bin/logger" <<'EOF_LOGGER'
#!/bin/sh
exit 0
EOF_LOGGER
cat > "$tmp_dir/bin/hdparm" <<'EOF_HDPARM'
#!/bin/sh
printf '%s\n' "$*" >> "$HDPARM_LOG"
if [ -n "${HDPARM_FAIL_OPTION:-}" ]; then
    case " $* " in *" $HDPARM_FAIL_OPTION "*) exit 7 ;; esac
fi
if [ -n "${HDPARM_FAIL_DEVICE:-}" ]; then
    case " $* " in *" $HDPARM_FAIL_DEVICE "*) exit 8 ;; esac
fi
exit 0
EOF_HDPARM
chmod +x "$tmp_dir/bin/nvram" "$tmp_dir/bin/logger" "$tmp_dir/bin/hdparm" "$tmp_dir/hddtune.sh"

call_hddtune()
{
    env PATH="$tmp_dir/bin:$PATH" \
        HDPARM_LOG="$tmp_dir/hdparm.log" \
        HDD_SPINDT="${HDD_SPINDT:-0}" HDD_APMOFF="${HDD_APMOFF:-0}" \
        HDPARM_FAIL_OPTION="${HDPARM_FAIL_OPTION:-}" \
        HDPARM_FAIL_DEVICE="${HDPARM_FAIL_DEVICE:-}" \
        sh "$tmp_dir/hddtune.sh" "$@"
}

line_count()
{
    wc -l < "$tmp_dir/hdparm.log" | tr -d ' '
}

: > "$tmp_dir/hdparm.log"
if ! HDD_SPINDT=1 HDD_APMOFF=1 call_hddtune sda; then
    echo "successful target tuning unexpectedly failed" >&2
    exit 1
fi
[ "$(line_count)" -eq 2 ]
grep -F -- '-S180 /dev/sda' "$tmp_dir/hdparm.log" >/dev/null
grep -F -- '-B254 /dev/sda' "$tmp_dir/hdparm.log" >/dev/null

# A failed first setting must not suppress the independent APM operation.
: > "$tmp_dir/hdparm.log"
if HDD_SPINDT=1 HDD_APMOFF=1 HDPARM_FAIL_OPTION=-S180 call_hddtune sda; then
    echo "spindown failure was incorrectly reported as success" >&2
    exit 1
fi
[ "$(line_count)" -eq 2 ]
grep -F -- '-B254 /dev/sda' "$tmp_dir/hdparm.log" >/dev/null

: > "$tmp_dir/hdparm.log"
if HDD_SPINDT=1 HDD_APMOFF=1 HDPARM_FAIL_OPTION=-B254 call_hddtune sda; then
    echo "APM failure was incorrectly reported as success" >&2
    exit 1
fi
[ "$(line_count)" -eq 2 ]

# Global mode continues with later fixed disks after the first one fails.
: > "$tmp_dir/hdparm.log"
if HDD_SPINDT=1 HDPARM_FAIL_DEVICE=/dev/sda call_hddtune; then
    echo "global mode swallowed a drive failure" >&2
    exit 1
fi
[ "$(line_count)" -eq 2 ]
grep -F -- '/dev/sdb' "$tmp_dir/hdparm.log" >/dev/null

# Removable media is skipped; unsafe names and missing hdparm fail.
: > "$tmp_dir/hdparm.log"
if ! call_hddtune sdc; then
    echo "removable media skip unexpectedly failed" >&2
    exit 1
fi
[ "$(line_count)" -eq 0 ]
if call_hddtune ../sda; then
    echo "unsafe device name was accepted" >&2
    exit 1
fi
mv "$tmp_dir/bin/hdparm" "$tmp_dir/bin/hdparm.disabled"
if call_hddtune sda; then
    echo "missing hdparm executable was incorrectly accepted" >&2
    exit 1
fi

echo "hddtune command failure aggregation regression: PASS"
