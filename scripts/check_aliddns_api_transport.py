#!/usr/bin/env python3
"""Source, package, trust-store and RC contracts for AliDDNS API transport."""

from pathlib import Path
import tarfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE_PATH = ROOT / "trunk/user/aliddns/aliddns.sh"
TEST_PATH = ROOT / "trunk/user/aliddns/tests/aliddns_api_transport.sh"
IPV4_TEST_PATH = ROOT / "trunk/user/aliddns/tests/aliddns_ipv4_lookup.sh"
NSLOOKUP_TEST_PATH = ROOT / "trunk/user/aliddns/tests/aliddns_nslookup_lifecycle.sh"
CUSTOM_SCRIPT_TEST_PATH = ROOT / "trunk/user/aliddns/tests/aliddns_custom_script_lifecycle.sh"
CA_BOOTSTRAP_TEST_PATH = ROOT / "trunk/user/aliddns/tests/copyscripts_ca_bootstrap.sh"
HMAC_TEST_PATH = ROOT / "trunk/user/aliddns/tests/aliddns_hmac_sha1.sh"
HMAC_SOURCE_PATH = ROOT / "trunk/user/aliddns/src/aliddns-hmac-sha1.c"
HMAC_MAKEFILE_PATH = ROOT / "trunk/user/aliddns/src/Makefile"
LIBSSL_MAKEFILE_PATH = ROOT / "trunk/libs/libssl/Makefile"
WORKFLOW_PATH = ROOT / ".github/workflows/aliddns-api-transport.yml"
IPV4_HELPER_PATH = ROOT / "trunk/user/aliddns/aliddns-ipv4.sh"
DDNS_SCRIPT_PATH = ROOT / "trunk/user/aliddns/ddns_script.sh"
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
ipv4_test = IPV4_TEST_PATH.read_text()
nslookup_test = NSLOOKUP_TEST_PATH.read_text()
custom_script_test = CUSTOM_SCRIPT_TEST_PATH.read_text()
ca_bootstrap_test = CA_BOOTSTRAP_TEST_PATH.read_text()
hmac_test = HMAC_TEST_PATH.read_text()
hmac_source = HMAC_SOURCE_PATH.read_text()
hmac_makefile = HMAC_MAKEFILE_PATH.read_text()
libssl_makefile = LIBSSL_MAKEFILE_PATH.read_text()
workflow = WORKFLOW_PATH.read_text()
ipv4_helper = IPV4_HELPER_PATH.read_text()
ddns_script = DDNS_SCRIPT_PATH.read_text()
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
hmac_helper = function_body(source, "aliddns_hmac_sha1() {")
info = function_body(source, "arDdnsInfo() {")
update = function_body(source, "arDdnsUpdate() {")
ddns_check = function_body(source, "arDdnsCheck() {")
nslookup = function_body(source, "aliddns_nslookup() {")
nslookup4 = function_body(source, "arNslookup() {")
nslookup6 = function_body(source, "arNslookup6() {")

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
require("--config -" in send_request and 'url = "$request_url"' in send_request,
        "credential-bearing AliDNS URL must be delivered through curl stdin rather than argv")
require('args="AccessKeyId=$aliddns_ak&Action=$1&Format=json&$2&Version=2015-01-09"' in send_request and
        'signing_input="GET&%2F&$(enc "$args")"' in send_request and
        'hash=$(printf \'%s\\000%s\' "$aliddns_sk&" "$signing_input" | aliddns_hmac_sha1)' in send_request and
        "openssl dgst" not in send_request and "-hmac" not in send_request,
        "API parameters and canonical HMAC-SHA1 bytes must stay unchanged without putting the secret in OpenSSL argv")
require(hmac_helper.strip() == "/usr/bin/aliddns-hmac-sha1" and
        "openssl dgst" not in hmac_helper,
        "HMAC wrapper must invoke exactly the fixed ROMFS helper without exposing the secret in argv")
