# Session Handoff

## Current repository state

- Current live main parent for this slice: `674adb3bdbe4d19a8de684561317f349b1e1068a`.
- Exact `1027e90` HTTPD `#45302330035` and AI handoff `#45302330042`: **PASS**; the corrected source-contract verifies all 48 AiDisk dictionary-backed JavaScript error responses.
- Exact `a5794a0` WOL DOM XSS: HTTPD `#45301947513`, AI `#45301947575` and process inventory `#45301947519`: **PASS**. Its XY-C1 `#45301947500` was IN_PROGRESS at last poll.
- Exact `cd7e54d` and `674adb3` HTTPD runs `#45302551766` and `#45302709748`: **FAIL** at `check_aidisk_usb_string_outputs()` because source boundaries searched for literal backslash-n text, not function/#endif line boundaries. All earlier HTTPD sanitizer, NVRAM context, WOL DOM and AiDisk message checks printed PASS; syslog assertions were not reached. This checker-only correction is the current commit.
- Exact `674adb3` AdGuardHome `#45302709648`, AliyunDrive `#45302709567`, process inventory `#45302709442`, Inadyn `#45302709395` and AI handoff `#45302709411`: **PASS**. FRP `#45302709418`, USB storage `#45302709472`, process regression `#45302709713`, NVRAM audit `#45302709430` and XY-C1 `#45302709412` were IN_PROGRESS at last poll.
- Syslog HTML escaping is implemented in `674adb3`; its source-contract test awaits execution after the preceding AiDisk USB checker defect is corrected.
- Read-only backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged and never written. Work only on `main`; never force-push or write to backup.

## Active task

Correct the false-negative in `check_aidisk_usb_string_outputs()`: its Python source-boundary strings were double-escaped and searched for the literal two-character sequence `\n`. Use actual Python newline escapes so it extracts `int\nej_get_usb_ports_info()` through the following `#endif`. Then re-run the exact-head HTTPD regression to reach and verify the syslog output contract as well as the USB enum-string check.

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

- AiDisk JavaScript output hardening (code committed in `eae527b` and `cd7e54d`): all 48 dictionary-backed action-error messages are serialized through `write_script_message()` and the shared JSON encoder; USB enum and modem transport labels now use the same string encoder. Exact `1027e90` HTTPD/AI checks pass; the `cd7e54d` child Actions remain pending.

### IPTV / process lifecycle

Independent snapshot/lifecycle changes remain in `99ad9397e3cf4450c6fc07758b58c3f7b51de115`, `d38250bfa213607dbf02ac65a75f60e933be34c9`, `b64511308e12091035a745055e2882a15addd166`, and `57c466d3001da525511160fc30418a774b99aa85`. The too-short overflow fixture and stale checker expectations were corrected; historical exact-SHA WAN/process/storage/XY-C1 results remain in earlier dev-log entries.

## Verification boundary

- Exact `1027e90` HTTPD `#45302330035` and AI handoff `#45302330042`: **PASS**, including all 48 AiDisk dictionary-message checks.
- Exact `a5794a0` HTTPD `#45301947513`, AI `#45301947575` and process inventory `#45301947519`: **PASS**. XY-C1 `#45301947500` remains IN_PROGRESS at last poll.
- Exact `cd7e54d` HTTPD `#45302551766` and exact `674adb3` HTTPD `#45302709748`: **FAIL** only at the AiDisk USB source-boundary assertion. HTML/JSON sanitizer, JavaScript NVRAM, dynamic hook, WOL DOM and AiDisk dictionary-message checks all printed PASS; the failure is repaired in this checker-only slice. Syslog source-contract test did not run on those SHAs.
- Exact `674adb3` AdGuardHome `#45302709648`, AliyunDrive `#45302709567`, process inventory `#45302709442`, Inadyn `#45302709395` and AI handoff `#45302709411`: **PASS**. Other listed runs had not completed at checkpoint.
- The syslog code/textarea-payload regression are in parent `674adb3`; the next HTTPD SHA must execute the corrected USB assertion and the syslog contract. No local execution, same-SHA L3 firmware result or hardware L4 validation is claimed.
- Backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Exact first action for the next GPT

1. Verify the exact-head HTTPD regression after this USB checker escape fix; it must reach both AiDisk USB string coverage and the new syslog HTML-output assertions.
2. Check exact-head XY-C1 Build/TRX/artifact and NVRAM/USB/process Actions without waiting.
3. Continue auditing dynamic HTTPD output producers for values from files, NVRAM, networkmap, device metadata and translated messages; preserve the archival backup.

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
