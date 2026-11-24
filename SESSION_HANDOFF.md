# Session Handoff

## Current repository state

- Live `main` parent for this WAN counter regression harness correction: `b757cc5ad9f1173d2a476475ef66ffb6be6ff8d7`.
- Exact `b757cc5` WAN sysfs byte-counter regression `#45308457252` **FAILED** at compile setup: the test includes `net_ifstats_policy.h` from the shared source directory, but the workflow omitted `-I trunk/user/shared/src`. No C assertions or integration contract ran; this is a test build-command issue, not a source-test result.
- Exact `b757cc5` AI handoff contract `#45308457405`: **PASS**. Other exact-push statuses at latest poll: `WAN DHCP callback state stability regression` #45308457813: **success**; `detect_link signal lifecycle stability regression` #45308457811: **success**; `Wireless radio state regression` #45308457695: **success**; `AdGuardHome config security regression` #45308457694: **success**; `USB modem WDM/NDIS physical identity stability` #45308457689: **success**; `miniupnpd atomic configuration publication regression` #45308457667: **success**; `NVRAM transaction boundary audit` #45308457643: **in_progress**; `USB storage hotplug stability regression` #45308457640: **success**; `watchdog signal lifecycle stability regression` #45308457629: **success**; `Inadyn fail-closed lifecycle regression` #45308457628: **success**; `VPN server lifecycle stability regression` #45308457613: **success**; `Process lifecycle inventory - b757cc5ad9f1173d2a476475ef66ffb6be6ff8d7` #45308457608: **success**; `FRP asynchronous startup lifecycle regression` #45308457582: **success**; `IPv6 service-type checked-read regression` #45308457574: **success**; `Firewall Startup Stability Regression` #45308457562: **success**; `WAN route operation status regression` #45308457554: **success**; `AliyunDrive WebDAV lifecycle stability regression` #45308457536: **in_progress**; `Router mode NVRAM update failure regression` #45308457499: **success**; `XY-C1 Firmware Validation` #45308457490: **in_progress**; `WAN gateway ARP probe socket stability regression` #45308457331: **success**; `Process inventory and lifecycle fail-closed regression` #45308457314: **success**; `ZeroTier lifecycle stability regression` #45308457305: **success**; `HTTPD Authentication and Security Regression` #45308457290: **success**; `WAN interface teardown stability regression` #45308457262: **success**; `DNSMasq lifecycle stability regression` #45308457258: **success**; `detect_internet signal lifecycle stability regression` #45308457232: **success**.
- The `b757cc5` source change adds checked RX/TX counter reads, preserves legacy scalar wrappers, avoids replacing WAN baselines on partial/error reads, and exports tri-state status to the page (numeric zero / N/A / hidden).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Active task

Re-run the WAN sysfs counter policy regression after adding the shared-source include directory to the host compiler invocation. Then inspect the source-contract step and exact-SHA dependent checks; do not infer source regression status from the previous compile-setup failure.

## Completed development slices

### HTTPD / web UI security

- `09576b630122b31bed95c161223f05a43f0cdd6f`: group compaction checks transaction-locked count against caller snapshot before row mutation; malformed, stale, negative and overflow counters fail closed.
- `beb57775ac957fb79a509f8fca9f22ee74510949`: HTML-escapes removable-media folder names at every AiDisk folder-tree text/title sink; Node VM XSS regression.
- `c89d1d777494dd13384270014c080b62c1b9f56a`, `6262b52ab098f98ceeb7466261a5916c1b4ed79e`, `3e02326eae477850f0f98fe74764b59f42c64baf`, `0136ceb43575bc4367fc288a3524ead11fcad7b7`: group Add is transaction-locked, snapshots row/cache/count, clears delete caches, publishes count last, rolls back failures, distinguishes incomplete rollback and returns HTTP 500. Fault injection covers malformed/full/missing counts, count/row read errors, partial writes and rollback failure.
- `96431bbdf8dafedf38db929ef31f9990ec9a2ab8`, `2b8333e65915adbfc7e617b81ded751b7e2a253b`, `c7bcd00de2d696a38e9dcae112b712c8dcfbcbf1`: group-delete parser consumes the full raw value, rejects oversized/truncated/malformed indexes and preserves exact legacy `-1`. Regression uses a 255-digit zero-padded prefix plus untrusted suffix.
- `edc313cdf61b4c888ead94e019cc1f4c87f02ffa`: added `nvram_get_js`, a complete JS string literal callback reusing the tested JS writer; classified as `HTTPD_DIRECT_EJ_ESCAPED_OUTPUT`. Fixed stored XSS in DDNS hostname JS initialization.
- `6cb5c16818eb5a91382a97365692f58105e4b5c2`, `2a50ce0c6653de44fe071c13049747b406d49f3c`: fixed the HTTPD workflow source extraction and exact DDNS template assertion.
- `51bd80eff75efd16fc49e46ac612b2dc39ea41ae`: applied JS-context escaping to WAN, LAN, DHCP, IPTV, Services, SettingBackup and MACFilter ASP pages, with per-template workflow source assertions. HTML contexts remain on the HTML escape path.