require('main(void)' in hmac_source and
        'EVP_MAC_fetch(NULL, "HMAC", NULL)' in hmac_source and
        'OSSL_MAC_PARAM_DIGEST' in hmac_source and
        'EVP_MAC_init(mac_context, key, key_length' in hmac_source and
        'EVP_MAC_update(mac_context' in hmac_source and
        'EVP_MAC_final(mac_context' in hmac_source and
        'EVP_EncodeBlock' in hmac_source and
        'key_length >= sizeof(key)' in hmac_source and
        'OPENSSL_cleanse(key, sizeof(key))' in hmac_source,
        "HMAC helper must stream NUL-framed stdin into OpenSSL 3 EVP_MAC-SHA1, bound/cleanse the key and return base64")
require('PKG_NAME:=aliddns' in package and
        'SRC_DIR:=./src' in package and
        'TARGET_CFLAGS += -I$(STAGING_DIR)/include' in package and
        'TARGET_LDFLAGS += -L$(STAGING_DIR)/lib' in package and
        '$(INSTALL_BIN) $(PKG_BUILD_DIR)/aliddns-hmac-sha1 $(ROMFSDIR)/usr/bin/aliddns-hmac-sha1' in package and
        'all: $(EXEC)' in hmac_makefile and '-lcrypto' in hmac_makefile and
        'dir_y += libssl' in ' '.join(libs_makefile.split()) and
        'PKG_VERSION:=3.0.21' in libssl_makefile and
        'DESTDIR="$(STAGING_DIR)"' in libssl_makefile and
        'install_sw install_ssldirs' in libssl_makefile,
        "HMAC helper must build against the staged OpenSSL 3 libcrypto and be installed into ROMFS")
require('aliddns-hmac-sha1.c' in test and
        'aliddns_hmac_sha1() {' in test and
        '$TEST_DIR/aliddns-hmac-sha1' in test and
        'expected_signature=' in test,
        "API transport fixture must compile and run the real HMAC helper and check its computed signature")
require('RFC 2202' in hmac_test and
        'AccessKeySecret absent from HMAC helper argv and environment: PASS' in hmac_test and
        'input-boundary regression: PASS' in hmac_test and
        'openssl dgst -sha1 -hmac' in hmac_test,
        "HMAC host regression must compare legacy byte output, RFC 2202, process argv/env and failure boundaries")

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
require("return 2" in info and '"$info_status" -eq 2' in ddns_check,
        "valid empty-record responses must remain distinguishable from transport/API failures")
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

require('install_ca_trust_store() {' in copy_scripts and
        '${COPYSCRIPTS_CERT_ARCHIVE:-/etc_ro/certs.tgz}' in copy_scripts and
        '${COPYSCRIPTS_CERT_DIR:-/etc/ssl}' in copy_scripts and
        'system("/usr/bin/copyscripts.sh")' in rc and
        'system("/usr/bin/copyscripts.sh &")' not in rc and
        rc.index('system("/usr/bin/copyscripts.sh")') < rc.index('system("/usr/bin/autostart.sh &")'),
        "firmware startup must install the CA trust store synchronously before autostart")
require('if [ $(nvram get aliddns_enable) = 1 ]' in autostart and
        '/usr/bin/aliddns.sh start' in autostart,
        "enabled AliDDNS must retain its existing startup path")

require('EVM_RESTART_ALIDDNS' in variables and
        'RCN_RESTART_ALIDDNS' in rc and
        'restart_aliddns();' in rc and
        'void restart_aliddns(void)' in services,
        "HTTPD NVRAM changes must continue to reach the existing RC lifecycle wrapper")

ipv4_valid = function_body(source, "aliddns_ipv4_valid() {")
ipv6_valid = function_body(source, "aliddns_ipv6_valid() {")
require('ALIDDNS_CA_BUNDLE="/etc/ssl/certs/ca-certificates.crt"' in ipv4_helper and
        '--cacert "$ALIDDNS_CA_BUNDLE"' in ipv4_helper,
        "public IPv4 helper must use the firmware CA bundle")
require("https://api.ipify.org" in ipv4_helper and "--proto '=https'" in ipv4_helper and
        "--ipv4" in ipv4_helper and "--connect-timeout 5" in ipv4_helper and "--max-time 15" in ipv4_helper,
        "public IPv4 helper must use bounded IPv4-only HTTPS to the plain-text provider")
require("--write-out '\\n%{http_code}'" in ipv4_helper and '[ "$http_status" = "200" ]' in ipv4_helper,
        "public IPv4 helper must reject non-200 status and strip the status trailer")
