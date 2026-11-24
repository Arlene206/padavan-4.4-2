# Session Handoff

## Current repository state

- Parent `main` for the AliyunDrive post-launch readiness cleanup: `dd0b374694cf9d1e99ca65c8b35fa53b482d377b`; live branch was checked immediately before building this atomic commit. Archival `backup/pre-plugin-overhaul-2026-11-14` remains frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- RAETH compile-integrity correction `dd0b374694cf9d1e99ca65c8b35fa53b482d377b`: exact-head Switch CLI #45312564409 **PASS** (new unique-definition/signature contract passed) and AI docs #45312564408 **PASS**. NVRAM #45312564407 and XY-C1 #45312564566 were **IN_PROGRESS** at last observation; the target build result is not yet known.
- The previous GE2 SGMII fix on `edb529c4baf1e60df1fd542a4e8192cf608ffbdc` passed Switch CLI/AI/NVRAM but XY-C1 failed at RAETH compilation. The duplicate-definition and stale-prototype corrections are now in `dd0b374`; do not transfer parent results or claim L3 until same-SHA Build/TRX/artifact complete.
- New userland lifecycle correction: when the daemon launched by the current start call remains executable-confirmed RUNNING but never serves a valid HTTP response, clean it up using the existing bounded stop routine. Existing pre-existing unhealthy processes and UNKNOWN executable states must remain untouched.

## Active task

Verify the AliyunDrive failed-start cleanup regression on its exact `main` SHA. Confirm the lifecycle Action proves a newly launched but permanently HTTP-503 daemon receives bounded TERM cleanup, while a pre-existing unhealthy process and UNKNOWN state remain fail-closed. In parallel, check XY-C1 Build/TRX/artifact on this same SHA for the RAETH compile fix; diagnose any new compiler failure before moving on.

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

- Parent `dd0b374694cf9d1e99ca65c8b35fa53b482d377b`: Switch CLI #45312564409 **PASS**, AI docs #45312564408 **PASS**, NVRAM #45312564407 **IN_PROGRESS**, XY-C1 #45312564566 **IN_PROGRESS**. These parent results do not validate AliyunDrive.
- The AliyunDrive startup cleanup in THIS COMMIT has not yet run in CI. No new firmware/TRX/artifact or hardware result is claimed.

## Exact first action for the next GPT

1. Fetch current `main` and inspect this commit's exact-SHA AliyunDrive lifecycle and AI handoff Actions; verify the test log includes the 503 startup-timeout cleanup case.
2. Check the exact same SHA's XY-C1 kernel build. Confirm `raether.o` passes, then verify full firmware Build, TRX and artifact upload; do not carry forward older-SHA results.
3. If both checks pass, resume the next highest-risk independent stability issue; keep unknown process states fail-closed and leave the archival backup unchanged.


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
