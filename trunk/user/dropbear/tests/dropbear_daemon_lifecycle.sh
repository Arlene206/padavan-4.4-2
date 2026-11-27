#!/bin/sh
set -eu
TEST_ROOT=$(mktemp -d /tmp/dropbear-lifecycle.XXXXXX)
trap 'rm -rf "$TEST_ROOT"' EXIT HUP INT TERM
TEST_PROC_ROOT="$TEST_ROOT/proc"
TEST_BIN="$TEST_ROOT/dropbear"
TEST_FUNCTIONS="$TEST_ROOT/functions.sh"
TEST_SIGNALS="$TEST_ROOT/signals"
mkdir -p "$TEST_PROC_ROOT"
: > "$TEST_BIN"; chmod 755 "$TEST_BIN"; : > "$TEST_SIGNALS"
DROPBEAR_PROC_ROOT="$TEST_PROC_ROOT"
DROPBEAR_BIN="$TEST_BIN"
DROPBEAR_STORAGE_DIR="$TEST_ROOT/storage"
DROPBEAR_KEYGEN_BIN="$TEST_ROOT/dropbearkey"
export DROPBEAR_PROC_ROOT DROPBEAR_BIN DROPBEAR_STORAGE_DIR DROPBEAR_KEYGEN_BIN
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SOURCE="$SCRIPT_DIR/../sshd.sh"
sed '/^action_status=0$/,$d' "$SOURCE" > "$TEST_FUNCTIONS"
. "$TEST_FUNCTIONS"
logger() { :; }
func_log() { :; }
sleep() { :; }
TEST_TERM_STUCK=0; TEST_SIGNAL_FAILURE=0; TEST_KILLALL_CALLS=0
killall() { TEST_KILLALL_CALLS=$((TEST_KILLALL_CALLS + 1)); return 99; }
TEST_KEYGEN_LOG="$TEST_ROOT/keygen.log"
: > "$TEST_KEYGEN_LOG"
func_keygen() {
  key_type="$1"
  key_path="$2"
  printf '%s %s\n' "$key_type" "$key_path" >> "$TEST_KEYGEN_LOG"
  printf 'generated:%s\n' "$key_type" > "$key_path"
  chmod 600 "$key_path"
}
kill() {
  [ "$#" -eq 2 ] || return 1
  printf '%s %s\n' "$1" "$2" >> "$TEST_SIGNALS"
  case "$1" in
    -TERM)
      [ "$TEST_SIGNAL_FAILURE" -eq 0 ] || return 77
      if [ "$TEST_TERM_STUCK" -eq 0 ]; then rm -rf "$TEST_PROC_ROOT/$2"; fi
      ;;
    -KILL) rm -rf "$TEST_PROC_ROOT/$2" ;;
    *) return 1 ;;
  esac
  return 0
}
fail() { echo "FAIL: $*" >&2; exit 1; }
make_proc() {
  pid=$1; comm=$2; exe=$3; start=$4
  mkdir -p "$TEST_PROC_ROOT/$pid"
  ln -s "$exe" "$TEST_PROC_ROOT/$pid/exe"
  # Linux /proc/PID/stat fields 3..22: state through starttime (20 tokens).
  printf '%s (%s) S 1 1 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 1 0 %s\n' "$pid" "$comm" "$start" > "$TEST_PROC_ROOT/$pid/stat"
}
reset_state() {
  rm -rf "$TEST_PROC_ROOT"/*
  : > "$TEST_SIGNALS"
  TEST_TERM_STUCK=0
  TEST_SIGNAL_FAILURE=0
}

make_proc 101 dropbear "$TEST_BIN" 5001
make_proc 102 dropbear /bin/sleep 5002
if dropbear_process_state; then state=0; else state=$?; fi
[ "$state" -eq 0 ] || fail "verified daemon was not found (state=$state)"
[ "$dropbear_process_pids" = "101:5001" ] || fail "spoofed comm entered snapshot: $dropbear_process_pids"
if dropbear_process_identity_matches 101 4999; then fail "PID reuse was accepted"; else status=$?; fi
[ "$status" -eq 1 ] || fail "PID reuse should be an identity mismatch"
func_stop || fail "verified Dropbear did not stop"
[ "$(cat "$TEST_SIGNALS")" = "-TERM 101" ] || fail "unexpected targeted signals: $(cat "$TEST_SIGNALS")"
[ -d "$TEST_PROC_ROOT/102" ] || fail "comm-spoofed process was removed"
[ "$TEST_KILLALL_CALLS" -eq 0 ] || fail "comm-only killall was called"
echo "Dropbear executable identity and targeted TERM stop: PASS"

reset_state
make_proc 201 dropbear /bin/sleep 6001
if dropbear_process_state; then state=0; else state=$?; fi
[ "$state" -eq 1 ] || fail "spoof-only state should be ABSENT, got $state"
func_stop || fail "stop should succeed with only spoof process"
[ ! -s "$TEST_SIGNALS" ] || fail "signal sent to spoof process"
echo "Dropbear comm spoof rejection: PASS"

reset_state
make_proc 301 dropbear "$TEST_BIN" 7001
TEST_TERM_STUCK=1
func_stop || fail "verified stuck daemon did not stop after KILL escalation"
[ "$(cat "$TEST_SIGNALS")" = "-TERM 301
-KILL 301" ] || fail "unexpected TERM/KILL sequence"
echo "Dropbear bounded TERM/KILL escalation: PASS"

reset_state
make_proc 401 dropbear "$TEST_BIN" 8001
TEST_SIGNAL_FAILURE=1
if func_stop; then fail "failed TERM was reported as success"; else status=$?; fi
[ "$status" -eq 77 ] || fail "TERM failure status not propagated ($status)"
[ -d "$TEST_PROC_ROOT/401" ] || fail "fixture unexpectedly removed after failed TERM"
[ "$(cat "$TEST_SIGNALS")" = "-TERM 401" ] || fail "unexpected signal after TERM failure"
echo "Dropbear signal failure propagation: PASS"

reset_state
DROPBEAR_PROC_ROOT="$TEST_ROOT/missing"
if dropbear_process_state; then state=0; else state=$?; fi
[ "$state" -eq 2 ] || fail "missing procfs should be UNKNOWN, got $state"
if func_stop; then fail "stop succeeded with missing procfs"; else status=$?; fi
[ "$status" -eq 2 ] || fail "stop should fail closed with missing procfs ($status)"
[ ! -s "$TEST_SIGNALS" ] || fail "signal sent with missing procfs"

DROPBEAR_PROC_ROOT="$TEST_PROC_ROOT"
mkdir -p "$TEST_PROC_ROOT/501"
printf 'malformed process stat\n' > "$TEST_PROC_ROOT/501/stat"
if dropbear_process_state; then state=0; else state=$?; fi
[ "$state" -eq 2 ] || fail "malformed stat should be UNKNOWN, got $state"
if func_stop; then fail "stop succeeded with malformed stat"; else status=$?; fi
[ "$status" -eq 2 ] || fail "stop should fail closed with malformed stat ($status)"
[ ! -s "$TEST_SIGNALS" ] || fail "signal sent with malformed stat"

reset_state
make_proc 601 dropbear "$TEST_BIN" 9001
rm -f "$TEST_PROC_ROOT/601/exe"
ln -s "$TEST_ROOT/nonexistent-executable" "$TEST_PROC_ROOT/601/exe"
if dropbear_process_state; then state=0; else state=$?; fi
[ "$state" -eq 2 ] || fail "missing executable identity should be UNKNOWN, got $state"
if func_stop; then fail "stop succeeded with unknown executable identity"; else status=$?; fi
[ "$status" -eq 2 ] || fail "stop should fail closed with unknown exe ($status)"
[ ! -s "$TEST_SIGNALS" ] || fail "signal sent with unknown executable identity"
echo "Dropbear incomplete process inventory fails closed: PASS"

if grep -F 'pidof dropbear' "$SOURCE" >/dev/null 2>&1; then fail "pidof dropbear remains"; fi
if grep -F 'killall -q dropbear' "$SOURCE" >/dev/null 2>&1; then fail "comm-only killall remains"; fi
echo "Dropbear process-name-only control removed: PASS"

# Missing one algorithm must not regenerate unrelated persistent host keys.
reset_keys() {
  rm -rf "$DROPBEAR_STORAGE_DIR"
  mkdir -p "$DROPBEAR_STORAGE_DIR"
  : > "$TEST_KEYGEN_LOG"
}
reset_keys
printf 'original-rsa\n' > "$rsa_key"
printf 'original-dss\n' > "$dss_key"
printf 'original-ecdsa\n' > "$ecdsa_key"
chmod 644 "$rsa_key" "$dss_key" "$ecdsa_key"
func_ensurekeys || fail "could not complete one missing host key"
[ "$(cat "$rsa_key")" = "original-rsa" ] || fail "RSA identity was overwritten while creating Ed25519"
[ "$(cat "$dss_key")" = "original-dss" ] || fail "DSS identity was overwritten while creating Ed25519"
[ "$(cat "$ecdsa_key")" = "original-ecdsa" ] || fail "ECDSA identity was overwritten while creating Ed25519"
[ "$(cat "$ed25519_key")" = "generated:ed25519" ] || fail "missing Ed25519 key was not generated"
[ "$(cat "$TEST_KEYGEN_LOG")" = "ed25519 $ed25519_key" ] || fail "unexpected key generation set: $(cat "$TEST_KEYGEN_LOG")"
for key_file in "$rsa_key" "$dss_key" "$ecdsa_key" "$ed25519_key"; do
  [ "$(stat -c '%a' "$key_file")" = 600 ] || fail "host key permissions were not restricted: $key_file"
done
echo "Dropbear single-key repair preserves existing host identities: PASS"

# The same preservation rule applies when the missing member is RSA.
reset_keys
printf 'original-dss\n' > "$dss_key"
printf 'original-ecdsa\n' > "$ecdsa_key"
printf 'original-ed25519\n' > "$ed25519_key"
func_ensurekeys || fail "could not regenerate missing RSA key"
[ "$(cat "$rsa_key")" = "generated:rsa" ] || fail "missing RSA key was not generated"
[ "$(cat "$dss_key")" = "original-dss" ] || fail "DSS identity was overwritten while creating RSA"
[ "$(cat "$ecdsa_key")" = "original-ecdsa" ] || fail "ECDSA identity was overwritten while creating RSA"
[ "$(cat "$ed25519_key")" = "original-ed25519" ] || fail "Ed25519 identity was overwritten while creating RSA"
[ "$(cat "$TEST_KEYGEN_LOG")" = "rsa $rsa_key" ] || fail "RSA repair generated unrelated key types"
echo "Dropbear RSA key repair preserves other algorithms: PASS"

# A conflicting symlink is not followed or replaced during key completion.
reset_keys
printf 'preserve-rsa\n' > "$rsa_key"
printf 'outside-private-key\n' > "$TEST_ROOT/outside-key"
ln -s "$TEST_ROOT/outside-key" "$dss_key"
if func_ensurekeys; then fail "symlinked host key was accepted"; else key_status=$?; fi
[ "$key_status" -ne 0 ] || fail "symlink key rejection lost its failure status"
[ "$(cat "$rsa_key")" = "preserve-rsa" ] || fail "existing RSA key was changed before symlink rejection"
[ "$(cat "$TEST_ROOT/outside-key")" = "outside-private-key" ] || fail "symlink target was modified"
[ -L "$dss_key" ] || fail "unsafe host-key symlink was replaced"
[ ! -s "$TEST_KEYGEN_LOG" ] || fail "keygen ran after an unsafe symlink was found"
echo "Dropbear host-key symlink rejection: PASS"

reset_keys
func_ensurekeys || fail "fresh key storage could not initialize all host keys"
[ "$(wc -l < "$TEST_KEYGEN_LOG" | tr -d ' ')" = 4 ] || fail "fresh storage did not generate exactly four key types"
for key_file in "$rsa_key" "$dss_key" "$ecdsa_key" "$ed25519_key"; do
  [ -f "$key_file" ] && [ ! -L "$key_file" ] || fail "fresh host-key file missing or unsafe: $key_file"
done
echo "Dropbear empty host-key store initialization: PASS"
