#!/usr/bin/env python3
"""Source, package, trust-store and RC contracts for AliDDNS API transport."""

from pathlib import Path
import tarfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE_PATH = ROOT / "trunk/user/aliddns/aliddns.sh"
TEST_PATH = ROOT / "trunk/user/aliddns/tests/aliddns_api_transport.sh"
PACKAGE_PATH = ROOT / "trunk/user/aliddns/Makefile"
USER_MAKEFILE_PATH = ROOT / "trunk/user/Makefile"
LIBS_MAKEFILE_PATH = ROOT / "trunk/libs/Makefile"
LIBCURL_MAKEFILE_PATH = ROOT / "trunk/libs/libcurl/Makefile"
CERT_ARCHIVE_PATH = ROOT / "trunk/user/scripts/files/etc_ro/certs.tgz"
COPY_SCRIPTS_PATH = ROOT / "trunk/user/scripts/files/usr/bin/copyscripts.sh"
AUTOSTART_PATH = ROOT / "trunk/user/scripts/files/usr/bin/autostart.sh"
RC_PATH = ROOT / "trunk/user/rc/src/rc.c"
SERVICES_PATH = ROOT / "trunk/user/rc/src/services.c"
VARIABLES_PATH = ROOT / "trunk/user/httpd/src/variables.c"

source = SOURCE_PATH.read_text()
test = TEST_PATH.read_text()
package = PACKAGE_PATH.read_text()
user_makefile = USER_MAKEFILE_PATH.read_text()
libs_makefile = LIBS_MAKEFILE_PATH.read_text()
libcurl_makefile = LIBCURL_MAKEFILE_PATH.read_text()
copy_scripts = COPY_SCRIPTS_PATH.read_text()
autostart = AUTOSTART_PATH.read_text()
rc = RC_PATH.read_text()
services = SERVICES_PATH.read_text()
variables = VARIABLES_PATH.read_text()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"AliDDNS API transport contract: FAIL: {message}")


def function_body(text: str, signature: str) -> str:
    start = text.find(signature)
    require(start >= 0, f"missing function {signature}")
    opening = text.find("{", start)
    require(opening >= 0, f"missing opening brace for {signature}")
    closing = text.find("\n}", opening + 1)
    require(closing >= 0, f"missing closing brace for {signature}")
    return text[opening + 1:closing]


send_request = function_body(source, "send_request() {")
response_ok = function_body(source, "aliddns_response_ok() {")
info = function_body(source, "arDdnsInfo() {")
update = function_body(source, "arDdnsUpdate() {")

require('ALIDDNS_CA_BUNDLE="/etc/ssl/certs/ca-certificates.crt"' in source and
        '--cacert "$ALIDDNS_CA_BUNDLE"' in send_request,
        "AliDNS must use the fixed firmware CA bundle explicitly")
require("https://alidns.aliyuncs.com/?" in send_request and
        "http://alidns.aliyuncs.com/?" not in send_request and
        "--proto '=https'" in send_request,
        "signed AliDNS requests must use the official HTTPS endpoint only")
require("--fail --silent --show-error" in send_request and
        "--connect-timeout 5" in send_request and "--max-time 20" in send_request and
        "--write-out '\\n%{http_code}'" in send_request and
        '2[0-9][0-9]' in send_request,
        "transport must bound connection/total time and accept only HTTP 2xx")
require("-k" not in send_request and "--insecure" not in send_request and
        "curl -L" not in send_request and "--location" not in send_request,
        "signed requests must not disable TLS checks or forward credentials through redirects")
require('args="AccessKeyId=$aliddns_ak&Action=$1&Format=json&$2&Version=2015-01-09"' in send_request and
        'hash=$(echo -n "GET&%2F&$(enc "$args")" | openssl dgst -sha1 -hmac "$aliddns_sk&" -binary | openssl base64)' in send_request,
        "existing RPC parameter ordering and HMAC-SHA1 signature canonicalization must remain unchanged")