require("curl -k" not in ipv4_helper and "--insecure" not in ipv4_helper and
        "--location" not in ipv4_helper and "wget" not in ipv4_helper,
        "public IPv4 helper must not bypass TLS or follow redirects")
require("awk -F." in ipv4_helper and "NF != 4" in ipv4_helper and "($i + 0) > 255" in ipv4_helper,
        "public IPv4 helper must validate four bounded decimal octets")
require("NR != 1" in ipv4_valid and "NF != 4" in ipv4_valid and
        "NR != 1" in ipv6_valid and "count >= 8" in ipv6_valid,
        "AliDDNS must validate exactly one IPv4 or syntactically valid IPv6 address")

require("/usr/bin/aliddns-ipv4.sh" in ddns_script and "arIpAddress6" in ddns_script and
        "curl -k" not in ddns_script and "--no-check-certificate" not in ddns_script and
        "http://" not in ddns_script,
        "stock ROMFS DDNS script must use the secure IPv4 helper and preserve IPv6")
require('ddns_script_path="${ALIDDNS_DDNS_SCRIPT_PATH:-/etc/storage/ddns_script.sh}"' in ddns_check and
        '[ -L "$ddns_script_path" ]' in ddns_check and
        'arIpAddress=""' in ddns_check and
        'if ! . "$ddns_script_path"; then' in ddns_check and
        "/usr/bin/aliddns-ipv4.sh" in ddns_check and
        'hostIP=$(/usr/bin/aliddns-ipv4.sh)' in ddns_check and
        "aliddns_ipv4_valid" in ddns_check and "aliddns_ipv6_valid" in ddns_check,
        "AliDDNS check must reject symlinks and failed custom-script loads before validating addresses")
require("curl" not in nslookup and "wget" not in nslookup and "http://" not in nslookup and
        "killall" not in nslookup and
        'lookup_candidate_dir="/tmp/arNslookup.$$.${lookup_index}"' in nslookup and
        "umask 077" in nslookup and 'trap \'rm -rf "$lookup_dir"\' 0' in nslookup and
        'lookup_pid=$!' in nslookup and 'kill "$lookup_pid"' in nslookup and
        'kill -9 "$lookup_pid"' in nslookup and 'wait "$lookup_pid"' in nslookup and
        "lookup_remaining=5" in nslookup and "lookup_timed_out=1" in nslookup and
        "aliddns_ipv4_valid" in nslookup and "aliddns_ipv6_valid" in nslookup and
        'aliddns_nslookup 4 "$1"' in nslookup4 and 'aliddns_nslookup 6 "$1"' in nslookup6,
        "DNS fallback must isolate scratch paths, reject timeout/failed partial results, validate both families and reap only its child")
require("Concurrent calls share " in nslookup_test and "shell descendants" in nslookup_test and
        "timed-out query leaked partial output" in nslookup_test and
        "timed-out resolver child survived cleanup" in nslookup_test and
        "unrelated resolver process" in nslookup_test and
        "invalid IPv4 candidate was not skipped" in nslookup_test and
        "invalid IPv6 candidate was not skipped" in nslookup_test and
        "AliDDNS bounded, isolated DNS lookup and strict IPv4/IPv6 lifecycle regression: PASS" in nslookup_test,
        "DNS lookup host regression must cover concurrency, timeout, cleanup and strict IPv4/IPv6")

line_break = chr(10)
push_trigger = "  push:" + line_break + "    paths:" + line_break
pull_request_trigger = "  pull_request:" + line_break + "    paths:" + line_break
require(push_trigger in workflow and pull_request_trigger in workflow,
        "AliDDNS workflow must define both push and pull_request path filters")
push_filter_block = workflow.split(push_trigger, 1)[1].split(pull_request_trigger, 1)[0]
pr_filter_block = workflow.split(pull_request_trigger, 1)[1].split(
    line_break + line_break + "permissions:", 1
)[0]
push_filter_paths = tuple(line.strip() for line in push_filter_block.splitlines()
                          if line.strip().startswith('- "'))
pr_filter_paths = tuple(line.strip() for line in pr_filter_block.splitlines()
                        if line.strip().startswith('- "'))
