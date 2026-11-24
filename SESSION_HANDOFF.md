# Session Handoff

## Current repository state

- Parent live `main` immediately before this atomic HTTPD security slice: `5c8c62f51b722ebeda9cbce811dd571392832a5a`. The HTML/JavaScript output handler, remaining UI migrations, regressions, workflow triggers, roadmap and handoff will be published in one complete commit; fetch the resulting exact SHA from live `main` before another write.
- Read-only backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged and never written.
- Prior migrations: `51bd80e` moved seven ASP pages to safe JS output and `453aefd` moved four more. This slice completes the audited 174-asset context pass while preserving HTML-only callsites.
- Main-only development; one complete Git tree/commit, non-forced expected-SHA ref updates, no partial file commits or force-push.

## Active task

Verify the XY-C1 regression-test path-filter correction in this checkpoint. The test file must trigger the workflow on its own while remaining classified as non-firmware input, so detector tests run but the expensive firmware Build/TRX/artifact stages skip for test-only changes. Then fix the discovered JSON string-writer newline bug in `get_nvram_list` and migrate the remaining legacy `nvram_get("rc_support")` JavaScript sink in `validator.js`.

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

- Exact `861e3fe` HTTPD Authentication and Security Regression `#45298769831`: **PASS**, including the new repository-wide JavaScript NVRAM context check and the AiDisk DOM-escaping regression. Exact AI handoff `#45298769865`, Process inventory `#45298769872`, Process lifecycle inventory `#45298769967` and NVRAM transaction audit `#45298769873`: **PASS**.
- Exact `861e3fe` XY-C1 Firmware Validation `#45298770098`: Build XY-C1 **IN_PROGRESS**; TRX verification and artifact upload **PENDING**. The older exact `5c8c62f` run `#45298161504` also remains in Build XY-C1; do not treat either as verified yet.
- Exact test-fixture correction `a2310ad` XY-C1 run `#45297887131`: **PASS** for the path-detector test. Build, TRX and artifact upload were **SKIPPED**, because that test/docs-only commit had no firmware production input. The original `453aefd` failure was confined to the test fixture's newline/NUL escaping and was corrected.
- `861e3fe` added the remaining JS-context migration, an HTML apostrophe/control encoder regression and a repository-wide template checker. The exact HTTPD run above passed; the source/UI firmware build for this SHA remains in progress. No physical-router L4 result is claimed.
- Backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Exact first action for the next GPT

1. Inspect exact-SHA Actions for the XY-C1 regression-test trigger correction. The self-test must run; Build/TRX/artifact should skip because the only firmware-related paths changed are workflow/test sources.
2. Fix `write_json_string()` in `web_ex.c`: its newline case currently emits a literal LF inside a quoted JSON string instead of the two-byte escape `\\n`. Add a GNU89 encoder regression covering quotes, backslashes, HTML script terminators, controls, UTF-8 and U+2028/U+2029; migrate the legacy `nvram_get("rc_support")` sink in `validator.js` to `nvram_get_js("", "rc_support")` and extend the output-context checker.
3. The completed 174-asset scan found no JavaScript uses of `nvram_get_list_x`, `nvram_get_buf_x` or `nvram_get_table_x`; `get_nvram_list` is the JSON-array path and uses `write_json_string`. Keep these context distinctions in regression coverage.
4. Keep the backup immutable and continue atomic commits while independent Actions are pending.

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
