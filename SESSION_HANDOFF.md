## Current repository state

- Active branch: `main`; parent for this slice: `5a7b2c4a625a5ebdae1e30494680875157848533`. Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. No open PRs.
- MTK AP private-ioctl and HTTPD selector fixes remain L3 verified on `d33fcf0` and `659b91b`; WAN source-PHY range guard is L3 verified on `d3733ec`, all with exact-SHA firmware Build/TRX/artifact PASS.
- WAN first-sample UNKNOWN guard: detect-link #45392147396, AI #45392146896, Process #45392147378/#45392146934, HTTPD #45392147738, NVRAM #45392147111 and USB modem #45392146933 PASS on `18af4fb`; same-source firmware L3 not recorded.
- SYN-cookie selector whitespace/sign fix on `5a7b2c4`: WAN route #45394597807, AI docs #45394597880, Process inventory #45394597845 and NVRAM audit #45394598006 PASS. XY-C1 #45394597932 is building; TRX/artifact pending. VSFTPD hidden-share and EPSV ALL fixes are included in that firmware candidate and remain below L3 until same-SHA Build/TRX/artifact pass.

## Active task — Fail closed on Ralink regspec NVRAM read errors

- `symlink_sku_file()` read optional `regspec_code` via `nvram_safe_get()`, collapsing an absent key and backend failure into the same empty-string fallback. A transient NVRAM error could silently select a different regulatory SKU rather than abort config generation.
- Replaced it with `nvram_get_checked()`: missing key preserves existing fallback, a present value is table-validated before path construction, and read/API errors fail the generator. Added a pure status-policy regression and wireless workflow source-order contract. The Ralink source and helper/test files are already in wireless stability and XY-C1 path filters.

## Verification boundary

- Parent `5a7b2c4`: GNU89 SYN-cookie policy regression/source checker, AI docs, Process inventory and NVRAM audit PASS. XY-C1 Build running; TRX/artifact pending.
- New Ralink wireless regression/source contract, AI docs, Process/NVRAM audits and XY-C1 production Build/TRX/artifact must be checked on its exact SHA. No L3 claim before firmware evidence. Physical-router L4 remains unverified.

## Exact first action for the next GPT

1. Fetch live `main` and query Actions for its exact head SHA.
2. Inspect Ralink wireless configuration regression/source contract for checked `regspec_code` semantics; fix any causal failure atomically.
3. Inspect XY-C1 Build/TRX/artifact for this production SHA and the prior VSFTPD/SYN-cookie changes; then continue an independent stability/security issue.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures, or a changed first action. Do not manufacture documentation churn or no-op commits.

A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Recover once from live repository state and execute development on `main`. Never create or use a development branch or PR; the frozen backup is immutable.

For each multi-file logical slice, create all blobs, one complete tree based on the latest main tree and one commit with the exact main parent, then update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit. Never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue implementation rather than repeating completed audits or stopping at planning. PASS advances; FAIL is investigated and fixed; PENDING does not block independent tasks. Preserve Linux 4.4, GNU89, MIPS/uClibc, router compatibility, security and configuration.

Before context exhaustion, finish the current safe slice and record exact latest main SHA, tests, CI states, unresolved issues and one concrete first action for the next GPT. At session end, inspect live main and exact-head push Actions; update this handoff only when active task or first action materially changes, append substantive milestones to `dev_log.md`, and update roadmap only for material module/status/priority changes. Do not create no-op documentation commits.