- NVRAM HTML/JavaScript output-context hardening (this commit): finished the 174-asset audit by migrating the remaining 89 JS-source `nvram_get_x` sinks; all 117 such sinks are now protected across `51bd80e`, `453aefd` and this slice. The three DDNS JS callsites use the JS-string writer, and generic HTML NVRAM output escapes apostrophes. A GNU89 ASan/UBSan byte regression and a source-template context checker are wired into HTTPD push/PR CI.

- WOL UI DOM XSS hardening (this commit): build client rows as DOM elements and render client MAC/name and vendor names only through jQuery `.text()`. Vendor API results are normalized to a string before rendering/caching. The HTTPD checker now rejects raw row concatenation of client data and `.html()` sinks for vendor labels.
- Syslog HTML output hardening (`674adb3`, verified by `28613f4`): route `/tmp/syslog.log` emitted by both `nvram_dump()` and `dump_syslog()` through the shared HTML byte encoder. The regression includes a `</textarea><script>` payload; exact HTTPD `#45302967232` and AI handoff `#45302967200` on `28613f4`: **PASS**.
- Language-option HTML output hardening (`a01de6b`): dictionary-derived labels pass through `write_html_text()`, while `<option value>` uses `write_html_attribute()`. Exact HTTPD `#45303339945` and AI `#45303340000`: **PASS**.

- Wireless scan/BSSID JSON hardening (`8f1d1d5`): serialize nearby AP SSIDs, BSSID/channel/signal strings and both NVRAM-sourced BSSID JS values using `httpd_json_escape_next()`; the AP-client caption now uses a constant `%s` format. Exact HTTPD `#45304169207`, AI `#45304169314` and process inventory `#45304169499`: **PASS**.
- Uptime format/output hardening (`f3c83db`): capture `rfctime()` and serialize it through `write_html_text()` rather than passing a dynamic value as `websWrite()`'s format argument. Exact HTTPD `#45303585359` and AI `#45303585354`: **PASS**. All non-XY-C1 gates on this SHA pass; XY-C1 `#45303585432` remains IN_PROGRESS.
- AiDisk JavaScript output hardening (code committed in `eae527b` and `cd7e54d`): all 48 dictionary-backed action-error messages are serialized through `write_script_message()` and the shared JSON encoder; USB enum and modem transport labels now use the same string encoder. Exact `1027e90` HTTPD/AI checks pass; the `cd7e54d` child Actions remain pending.

### IPTV / process lifecycle

Independent snapshot/lifecycle changes remain in `99ad9397e3cf4450c6fc07758b58c3f7b51de115`, `d38250bfa213607dbf02ac65a75f60e933be34c9`, `b64511308e12091035a745055e2882a15addd166`, and `57c466d3001da525511160fc30418a774b99aa85`. The too-short overflow fixture and stale checker expectations were corrected; historical exact-SHA WAN/process/storage/XY-C1 results remain in earlier dev-log entries.

## Verification boundary

- Exact `b757cc5` WAN run `#45308457252`: **FAIL** at compile setup only: missing `-I trunk/user/shared/src`. No host assertions or source-contract checks executed.
- Exact `b757cc5` AI handoff `#45308457405`: **PASS**. Other Process/HTTPD/NVRAM/XY-C1 results are recorded above with their exact run IDs/statuses.
- This is a workflow-only include-path correction; no new production C, firmware L3 or hardware L4 result is claimed.

## Exact first action for the next GPT

1. Verify the corrected WAN sysfs regression on this commit's exact SHA; if compilation passes, repair any actual policy or source-contract failure.
2. Reconcile exact `b757cc5` Process, HTTPD, NVRAM and XY-C1 statuses by run ID; continue independent work while any remain pending.
3. Continue auditing other source-proven resource/error paths. Keep the frozen backup read-only.


# Cross-session execution mandate

Treat this repository as one continuous engineering assignment across GPT conversations. Recover once, then execute. Work only on `main`; each logical source or CI change is one atomic main commit: create all changed blobs and one complete Git tree before creating that commit, then update `main` using a non-forced expected-SHA compare-and-swap. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop

- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task. Never claim unverified work.
