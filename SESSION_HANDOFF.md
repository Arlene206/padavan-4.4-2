# Session Handoff

## Current repository state

- Parent live `main` immediately before this atomic HTTPD security slice: `5c8c62f51b722ebeda9cbce811dd571392832a5a`. The HTML/JavaScript output handler, remaining UI migrations, regressions, workflow triggers, roadmap and handoff will be published in one complete commit; fetch the resulting exact SHA from live `main` before another write.
- Read-only backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged and never written.
- Prior migrations: `51bd80e` moved seven ASP pages to safe JS output and `453aefd` moved four more. This slice completes the audited 174-asset context pass while preserving HTML-only callsites.
- Main-only development; one complete Git tree/commit, non-forced expected-SHA ref updates, no partial file commits or force-push.

## Active task

Verify the dynamic JavaScript hook migration and continue scanning remaining C-side `websWrite()` script emitters for runtime strings that still bypass `write_js_string()`.

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

### IPTV / process lifecycle

Independent snapshot/lifecycle changes remain in `99ad9397e3cf4450c6fc07758b58c3f7b51de115`, `d38250bfa213607dbf02ac65a75f60e933be34c9`, `b64511308e12091035a745055e2882a15addd166`, and `57c466d3001da525511160fc30418a774b99aa85`. The too-short overflow fixture and stale checker expectations were corrected; historical exact-SHA WAN/process/storage/XY-C1 results remain in earlier dev-log entries.

## Verification boundary

- Exact `dd099f6` HTTPD Authentication/Security run `#45300345684` and related checks had started. The LAN port-status contract check was narrowed in the follow-up commit to distinguish dynamic values from the literal `"No link"` fallback; the exact child SHA Actions must be checked.
- Exact `08c6d43` HTTPD `#45299995452`, AI handoff `#45299995515`, Process lifecycle inventory `#45299995326`, Process regression `#45299995463`, FRP `#45299995540`, Inadyn `#45299995467`, AliyunDrive `#45299995485`, AdGuardHome `#45299995325` and USB storage job `#45299995495`: **PASS** at last poll; NVRAM transaction audit `#45299995549` was still running. Its XY-C1 run `#45299995388` had Build pending.
- Exact `212aed2` XY-C1 `#45299517972`: detector test **PASS**, Build in progress with TRX/artifact pending. AI handoff `#45299517974` and Process inventory `#45299517987`: **PASS**.
- Exact `5c8c62f` XY-C1 `#45298161504`: **PASS**, including Build, TRX verification and artifact upload.
- No physical-router L4 result is claimed. Backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Exact first action for the next GPT

1. Verify the corrected dynamic-hook source-contract assertion on the exact new SHA and inspect HTTPD/XY-C1 Actions.
2. Continue finding any data-derived JavaScript values emitted through raw `websWrite()` format strings; route runtime strings through `write_js_string()`, keeping numeric/static output unchanged.
3. Keep backup read-only and continue atomic commits while independent Actions are pending.

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
