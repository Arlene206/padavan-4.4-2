# Session Handoff

## Current repository state

- Current live main parent for this slice: `fbbedd70659f2822ee4c4613342acda2c4360c82`.
- Exact `fbbedd7` HTTPD Authentication/Security Regression `#45301473424` and AI handoff contract `#45301473510`: **PASS**. The WOL JSON response encoder and corrected source-range regression both passed.
- Exact `fd68cee` HTTPD run `#45301344235`: **FAIL** because the new WOL source checker searched for literal backslash-n sequences instead of function-boundary newlines; corrected in `fbbedd7`. Its AI handoff `#45301344195`, NVRAM audit `#45301344189`, FRP `#45301344338`, Inadyn `#45301344311`, USB storage `#45301344302`, Process regression `#45301344737`, Process inventory `#45301344475` and AdGuardHome `#45301344973`: **PASS**. AliyunDrive `#45301344287` and XY-C1 `#45301344269` were still IN_PROGRESS at last poll; no L3 result is claimed.
- Read-only backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged and never written.
- Main-only, one complete Git tree/commit, non-forced expected-SHA update. Never force-push or write to backup.

## Active task

An additional output-context defect was found in AiDisk. All 48 action-error responses formatted dictionary messages directly into JavaScript calls; they are being routed through the shared JSON encoder, with a regression requiring every dictionary message to use the safe helper. The WOL DOM text-sink fix and checker from parent a5794a0 are pending exact-SHA verification.
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

### IPTV / process lifecycle

Independent snapshot/lifecycle changes remain in `99ad9397e3cf4450c6fc07758b58c3f7b51de115`, `d38250bfa213607dbf02ac65a75f60e933be34c9`, `b64511308e12091035a745055e2882a15addd166`, and `57c466d3001da525511160fc30418a774b99aa85`. The too-short overflow fixture and stale checker expectations were corrected; historical exact-SHA WAN/process/storage/XY-C1 results remain in earlier dev-log entries.

## Verification boundary

- Exact fbbedd7 HTTPD #45301473424 and AI handoff #45301473510: PASS, including the corrected WOL JSON source-boundary checker.
- Exact a5794a0 WOL UI DOM XSS change: HTTPD #45301947513, AI handoff #45301947575, Process lifecycle inventory #45301947519 and XY-C1 #45301947500 were IN_PROGRESS at checkpoint; no result claimed.
- Exact 89128ba HTTPD #45300665513 and all associated AI/process/NVRAM/FRP/Inadyn/AdGuardHome/USB/AliyunDrive regression jobs: PASS. Its XY-C1 #45300665497 detector PASS; Build IN_PROGRESS; TRX/artifact PENDING at last check.
- Exact dd099f6 HTTPD #45300345684 failed due to an over-broad source-contract assertion; c6f5805 corrected it, and HTTPD #45300471389 and AI #45300471279 passed.
- Exact 08c6d43 HTTPD #45299995452 and its AI/process/security side regression jobs passed. Exact 5c8c62f XY-C1 #45298161504 and 861e3fe XY-C1 #45298770098 passed Build, TRX and artifact upload.
- No physical-router L4 result is claimed. Backup backup/pre-plugin-overhaul-2026-11-14 remains read-only at 72f4f157a084f8184b9597ca5bf334b8080e5807.

## Exact first action for the next GPT

1. Verify exact-SHA HTTPD coverage for AiDisk: all 48 dictionary-backed error callbacks must use write_script_message(), which emits a complete JSON-encoded JavaScript string.
2. Review remaining AiDisk JavaScript data emitters (USB enumeration, printer/manufacturer/model lists and initial disk mapping) for runtime strings still bypassing the common encoder.
3. Check exact-head XY-C1 detector/build/TRX/artifact status without waiting; keep the backup immutable.

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
