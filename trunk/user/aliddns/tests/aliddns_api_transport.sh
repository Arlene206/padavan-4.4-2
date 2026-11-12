#!/bin/sh
set -eu
TEST_DIR="$(mktemp -d)"
trap 'rm -rf "$TEST_DIR"' EXIT HUP INT TERM
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
SOURCE="$SCRIPT_DIR/../aliddns.sh"
FUNCTIONS="$TEST_DIR/functions.sh"
MOCK_BIN="$TEST_DIR/bin"
mkdir -p "$MOCK_BIN"
: > "$FUNCTIONS"

# Load only production functions under test; never run router startup code.
sed -n '/^send_request() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^aliddns_response_ok() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^aliddns_update_failed() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^query_recordid() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^query_recordInfo() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^update_record() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^add_record() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^get_recordid() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^arDdnsUpdate() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
. "$FUNCTIONS"

ALIDDNS_CA_BUNDLE="$TEST_DIR/ca-certificates.crt"
: > "$ALIDDNS_CA_BUNDLE"
CURL_ARGS_LOG="$TEST_DIR/curl-args.log"
CURL_COUNT_FILE="$TEST_DIR/curl-count"
NVRAM_LOG="$TEST_DIR/nvram.log"
: > "$NVRAM_LOG"
export CURL_ARGS_LOG CURL_COUNT_FILE NVRAM_LOG

logger() { :; }
nvram() {
	[ "$1" = "set" ] || return 1
	shift
	printf '%s\n' "$*" >> "$NVRAM_LOG"
}
sleep() { :; }
enc() { printf '%s' "$1"; }
openssl() {
	case "$1" in
		dgst) printf '%s' "raw-signature" ;;
		base64) printf '%s' "c2ln" ;;
		*) return 1 ;;
	esac
}

cat > "$MOCK_BIN/curl" <<'MOCK_CURL'
#!/bin/sh
printf '%s\n' "$@" > "$CURL_ARGS_LOG"
count=0
if [ -r "$CURL_COUNT_FILE" ]; then
	IFS= read -r count < "$CURL_COUNT_FILE"
fi
count=$((count + 1))
printf '%s\n' "$count" > "$CURL_COUNT_FILE"

case "$FAKE_SCENARIO" in
	transport-success)
		printf '%s\n' '{"RequestId":"transport"}' '200'
		;;
	certificate-failure|lookup-failure)
		echo "mock TLS verification failure" >&2
		exit 60
		;;
	http-failure)
		echo "mock HTTP failure" >&2
		exit 22
		;;
	redirect)
		printf '%s\n' '<html>redirect</html>' '302'
		;;
	update-api-error)
		if [ "$count" -eq 1 ]; then
			printf '%s\n' '{"RequestId":"query","RecordId":"123"}' '200'
		else
			printf '%s\n' '{"RequestId":"update","Code":"RequestThrottled"}' '200'
		fi
		;;
	update-success)
		if [ "$count" -eq 1 ]; then
			printf '%s\n' '{"RequestId":"query","RecordId":"123"}' '200'
		else
			printf '%s\n' '{"RequestId":"update"}' '200'
		fi
		;;
	add-success)
		if [ "$count" -lt 4 ]; then
			printf '%s\n' '{"RequestId":"query","DomainRecords":{"Record":[]}}' '200'
		else
			printf '%s\n' '{"RequestId":"add","RecordId":"456"}' '200'
		fi
		;;
	*)
		echo "unknown test scenario: $FAKE_SCENARIO" >&2
		exit 90
		;;
esac
MOCK_CURL
chmod 700 "$MOCK_BIN/curl"
PATH="$MOCK_BIN:$PATH"
export PATH

aliddns_ak="test-access-key"
aliddns_sk="test-access-secret"
timestamp="2026-11-12T16%3A30%3A00Z"
name="www"
domain="example.com"
name1="www"
domain_type="A"
IPv6=0
hostIP="203.0.113.9"
aliddns_ttl="600"

reset_scenario() {
	FAKE_SCENARIO="$1"
	export FAKE_SCENARIO
	rm -f "$CURL_COUNT_FILE"
	: > "$NVRAM_LOG"
}
fail() {
	echo "AliDDNS API transport regression: FAIL: $1" >&2
	exit 1
}

# The request preserves its signed parameters but pins HTTPS, trust and endpoint.
reset_scenario transport-success
response="$(send_request "DescribeSubDomainRecords" "SignatureMethod=HMAC-SHA1&SignatureNonce=nonce&SignatureVersion=1.0&SubDomain=www.example.com&Timestamp=$timestamp&Type=A")" ||
	fail "valid HTTPS request failed"
