#!/bin/sh
# Exercise OpenVPN certificate generation with fake crypto tools, without
# creating or touching live router certificate/key paths.
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
SOURCE="$ROOT/trunk/user/openvpn/openvpn-cert.sh"
TMP=$(mktemp -d "${TMPDIR:-/tmp}/openvpn-cert-key-test.XXXXXX")
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
mkdir -p "$TMP/bin" "$TMP/storage"

cat > "$TMP/bin/openssl" <<'MOCK_OPENSSL'
#!/bin/sh
command_name=$1
shift
case "$command_name" in
list)
	printf '%s\n' 'sha256'
	exit 0
	;;
genrsa|ecparam|genpkey|req|x509|dhparam)
	if [ "${MOCK_OPENSSL_FAIL_COMMAND:-}" = "$command_name" ]; then
		output=
		while [ "$#" -gt 0 ]; do
			if [ "$1" = "-out" ]; then shift; output=$1; fi
			shift
		done
		[ -n "$output" ] && printf 'partial\\n' > "$output"
		exit 74
	fi
	if [ "${MOCK_OPENSSL_EMPTY_COMMAND:-}" = "$command_name" ]; then
		exit 0
	fi
	output=
	while [ "$#" -gt 0 ]; do
		if [ "$1" = "-out" ]; then
			shift
			output=$1
		fi
		shift
	done
	[ -n "$output" ] || exit 64
	printf 'mock-%s\n' "$command_name" > "$output"
	mode=$(stat -c '%a' "$output")
	printf '%s %s %s\n' "$command_name" "$output" "$mode" >> "$OPENSSL_MODE_LOG"
	;;
*)
	exit 64
	;;
esac
MOCK_OPENSSL
chmod 755 "$TMP/bin/openssl"

cat > "$TMP/bin/openvpn" <<'MOCK_OPENVPN'
#!/bin/sh
[ "$1" = "--genkey" ] || exit 64
key_type=$2
output=$3
if [ "${MOCK_OPENVPN_FAIL_TYPE:-}" = "$key_type" ]; then
	printf 'partial-secret\\n' > "$output"
	exit 73
fi
printf 'mock-secret-%s\n' "$key_type" > "$output"
mode=$(stat -c '%a' "$output")
printf '%s %s %s\n' "$key_type" "$output" "$mode" >> "$OPENSSL_MODE_LOG"
MOCK_OPENVPN
chmod 755 "$TMP/bin/openvpn"

cat > "$TMP/bin/chmod" <<'MOCK_CHMOD'
#!/bin/sh
if [ "${MOCK_CHMOD_FAIL_FILE:-}" = "${2##*/}" ]; then exit 75; fi
exec /bin/chmod "$@"
MOCK_CHMOD
chmod 755 "$TMP/bin/chmod"

# Relocate only the three fixed paths in a disposable copy; production
# continues to use its configured /usr/sbin and /etc/storage paths.
sed \
	-e "s|^OPENVPN=/usr/sbin/openvpn$|OPENVPN=$TMP/bin/openvpn|" \
	-e "s|^CRT_PATH=/etc/storage/openvpn$|CRT_PATH=$TMP/storage|" \
	-e "s|^SSL_EXT_FILE=/etc/ssl/openssl_ext1.cnf$|SSL_EXT_FILE=$TMP/openssl-ext.cnf|" \
	"$SOURCE" > "$TMP/openvpn-cert-test.sh"
chmod 755 "$TMP/openvpn-cert-test.sh"

run_server()
{
	OPENSSL_MODE_LOG="$TMP/modes.log" PATH="$TMP/bin:$PATH" \
		bash "$TMP/openvpn-cert-test.sh" server "$@" >/dev/null
}

assert_private()
{
	actual=$(stat -c '%a' "$1")
	if [ "$actual" != "600" ]; then
		echo "Expected mode 600 for $1; found $actual" >&2
		exit 1
	fi
}

run_server
SERVER_DIR="$TMP/storage/server"
for key in ca.key server.key ta.key stc2.key; do
	assert_private "$SERVER_DIR/$key"
done

# Verify TLS keys are repaired when they already exist with the old
# world-readable mode.
chmod 644 "$SERVER_DIR/ta.key" "$SERVER_DIR/stc2.key"
run_server
assert_private "$SERVER_DIR/ta.key"
assert_private "$SERVER_DIR/stc2.key"

# Private keys must already be mode 0600 when created, not just after chmod.
awk '$2 ~ /\.key$/ && $3 != "600" { failed = 1 } END { exit failed }' "$TMP/modes.log"

# A failed TLS-crypt-v2 server-key generator must fail the whole action.
rm -f "$SERVER_DIR/stc2.key"
if OPENSSL_MODE_LOG="$TMP/modes.log" MOCK_OPENVPN_FAIL_TYPE=tls-crypt-v2-server \
	PATH="$TMP/bin:$PATH" bash "$TMP/openvpn-cert-test.sh" server >/dev/null 2>&1; then
	echo "Expected TLS-crypt-v2 server-key generation failure to propagate" >&2
	exit 1
