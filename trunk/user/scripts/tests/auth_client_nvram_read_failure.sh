#!/bin/sh
set -eu

TEST_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$TEST_DIR/../../../../" && pwd)
SCUT_SOURCE="$REPO_ROOT/trunk/user/scutclient/scripts/scutclient.sh"
MENTOHUST_SOURCE="$REPO_ROOT/trunk/user/mentohust/scripts/mentohust.sh"
TEST_ROOT=$(mktemp -d "${TMPDIR:-/tmp}/auth-client-nvram.XXXXXX")
trap 'rm -rf "$TEST_ROOT"' EXIT HUP INT TERM
export TEST_ROOT

mkdir -p "$TEST_ROOT/bin" "$TEST_ROOT/storage"
PATH="$TEST_ROOT/bin:$PATH"
export PATH
START_LOG="$TEST_ROOT/start.log"
NVRAM_LOG="$TEST_ROOT/nvram.log"
export START_LOG NVRAM_LOG
: > "$START_LOG"
: > "$NVRAM_LOG"

cat > "$TEST_ROOT/process_lifecycle.sh" <<'STUB'
pl_process_state() { return 1; }
pl_wait_started() { return 0; }
pl_stop_named() { return 0; }
STUB
PROCESS_LIFECYCLE_LIB="$TEST_ROOT/process_lifecycle.sh"
export PROCESS_LIFECYCLE_LIB

cat > "$TEST_ROOT/bin/nvram" <<'STUB'
#!/bin/sh
case "$1" in
get)
	# Model the old accessor's failure ambiguity: an I/O failure is reported
	# as an empty value and success, so callers cannot distinguish it.
	printf '%s\n' ""
	;;
get_checked)
	printf '%s\n' "$2" >> "$NVRAM_LOG"
	if [ "${NVRAM_FAIL_KEY:-}" = "$2" ]; then
		exit "${NVRAM_FAIL_STATUS:-47}"
	fi
	case "$2" in
		scutclient_debug) printf '%s\n' "${SCUT_DEBUG-1}" ;;
		scutclient_skip_udp_hb) printf '%s\n' "${SCUT_SKIP_UDP_HB-1}" ;;
		scutclient_auth_exec) printf '%s\n' "${SCUT_AUTH_HOOK-}" ;;
		scutclient_fail_exec) printf '%s\n' "${SCUT_FAIL_HOOK-}" ;;
		wan_dns1_x) printf '%s\n' "${WAN_DNS-8.8.8.8 1.1.1.1}" ;;
		scutclient_username) printf '%s\n' "${SCUT_USERNAME-router-user}" ;;
		scutclient_password) printf '%s\n' "${SCUT_PASSWORD-router-secret}" ;;
		wan_ifname) printf '%s\n' "${WAN_IFNAME-eth2.1}" ;;
		scutclient_hostname) printf '%s\n' "${SCUT_HOSTNAME-router}" ;;
		scutclient_server_auth_ip) printf '%s\n' "${SCUT_AUTH_IP-192.0.2.1}" ;;
		scutclient_version) printf '%s\n' "${SCUT_VERSION-1}" ;;
		scutclient_hash) printf '%s\n' "${SCUT_HASH-hash-value}" ;;
		mentohust_nic) printf '%s\n' "${MENTOHUST_NIC-eth2.1}" ;;
		mentohust_username) printf '%s\n' "${MENTOHUST_USERNAME-router-user}" ;;
		mentohust_password) printf '%s\n' "${MENTOHUST_PASSWORD-router-secret}" ;;
		mentohust_ip) printf '%s\n' "${MENTOHUST_IP-192.0.2.2}" ;;
		mentohust_mask) printf '%s\n' "${MENTOHUST_MASK-255.255.255.0}" ;;
		mentohust_gw) printf '%s\n' "${MENTOHUST_GW-192.0.2.1}" ;;
		mentohust_dns) printf '%s\n' "${MENTOHUST_DNS-8.8.8.8}" ;;
		mentohust_pinghost) printf '%s\n' "${MENTOHUST_PINGHOST-8.8.4.4}" ;;
		mentohust_timeout) printf '%s\n' "${MENTOHUST_TIMEOUT-8}" ;;
		mentohust_interval) printf '%s\n' "${MENTOHUST_INTERVAL-30}" ;;
		mentohust_restart_wait) printf '%s\n' "${MENTOHUST_RESTART_WAIT-10}" ;;
		mentohust_maxfail) printf '%s\n' "${MENTOHUST_MAXFAIL-3}" ;;
		mentohust_startmode) printf '%s\n' "${MENTOHUST_STARTMODE-1}" ;;
		mentohust_dhcp) printf '%s\n' "${MENTOHUST_DHCP-0}" ;;
		mentohust_daemon) printf '%s\n' "${MENTOHUST_DAEMON-1}" ;;
		mentohust_ver) printf '%s\n' "${MENTOHUST_VER-1}" ;;
		mentohust_datafile) printf '%s\n' "${MENTOHUST_DATAFILE-}" ;;
		mentohust_dhcpscript) printf '%s\n' "${MENTOHUST_DHCPSCRIPT-}" ;;
		mentohust_service) printf '%s\n' "${MENTOHUST_SERVICE-internet}" ;;
		*) printf '%s\n' "" ;;
	esac
	;;
