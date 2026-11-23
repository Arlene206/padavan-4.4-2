#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
SOURCE="$ROOT/trunk/user/scripts/files/usr/bin/ap_script.sh"
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/ap-script-lock-test.XXXXXX")
trap 'rm -rf "$TMP_DIR"' EXIT HUP INT TERM
[ -r "$SOURCE" ] || { echo "AP monitor source missing" >&2; exit 1; }
SNIPPET=$(awk '
 /^ap_script_num=/ { if (found != 0) exit 1; found=1; capture=1 }
 capture { print }
 capture && /^fi$/ { complete=1; exit }
 END { if (found != 1 || complete != 1) exit 1 }
' "$SOURCE") || { echo "could not extract one complete AP monitor gate" >&2; exit 1; }
EXPECTED='if [[ $ap_script_num == "0" && $sh_ezscript_num == "0" ]]; then'
printf '%s\n' "$SNIPPET" | grep -Fqx "$EXPECTED" || {
 echo "AP lock gate must independently check both process counts" >&2; exit 1;
}
[ "$(printf '%s\n' "$SNIPPET" | grep -Ec '^[[:space:]]*rm -f /tmp/apc\.lock[[:space:]]*$')" -eq 1 ] || {
 echo "AP lock removal must remain a single guarded operation" >&2; exit 1;
}
SNIPPET=$(printf '%s\n' "$SNIPPET" | sed 's|rm -f /tmp/apc.lock|rm -f "$APC_LOCK_FILE"|')
run_case()
{
 state=$1; expected=$2; lock_file="$TMP_DIR/$state.lock"; : > "$lock_file"
 APC_LOCK_FILE="$lock_file" MOCK_PS_STATE="$state" AP_MONITOR_LOCK_SNIPPET="$SNIPPET" bash -c '
  ps()
  {
   case "$MOCK_PS_STATE" in
    script_only) printf "%s\n" "123 ap_script.sh" ;;
    helper_only) printf "%s\n" "124 sh_ezscript.sh connAPSite_scan" ;;
    both) printf "%s\n" "123 ap_script.sh"; printf "%s\n" "124 sh_ezscript.sh connAPSite_scan" ;;
    none) : ;;
    *) return 2 ;;
   esac
  }
  eval "$AP_MONITOR_LOCK_SNIPPET"
 ' || { echo "AP lock gate failed for state: $state" >&2; exit 1; }
 case "$expected" in
  keep) [ -f "$lock_file" ] || { echo "lock removed while process remained: $state" >&2; exit 1; } ;;
  remove) [ ! -e "$lock_file" ] || { echo "lock retained after both processes exited" >&2; exit 1; } ;;
  *) exit 1 ;;
 esac
}
run_case script_only keep
run_case helper_only keep
run_case both keep
run_case none remove
printf '%s\n' "AP relay lock cleanup process-presence regression: PASS"