require(push_filter_paths == pr_filter_paths and
        '- "trunk/user/aliddns/tests/aliddns_nslookup_lifecycle.sh"' in push_filter_paths and
        '- "trunk/user/aliddns/tests/aliddns_custom_script_lifecycle.sh"' in push_filter_paths and
        '- "trunk/user/aliddns/tests/copyscripts_ca_bootstrap.sh"' in push_filter_paths and
        '- "trunk/user/aliddns/tests/aliddns_hmac_sha1.sh"' in push_filter_paths and
        '- "trunk/user/aliddns/src/aliddns-hmac-sha1.c"' in push_filter_paths and
        '- "trunk/user/aliddns/src/Makefile"' in push_filter_paths and
        '- "trunk/user/aliddns/aliddns.sh"' in push_filter_paths,
        "AliDDNS push and pull_request filters must symmetrically cover production source and every host test")

initconfig_start = source.find("initconfig () {")
initconfig_end = source.find("\n}\n\ninitconfig\n", initconfig_start)
require(initconfig_start >= 0 and initconfig_end > initconfig_start,
        "could not locate persistent DDNS default-script initializer")
initconfig_body = source[initconfig_start:initconfig_end]
require('ddns_template_path="${ALIDDNS_DDNS_TEMPLATE_PATH:-/etc_ro/ddns_script.sh}"' in initconfig_body and
        'cp "$ddns_template_path" "$ddns_stage_file"' in initconfig_body and
        'chmod 755 "$ddns_stage_file"' in initconfig_body and
        'mv -f "$ddns_stage_file" "$ddns_script_path"' in initconfig_body and
        '[ ! -L "$ddns_script_path" ]' in initconfig_body and
        "trap \'rm -rf \"$ddns_stage_dir\"\' 0" in initconfig_body,
        "default initializer must seed the packaged ROMFS template through a private atomic stage and reject symlinks")
require('$(ROMFSINST) /etc_ro/ddns_script.sh' in package,
        "packaged DDNS default must be installed read-only in ROMFS")
require('ddns_script_path="/etc/storage/ddns_script.sh"' in copy_scripts and
        '[ -L "$ddns_script_path" ]' in copy_scripts and
        "cp -rf /etc_ro/ddns_script.sh /etc/storage/" not in copy_scripts,
        "boot-time script seeding must refuse persistent DDNS symlinks")
ca_install = function_body(copy_scripts, "install_ca_trust_store() {")
require('if ! tar -xzf "$copyscripts_cert_archive" -C "$copyscripts_cert_dir"; then' in ca_install and
        'if [ ! -r "$copyscripts_ca_bundle" ]; then' in ca_install and
        "copyscripts_ca_status=1" in copy_scripts and
        'exit "$copyscripts_ca_status"' in copy_scripts and
        'if (system("/usr/bin/copyscripts.sh") != 0)' in rc,
        "CA extraction must propagate tar and readable-bundle failures to the RC startup caller")
require("curl -k" not in initconfig_body and "--no-check-certificate" not in initconfig_body and
        "http://" not in initconfig_body,
        "new persistent default script must not contain insecure public-IP sources")
require("curl -k" not in source and "--no-check-certificate" not in source and
        "http://119.29.29.29" not in source,
        "stock AliDDNS source retains an insecure public-IP or DNS lookup")
require('$(ROMFSINST) -p +x $(THISDIR)/aliddns-ipv4.sh /usr/bin/aliddns-ipv4.sh' in package,
        "IPv4 helper must be installed executable in ROMFS")
require("tls-failure)" in ipv4_test and "timeout)" in ipv4_test and
        "multi-line data" in ipv4_test and
        "strict IPv4/IPv6 address validation regression: PASS" in ipv4_test,
        "public-IP regression must cover transport failures and strict address validation")

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

for marker in (
    "AliDDNS custom-script source failure and strict address validation regression: PASS",
    "AliDDNS ROMFS template seeding, atomic replacement and symlink rejection regression: PASS",
):
    require(marker in custom_script_test, f"custom-script host regression missing result marker: {marker}")

for marker in (
    "CA trust-store bootstrap success regression: PASS",
    "CA trust-store bootstrap failure propagation regression: PASS",
):
    require(marker in ca_bootstrap_test, f"CA bootstrap regression missing result marker: {marker}")

print("AliDDNS HTTPS transport, firmware trust-store, API status and RC integration contracts: PASS")
