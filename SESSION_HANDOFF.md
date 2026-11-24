# Session Handoff

## Current repository state

- Live `main`: `c7bcd00de2d696a38e9dcae112b712c8dcfbcbf1` (verified immediately before this documentation checkpoint).
- Read-only backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged and not written.
- All source, tests, and workflow changes in this session were committed to `main` with complete Git trees and non-forced expected-SHA updates. No force-push or partial per-file commit was used.
- Earlier handoff text below the execution mandate is historical. Its old live SHA, old next task, and old CI status are superseded by this checkpoint.

## Active task

### HTTPD stability/security: transactional NVRAM group operations and bounded request parsing

Completed source slices:

- `09576b630122b31bed95c161223f05a43f0cdd6f`: NVRAM group compaction now checks the count read under the transaction lock against the caller snapshot before mutating any row. Stale/inconsistent/malformed counts fail closed. Added stale-higher/stale-lower/junk/negative/overflow and legacy-decimal regressions.
- `beb57775ac957fb79a509f8fca9f22ee74510949`: escaped removable-media folder names at every AiDisk folder-tree HTML text/attribute sink. Added a Node VM XSS regression and workflow contract. Both label paths and both title sinks are covered.
- `c89d1d777494dd13384270014c080b62c1b9f56a` plus `6262b52ab098f98ceeb7466261a5916c1b4ed79e`: replaced unchecked HTTPD NVRAM group Add writes with a single transaction-locked all-column append. It snapshots destination/cache/count cells, clears deletion caches, publishes count last, rolls back partial writes, reports incomplete rollback, and returns HTTP 500 when add fails. Follow-up corrected diagnostic newline escaping and the workflow's source-signature matcher.
- `3e02326eae477850f0f98fe74764b59f42c64baf`: added fault injection proving that an incomplete group-add rollback returns the distinct failure status rather than reporting success.
- `0136ceb43575bc4367fc288a3524ead11fcad7b7`: added a regression that a backend error reading the group count aborts before any NVRAM write.
- `96431bbdf8dafedf38db929ef31f9990ec9a2ab8` (current source carried forward to this main): group-delete request parsing no longer silently parses a truncated prefix of an oversized index value. The bounded parser preserves the exact legacy `-1` end marker and rejects malformed, negative, overflowing, out-of-range, and oversized values.
- `2b8333e65915adbfc7e617b81ded751b7e2a253b` and `c7bcd00de2d696a38e9dcae112b712c8dcfbcbf1`: fixed the workflow assertion and strengthened the truncation regression to use a valid 255-digit zero-padded index followed by an untrusted suffix. The old code would parse the truncated numeric prefix; the new bounded parser rejects the full oversized value.
- Independent IPTV/process-lifecycle follow-ups already present on main include `99ad9397e3cf4450c6fc07758b58c3f7b51de115`, `d38250bfa213607dbf02ac65a75f60e933be34c9`, `b64511308e12091035a745055e2882a15addd166`, and checker alignment `57c466d3001da525511160fc30418a774b99aa85`. The too-short IPTV overflow fixture and stale lifecycle checker expectations were corrected; see exact run statuses below.

## Verification boundary

- Exact live `main` SHA: `c7bcd00de2d696a38e9dcae112b712c8dcfbcbf1`.
- Exact-SHA HTTPD Authentication/Security Regression #45294291762: **PASS** (all 51 steps). This includes the bounded group-delete parser regression/source contract, NVRAM group compaction/add policy regression, and AiDisk folder-tree XSS regression.
- Exact-SHA Process lifecycle inventory #45294291601: **PASS**.
- Exact-SHA NVRAM transaction boundary audit #45294292010: **PENDING / IN_PROGRESS** at the last check, step “Audit raw userland ioctl paths and kernel NVRAM API callers” (12/14 completed; no failure observed). Do not treat the earlier cancelled runs #45293821282 and #45293720232 as failures.
- Product-source SHA `96431bbdf8dafedf38db929ef31f9990ec9a2ab8` has XY-C1 Firmware Validation #45294157650 **IN_PROGRESS** at “Build XY-C1” (37/44 steps completed at last check). It contains the current production-source changes; later `2b8333e` and `c7bcd00` commits changed only the workflow/test fixture, not firmware production inputs. Until Build, TRX verification, and artifact upload finish successfully on that run, product-source L3 remains **NOT VERIFIED**.
- Exact `6262b52` Actions were HTTPD #45293347938 PASS, process inventory #45293347859 PASS, fail-closed lifecycle #45293347851 PASS, storage hotplug #45293348046 PASS, NVRAM boundary #45293348079 PASS, AI handoff #45293347748 PASS, AdGuardHome #45293347875 PASS, Inadyn #45293347871 PASS, FRP #45293348404 PASS, and AliyunDrive WebDAV #45293348099 PASS. XY-C1 #45293348126 was still in progress at its Build step when superseded by the later production-source build above.
- Exact earlier verification: `57c466d` process lifecycle #45292778140 PASS, storage hotplug #45292778219 PASS, and XY-C1 #45292778199 PASS; `b645113` WAN route #45292528186, process inventory #45292528141, and NVRAM audit #45292528207 PASS. These are historical exact-SHA results, not claims that those older SHAs equal current main.
- No local MIPS cross-build, physical router test, or hardware L4 validation is claimed.

## Exact first action for the next GPT

1. Poll NVRAM transaction audit #45294292010 on exact main `c7bcd00de2d696a38e9dcae112b712c8dcfbcbf1`; record its terminal result and inspect logs only if it fails.
2. Poll XY-C1 run #45294157650 on production-source SHA `96431bbdf8dafedf38db929ef31f9990ec9a2ab8`. Confirm Build, TRX check, and artifact upload individually before claiming L3.
3. Continue an independent userland defect investigation while either check is pending. A concrete next candidate is to trace all `ej_nvram_get_x()` call sites and verify their HTML/attribute/JavaScript output contexts before deciding whether the general NVRAM output helper needs additional context-specific escaping. Do not change it based only on the missing apostrophe escape; first demonstrate a vulnerable sink and add a regression.
4. Keep working on `main`, use one complete tree and one atomic commit per logical fix, and check Actions for its exact SHA.

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