set|commit)
	printf '%s\n' "$*" >> "$NVRAM_LOG"
	;;
*)
	exit 2
	;;
esac
STUB
cat > "$TEST_ROOT/bin/logger" <<'STUB'
#!/bin/sh
printf '%s\n' "$*" >> "$TEST_ROOT/logger.log"
STUB
cat > "$TEST_ROOT/bin/start-stop-daemon" <<'STUB'
#!/bin/sh
printf '%s\n' "$*" >> "$START_LOG"
exit 0
STUB
cat > "$TEST_ROOT/bin/bin_scutclient" <<'STUB'
#!/bin/sh
printf 'scutclient-binary %s\n' "$*" >> "$START_LOG"
exit 0
STUB
chmod 755 "$TEST_ROOT/bin/"*

fail() {
	echo "Authentication client NVRAM regression: FAIL: $*" >&2
	exit 1
}

run_scut_fail_case() {
	key=$1
	: > "$START_LOG"
	NVRAM_FAIL_KEY=$key
	NVRAM_FAIL_STATUS=47
	export NVRAM_FAIL_KEY NVRAM_FAIL_STATUS
	if sh "$SCUT_SOURCE" start nolog >/dev/null 2>&1; then
		fail "SCUT launched despite checked NVRAM failure for $key"
	else
		status=$?
		[ "$status" -eq 47 ] || fail "SCUT changed $key failure status to $status"
	fi
	[ ! -s "$START_LOG" ] || fail "SCUT invoked start-stop-daemon after $key failure"
}

for key in \
	scutclient_debug scutclient_skip_udp_hb scutclient_auth_exec \
	scutclient_fail_exec wan_dns1_x scutclient_username scutclient_password \
	wan_ifname scutclient_hostname scutclient_server_auth_ip \
	scutclient_version scutclient_hash
do
	run_scut_fail_case "$key"
done

# A successful missing-key response is empty, not an I/O failure. Preserve
# the legacy default DNS and optional empty hook/credential arguments.
NVRAM_FAIL_KEY=
SCUT_DEBUG=
SCUT_SKIP_UDP_HB=
SCUT_AUTH_HOOK=
SCUT_FAIL_HOOK=
WAN_DNS=
SCUT_USERNAME=
SCUT_PASSWORD=
WAN_IFNAME=
SCUT_HOSTNAME=
SCUT_AUTH_IP=
SCUT_VERSION=
SCUT_HASH=
export NVRAM_FAIL_KEY SCUT_DEBUG SCUT_SKIP_UDP_HB SCUT_AUTH_HOOK SCUT_FAIL_HOOK
export WAN_DNS SCUT_USERNAME SCUT_PASSWORD WAN_IFNAME SCUT_HOSTNAME SCUT_AUTH_IP
export SCUT_VERSION SCUT_HASH
: > "$START_LOG"
sh "$SCUT_SOURCE" start nolog >/dev/null 2>&1 ||
	fail "SCUT rejected successful missing-key NVRAM reads"
grep -Fq -- '-n 114.114.114.114' "$START_LOG" ||
	fail "SCUT lost its fallback DNS when the key is absent"