require('"RequestId"[[:space:]]*:' in response_ok and
        '"Code"[[:space:]]*:' in response_ok,
        "API responses must include a request ID and must reject API error codes")
require('response=$(query_recordid)' in info and
        'if ! aliddns_response_ok "$response"; then' in info and
        'response=$(query_recordInfo "$aliddns_record_id")' in info,
        "record detail lookup must propagate transport and API failures")
require('query_ok=0' in update and
        'if [ "$request_status" -eq 0 ] && aliddns_response_ok "$response"; then' in update and
        'if [ "$query_ok" != "1" ]; then' in update,
        "failed record discovery must never be treated as a valid empty record set")
require('response=$(add_record)' in update and
        'response=$(update_record "$aliddns_record_id")' in update and
        update.index('response=$(update_record "$aliddns_record_id")') < update.index('nvram set aliddns_record_id='),
        "updates/additions must validate their API response before publishing success state")

require('--with-openssl=$(STAGING_DIR)' in libcurl_makefile and
        '--with-ca-bundle=/etc/ssl/certs/ca-certificates.crt' in libcurl_makefile,
        "curl must remain linked to OpenSSL and the configured CA bundle path")
require('ifeq ($(CONFIG_FIRMWARE_INCLUDE_ALIDDNS),y)\nLIBS_INCLUDE_LIBCURL=y\nendif' in libs_makefile and
        'ifneq ($(filter y,$(CONFIG_FIRMWARE_INCLUDE_CURL) $(CONFIG_FIRMWARE_INCLUDE_ALIDDNS)),)' in libcurl_makefile,
        "enabling AliDDNS must build and install its required curl CLI")
require('$(ROMFSINST) -p +x $(THISDIR)/aliddns.sh /usr/bin/aliddns.sh' in package and
        'dir_$(CONFIG_FIRMWARE_INCLUDE_ALIDDNS)' in user_makefile,
        "AliDDNS firmware package and feature gate must remain intact")

require('mkdir -p /etc/ssl' in copy_scripts and
        'tar -xzf /etc_ro/certs.tgz -C /etc/ssl' in copy_scripts and
        'system("/usr/bin/copyscripts.sh &");' in rc,
        "firmware startup must populate the CA trust store from the packaged certificate archive")
require('if [ $(nvram get aliddns_enable) = 1 ]' in autostart and
        '/usr/bin/aliddns.sh start' in autostart,
        "enabled AliDDNS must retain its existing startup path")

require('EVM_RESTART_ALIDDNS' in variables and
        'RCN_RESTART_ALIDDNS' in rc and
        'restart_aliddns();' in rc and
        'void restart_aliddns(void)' in services,
        "HTTPD NVRAM changes must continue to reach the existing RC lifecycle wrapper")

with tarfile.open(CERT_ARCHIVE_PATH, "r:gz") as archive:
    members = {
        member.name.replace("\\", "/").lstrip("./")
        for member in archive.getmembers()
        if member.isfile()
    }
if "certs/ca-certificates.crt" not in members:
    candidates = sorted(name for name in members if "cert" in name.lower() or "ca" in name.lower())
    raise SystemExit(
        "AliDDNS API transport contract: FAIL: expected CA bundle missing from cert archive; "
        f"certificate candidates: {candidates[:30]}"
    )

for marker in (
    "Missing trust anchors fail before curl is invoked.",
    "certificate verification failure was reported as success",
    "non-2xx redirect response was reported as success",
    "record update succeeded after all lookups failed",
    "API-level update error was reported as success",
    "successful update did not save the record ID",
    "AliDDNS HTTPS-only request, CA verification, timeout and HTTP-status regression: PASS",
    "AliDDNS API failure propagation and update publication regression: PASS",
):
    require(marker in test, f"host regression missing case or result marker: {marker}")

print("AliDDNS HTTPS transport, firmware trust-store, API status and RC integration contracts: PASS")