[ "$response" = '{"RequestId":"transport"}' ] || fail "HTTP status trailer leaked into API body"
grep -Fqx -- '--fail' "$CURL_ARGS_LOG" || fail "curl is missing --fail"
grep -Fqx -- '--connect-timeout' "$CURL_ARGS_LOG" || fail "curl is missing a connect timeout"
grep -Fqx -- '5' "$CURL_ARGS_LOG" || fail "connect timeout is not five seconds"
grep -Fqx -- '--max-time' "$CURL_ARGS_LOG" || fail "curl is missing a total timeout"
grep -Fqx -- '20' "$CURL_ARGS_LOG" || fail "total timeout is not twenty seconds"
grep -Fqx -- '--proto' "$CURL_ARGS_LOG" || fail "curl is missing a protocol allowlist"
grep -Fqx -- '=https' "$CURL_ARGS_LOG" || fail "curl protocol allowlist is not HTTPS-only"
grep -Fqx -- '--cacert' "$CURL_ARGS_LOG" || fail "curl is missing the explicit trust bundle"
grep -Fqx -- "$ALIDDNS_CA_BUNDLE" "$CURL_ARGS_LOG" || fail "curl did not use the configured CA bundle"
! grep -Fqx -- '-k' "$CURL_ARGS_LOG" || fail "TLS verification bypass was enabled"
! grep -Fqx -- '-L' "$CURL_ARGS_LOG" || fail "credential-bearing request follows redirects"
! grep -Fqx -- '--location' "$CURL_ARGS_LOG" || fail "credential-bearing request follows redirects"
url="$(tail -n 1 "$CURL_ARGS_LOG")"
case "$url" in
	https://alidns.aliyuncs.com/?AccessKeyId=test-access-key\&Action=DescribeSubDomainRecords\&Format=json\&SignatureMethod=HMAC-SHA1*Signature=c2ln)
		;;
	*) fail "signed query was not sent to the fixed HTTPS API endpoint" ;;
esac

# Missing trust anchors fail before curl is invoked.
count_before="$(cat "$CURL_COUNT_FILE")"
rm -f "$ALIDDNS_CA_BUNDLE"
if send_request "DescribeSubDomainRecords" "SignatureNonce=nonce"; then
	fail "request succeeded without a readable CA bundle"
fi
[ "$(cat "$CURL_COUNT_FILE")" = "$count_before" ] || fail "curl ran despite the missing CA bundle"
: > "$ALIDDNS_CA_BUNDLE"

# TLS validation failures, HTTP errors and redirects remain failures.
reset_scenario certificate-failure
if send_request "DescribeSubDomainRecords" "SignatureNonce=nonce"; then
	fail "certificate verification failure was reported as success"
fi
reset_scenario http-failure
if send_request "DescribeSubDomainRecords" "SignatureNonce=nonce"; then
	fail "HTTP error was reported as success"
fi
reset_scenario redirect
if send_request "DescribeSubDomainRecords" "SignatureNonce=nonce"; then
	fail "non-2xx redirect response was reported as success"
fi

# Query transport failures are not confused with an empty record set.
reset_scenario lookup-failure
if arDdnsUpdate; then
	fail "record update succeeded after all lookups failed"
fi
[ "$(cat "$CURL_COUNT_FILE")" -eq 3 ] || fail "failed lookup did not stop after three bounded attempts"
! grep -Fq 'aliddns_record_id=' "$NVRAM_LOG" || fail "failed lookup published a record ID"
grep -Fq 'aliddns_last_act=' "$NVRAM_LOG" && grep -Fq '更新失败' "$NVRAM_LOG" ||
	fail "failed lookup did not publish a failure status"

# HTTP 200 API error payloads are not successful updates.
reset_scenario update-api-error
if arDdnsUpdate; then
	fail "API-level update error was reported as success"
fi
[ "$(cat "$CURL_COUNT_FILE")" -eq 2 ] || fail "update path made an unexpected number of requests"
! grep -Fq 'aliddns_record_id=123' "$NVRAM_LOG" || fail "failed update published success state"
grep -Fq '更新失败' "$NVRAM_LOG" || fail "API-level update failure was not reported"

reset_scenario update-success
arDdnsUpdate || fail "successful update response was rejected"
[ "$(cat "$CURL_COUNT_FILE")" -eq 2 ] || fail "successful update made an unexpected number of requests"
grep -Fqx 'aliddns_record_id=123' "$NVRAM_LOG" || fail "successful update did not save the record ID"
grep -Fq '成功更新：203.0.113.9' "$NVRAM_LOG" || fail "successful update status was not published"

# AddDomainRecord is attempted only after valid, successful empty-record reads.
reset_scenario add-success
aliddns_record_id=""
arDdnsUpdate || fail "new record flow failed"
[ "$(cat "$CURL_COUNT_FILE")" -eq 4 ] || fail "new record flow did not use three reads and one add"
grep -Fqx 'aliddns_record_id=456' "$NVRAM_LOG" || fail "new record ID was not published"

echo "AliDDNS HTTPS-only request, CA verification, timeout and HTTP-status regression: PASS"
echo "AliDDNS API failure propagation and update publication regression: PASS"