NVRAM_FAIL_KEY=wan_ifname
export NVRAM_FAIL_KEY
: > "$START_LOG"
if sh "$SCUT_SOURCE" stop nolog >/dev/null 2>&1; then
	fail "SCUT logout path hid a failed WAN-interface read"
else
	status=$?
	[ "$status" -eq 47 ] || fail "SCUT stop changed NVRAM failure status to $status"
fi
if grep -Fq 'bin_scutclient -o' "$START_LOG"; then
	fail "SCUT ran logout with an unchecked empty interface"
fi

run_mentohust_fail_case() {
	key=$1
	case_dir="$TEST_ROOT/mentohust-$key"
	mkdir -p "$case_dir/storage"
	MENTOHUST_CONF_FILE="$case_dir/mentohust.conf"
	MENTOHUST_BIN_CONF_FILE="$case_dir/storage/mentohust.conf"
	NVRAM_FAIL_KEY=$key
	NVRAM_FAIL_STATUS=47
	MENTOHUST_NIC=eth2.1
	export MENTOHUST_CONF_FILE MENTOHUST_BIN_CONF_FILE NVRAM_FAIL_KEY NVRAM_FAIL_STATUS MENTOHUST_NIC
	if [ "$key" = "wan_ifname" ]; then
		MENTOHUST_NIC=
		export MENTOHUST_NIC
	fi
	: > "$START_LOG"
	if sh "$MENTOHUST_SOURCE" start >/dev/null 2>&1; then
		fail "MentoHUST launched despite checked NVRAM failure for $key"
	else
		status=$?
		[ "$status" -eq 47 ] || fail "MentoHUST changed $key failure status to $status"
	fi
	[ ! -e "$MENTOHUST_CONF_FILE" ] || fail "MentoHUST published config after $key failure"
	[ ! -L "$MENTOHUST_BIN_CONF_FILE" ] || fail "MentoHUST published config link after $key failure"
	[ ! -s "$START_LOG" ] || fail "MentoHUST launched after $key failure"
}

run_mentohust_fail_case mentohust_nic
run_mentohust_fail_case wan_ifname
for key in \
	mentohust_username mentohust_password mentohust_nic mentohust_ip \
	mentohust_mask mentohust_gw mentohust_dns mentohust_pinghost \
	mentohust_timeout mentohust_interval mentohust_restart_wait \
	mentohust_maxfail mentohust_startmode mentohust_dhcp mentohust_daemon \
	mentohust_ver mentohust_datafile mentohust_dhcpscript mentohust_service
do
	run_mentohust_fail_case "$key"
done

# Missing interface values remain a successful empty result; MentoHUST
# retains the legacy WAN-ifname fallback and private config publication.
MENTOHUST_CONF_FILE="$TEST_ROOT/missing-keys/mentohust.conf"
MENTOHUST_BIN_CONF_FILE="$TEST_ROOT/missing-keys/storage/mentohust.conf"
mkdir -p "$TEST_ROOT/missing-keys/storage"
MENTOHUST_NIC=
WAN_IFNAME=
MENTOHUST_SERVICE=
NVRAM_FAIL_KEY=
export MENTOHUST_CONF_FILE MENTOHUST_BIN_CONF_FILE MENTOHUST_NIC WAN_IFNAME MENTOHUST_SERVICE NVRAM_FAIL_KEY
: > "$START_LOG"
sh "$MENTOHUST_SOURCE" start >/dev/null 2>&1 ||
	fail "MentoHUST rejected successful missing-key reads"
[ -f "$MENTOHUST_CONF_FILE" ] || fail "MentoHUST did not publish missing-key config"
[ -L "$MENTOHUST_BIN_CONF_FILE" ] || fail "MentoHUST did not publish compatibility link"
[ "$(stat -c '%a' "$MENTOHUST_CONF_FILE")" = 600 ] ||
	fail "MentoHUST config permissions are not 0600"
grep -Fqx 'Nic=' "$MENTOHUST_CONF_FILE" ||
	fail "MentoHUST did not preserve empty interface fallback semantics"
grep -Fqx 'Service=' "$MENTOHUST_CONF_FILE" ||
	fail "MentoHUST did not preserve empty optional mapping semantics"

echo "SCUT and MentoHUST checked NVRAM reads fail closed and preserve absent-key behavior: PASS"
