# Session Handoff

## Current repository state

- Live `main`: `2a50ce0c6653de44fe071c13049747b406d49f3c`.
- Read-only backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged, not written.
- This checkpoint follows source commit `edc313cdf61b4c888ead94e019cc1f4c87f02ffa`, workflow correction `6cb5c16818eb5a91382a97365692f58105e4b5c2`, and final workflow assertion correction `2a50ce0c6653de44fe071c13049747b406d49f3c`.
- All logical changes were published with complete Git trees and non-forced expected-SHA updates. No force-push or partial file commit.

## Completed development slices

### HTTPD stability / security

- `09576b630122b31bed95c161223f05a43f0cdd6f`: group compaction validates the transactional count against the caller snapshot before modifying rows; malformed, stale, negative, and overflow counts fail closed.
- `beb57775ac957fb79a509f8fca9f22ee74510949`: AiDisk removable-media folder names are HTML-escaped at every folder-tree label and title-attribute sink; Node VM regression covers XSS payloads.
- `c89d1d777494dd13384270014c080b62c1b9f56a`, `6262b52ab098f98ceeb7466261a5916c1b4ed79e`, `3e02326eae477850f0f98fe74764b59f42c64baf`, `0136ceb43575bc4367fc288a3524ead11fcad7b7`: NVRAM group Add is transaction-locked, snapshots destination/cache/count values, clears deletion caches, publishes count last, rolls back partial writes, returns a distinct incomplete-rollback result, and propagates failure to HTTP 500. Fault injection covers missing/malformed/full counts, count/destination read errors, row write failure, count write applied-then-failed, and incomplete rollback.
- `96431bbdf8dafedf38db929ef31f9990ec9a2ab8` plus `2b8333e65915adbfc7e617b81ded751b7e2a253b` and `c7bcd00de2d696a38e9dcae112b712c8dcfbcbf1`: group-delete indexes are parsed from the full raw request value; no silent fixed-buffer truncation before `strtol`. Exact legacy `-1` terminator is retained. Regression uses a 255-digit zero-padded numeric prefix followed by an untrusted suffix.
- `edc313cdf61b4c888ead94e019cc1f4c87f02ffa`: fixed stored XSS in DDNS hostname JavaScript initialization. `ddns_hostname_x` was emitted into a single-quoted JS literal through HTML-only `nvram_get_x`; an apostrophe could terminate the script string. Added `nvram_get_js`, which uses the tested JavaScript string writer, marked it `HTTPD_DIRECT_EJ_ESCAPED_OUTPUT`, added registry/authorization/source-contract coverage, and switched only the JS sink in `Advanced_DDNS_Content.asp`. The separate double-quoted HTML input continues using HTML escaping.
- `6cb5c16818eb5a91382a97365692f58105e4b5c2` and `2a50ce0c6653de44fe071c13049747b406d49f3c`: corrected the HTTPD workflow's new source-contract extraction and exact template whitespace assertion. Those commits change workflow checks only, not firmware source.

### IPTV / process lifecycle

The independent IPTV snapshot/lifecycle work remains on main: `99ad9397e3cf4450c6fc07758b58c3f7b51de115`, `d38250bfa213607dbf02ac65a75f60e933be34c9`, `b64511308e12091035a745055e2882a15addd166`, `57c466d3001da525511160fc30418a774b99aa85`. The too-short oversized altnet fixture and stale checker expectations were corrected. Earlier exact-SHA WAN route, process, storage, and XY-C1 checks are recorded in the historical log below.

## Verification boundary

- Exact current-main HTTPD Authentication/Security Regression `#45295707346`: **PASS**.
- Exact current-main AI handoff documentation contract `#45295707326`: **PASS**.
- NVRAM transaction-boundary audit `#45295392634` on source SHA `edc313cdf61b4c888ead94e019cc1f4c87f02ffa`: **PASS**, 15/15 steps.
- XY-C1 Firmware Validation `#45295392702` on the same production-source SHA: last observed **IN_PROGRESS** at `Build XY-C1` (37/44 steps); TRX verification and artifact upload had not started. Do not claim product L3 until Build, TRX verification, and artifact upload all pass.
- The HTTPD run on `edc313...` initially failed its new static contract because it used a nonexistent helper; `6cb5...` fixed extraction but exposed an exact whitespace mismatch; `2a50...` corrected that assertion. The final exact-main HTTPD run above passed. These transient failures are not hidden.
- No local MIPS cross-build, physical router test, or hardware L4 validation is claimed.

## Exact first action for the next GPT

1. Poll XY-C1 run `#45295392702` for production source SHA `edc313cdf61b4c888ead94e019cc1f4c87f02ffa`. Confirm Build, TRX verification, and artifact upload individually.
2. Continue the remaining HTTPD output-context audit: enumerate other `nvram_get_x()` calls embedded in JavaScript strings or script blocks. The DDNS hostname sink is fixed; do not replace every HTML-context use indiscriminately. For each candidate, prove the output context and add a focused regression before changing it.
3. Continue independent userland stability/security work while firmware CI is running. Keep atomic commits and exact-SHA Actions checks. Keep the backup branch read-only.

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
