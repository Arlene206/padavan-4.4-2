# Session Handoff

## Current repository state

- Reviewed production/template source baseline: `51bd80eff75efd16fc49e46ac612b2dc39ea41ae`.
- Main descends through the documentation-only checkpoint `58cbb6735d00ceb648ae1c377611f2c35e97b35d`; this correction also changes documentation only.
- Read-only backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged.
- Development commits use complete Git trees and non-forced expected-SHA updates.

## Active task

Continue the HTTPD JS-output-context audit. The seven ASP pages in `51bd80e` were changed only at JavaScript-source sinks; HTML input/attribute/text uses intentionally remain on `nvram_get_x`. Inspect remaining templates and add focused regressions for each proven JS sink.

## Completed development slices

### HTTPD / web UI security

- `09576b630122b31bed95c161223f05a43f0cdd6f`: group compaction checks transaction-locked count against caller snapshot before row mutation; malformed, stale, negative and overflow counters fail closed.
- `beb57775ac957fb79a509f8fca9f22ee74510949`: HTML-escapes removable-media folder names at every AiDisk folder-tree text/title sink; Node VM XSS regression.
- `c89d1d777494dd13384270014c080b62c1b9f56a`, `6262b52ab098f98ceeb7466261a5916c1b4ed79e`, `3e02326eae477850f0f98fe74764b59f42c64baf`, `0136ceb43575bc4367fc288a3524ead11fcad7b7`: group Add is transaction-locked, snapshots row/cache/count, clears delete caches, publishes count last, rolls back failures, distinguishes incomplete rollback and returns HTTP 500. Fault injection covers malformed/full/missing counts, count/row read errors, partial writes and rollback failure.
- `96431bbdf8dafedf38db929ef31f9990ec9a2ab8`, `2b8333e65915adbfc7e617b81ded751b7e2a253b`, `c7bcd00de2d696a38e9dcae112b712c8dcfbcbf1`: group-delete parser consumes the full raw value, rejects oversized/truncated/malformed indexes and preserves exact legacy `-1`. Regression uses a 255-digit zero-padded prefix plus untrusted suffix.
- `edc313cdf61b4c888ead94e019cc1f4c87f02ffa`: added `nvram_get_js`, a complete JS string literal callback reusing the tested JS writer; classified as `HTTPD_DIRECT_EJ_ESCAPED_OUTPUT`. Fixed stored XSS in DDNS hostname JS initialization.
- `6cb5c16818eb5a91382a97365692f58105e4b5c2`, `2a50ce0c6653de44fe071c13049747b406d49f3c`: fixed the HTTPD workflow source extraction and exact DDNS template assertion.
- `51bd80eff75efd16fc49e46ac612b2dc39ea41ae`: applied JS-context escaping to WAN, LAN, DHCP, IPTV, Services, SettingBackup and MACFilter ASP pages, with per-template workflow source assertions. HTML contexts remain on the HTML escape path.

### IPTV / process lifecycle

Independent snapshot/lifecycle changes remain in `99ad9397e3cf4450c6fc07758b58c3f7b51de115`, `d38250bfa213607dbf02ac65a75f60e933be34c9`, `b64511308e12091035a745055e2882a15addd166`, and `57c466d3001da525511160fc30418a774b99aa85`. The too-short overflow fixture and stale checker expectations were corrected; historical exact-SHA WAN/process/storage/XY-C1 results remain in earlier dev-log entries.

## Verification boundary

- Exact `51bd80e` HTTPD Authentication/Security Regression `#45296325091`: **PASS**, 51/51 steps.
- Exact `51bd80e` Process lifecycle inventory `#45296325017`: **PASS**, 5/5 steps.
- Exact `58cbb67` AI handoff contract `#45296533165`: **FAIL**, because `REFACTOR_ROADMAP.md` was 131,598 bytes, above its 131,072-byte limit. This correction reduces its size; no source/test failure was reported.
- XY-C1 Firmware Validation `#45295392702` remains **IN_PROGRESS** on production source `edc313c`, at `Build XY-C1` (37/44 steps at last poll); TRX and artifact remain pending. This SHA predates the seven ASP updates in `51bd80e`; it does not verify latest main's UI payload.
- NVRAM transaction-boundary audit `#45295392634` on `edc313c`: **PASS**, 15/15. Its audited C transaction code is unchanged in `51bd80e`.
- No local MIPS cross-build, physical router test, or hardware L3/L4 validation is claimed.

## Exact first action for the next GPT

1. Poll AI handoff validation for this documentation correction and confirm the roadmap byte-limit fix.
2. Find a supported way to run XY-C1 validation for source/template SHA `51bd80e`; verify Build, TRX output and artifact upload individually.
3. Continue scanning remaining web templates for NVRAM output inside JavaScript source; preserve HTML handling and add regression markers for each newly confirmed JS sink.
4. Continue independent fixes while Actions are pending; keep `backup/pre-plugin-overhaul-2026-11-14` read-only.

---

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