fi
[ ! -e "$SERVER_DIR/stc2.key" ]


# OpenSSL errors must stop the compound server workflow immediately.
assert_server_openssl_failure()
{
	failure_command=$1
	shift
	rm -rf "$SERVER_DIR"
	if OPENSSL_MODE_LOG="$TMP/modes.log" MOCK_OPENSSL_FAIL_COMMAND="$failure_command" PATH="$TMP/bin:$PATH" bash "$TMP/openvpn-cert-test.sh" server "$@" >/dev/null 2>&1; then
		echo "Expected OpenSSL $failure_command failure to abort server generation" >&2
		exit 1
	fi
}
assert_server_openssl_empty_output()
{
	empty_command=$1
	shift
	rm -rf "$SERVER_DIR"
	if OPENSSL_MODE_LOG="$TMP/modes.log" MOCK_OPENSSL_EMPTY_COMMAND="$empty_command" PATH="$TMP/bin:$PATH" bash "$TMP/openvpn-cert-test.sh" server "$@" >/dev/null 2>&1; then
		echo "Expected empty OpenSSL $empty_command output to abort server generation" >&2
		exit 1
	fi
}
assert_server_openssl_failure genrsa
[ ! -e "$SERVER_DIR/ca.key" ]
assert_server_openssl_failure ecparam -b secp256r1
[ ! -e "$SERVER_DIR/ca.key" ]
assert_server_openssl_failure genpkey -b ed25519
[ ! -e "$SERVER_DIR/ca.key" ]
assert_server_openssl_failure req
[ ! -e "$SERVER_DIR/ca.crt" ]
assert_server_openssl_empty_output genrsa
[ ! -e "$SERVER_DIR/ca.key" ]
assert_server_openssl_empty_output req
[ ! -s "$SERVER_DIR/ca.crt" ]
# A failed signer must leave its CSR available for retry and must not proceed.
assert_server_openssl_failure x509
[ -s "$SERVER_DIR/server.csr" ]
[ ! -e "$SERVER_DIR/server.crt" ]
[ ! -e "$SERVER_DIR/stc2.key" ]
assert_server_openssl_failure dhparam
[ ! -e "$SERVER_DIR/dh1024.pem" ]
[ ! -e "$SERVER_DIR/ta.key" ]
assert_server_openssl_empty_output dhparam
[ ! -s "$SERVER_DIR/dh1024.pem" ]
[ ! -e "$SERVER_DIR/ta.key" ]
# A key-protection failure aborts before a CSR can be generated.
rm -rf "$SERVER_DIR"
if OPENSSL_MODE_LOG="$TMP/modes.log" MOCK_CHMOD_FAIL_FILE=ca.key PATH="$TMP/bin:$PATH" bash "$TMP/openvpn-cert-test.sh" server >/dev/null 2>&1; then
	echo "Expected failed private-key chmod to abort certificate generation" >&2
	exit 1
fi
[ ! -e "$SERVER_DIR/ca.crt" ]
# TLS key protection failures remove the exposed key and abort the action.
rm -rf "$SERVER_DIR"
if OPENSSL_MODE_LOG="$TMP/modes.log" MOCK_CHMOD_FAIL_FILE=ta.key PATH="$TMP/bin:$PATH" bash "$TMP/openvpn-cert-test.sh" server >/dev/null 2>&1; then
	echo "Expected TLS Auth key chmod failure to abort certificate generation" >&2
	exit 1
fi
[ ! -e "$SERVER_DIR/ta.key" ]

rm -rf "$SERVER_DIR"
if OPENSSL_MODE_LOG="$TMP/modes.log" MOCK_CHMOD_FAIL_FILE=stc2.key PATH="$TMP/bin:$PATH" bash "$TMP/openvpn-cert-test.sh" server >/dev/null 2>&1; then
	echo "Expected TLS Crypt v2 key chmod failure to abort certificate generation" >&2
	exit 1
fi
[ ! -e "$SERVER_DIR/stc2.key" ]

# Client orchestration must not continue after CSR generation fails.
rm -rf "$TMP/storage/client"
if OPENSSL_MODE_LOG="$TMP/modes.log" MOCK_OPENSSL_FAIL_COMMAND=req PATH="$TMP/bin:$PATH" bash "$TMP/openvpn-cert-test.sh" client -n client-test >/dev/null 2>&1; then
	echo "Expected failed client CSR generation to abort client signing" >&2
	exit 1
fi
[ ! -e "$TMP/storage/client/client.csr" ]
[ ! -e "$TMP/storage/client/client.crt" ]
echo "OpenVPN private-key permissions and certificate failure propagation regression: PASS"
