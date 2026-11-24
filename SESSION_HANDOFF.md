## Current repository state

- Active branch: `main`; source parent for this WAN first-sample slice: `c954e19234f35f23396d7db4f202249cabae9fc4`. Re-fetch live main and tree before every write.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable, never develop on or modify it.
- No open PRs.
- MTK AP private-ioctl safety is L3 VERIFIED on exact source SHA `d33fcf06b73fbe2771ab0b559116d3e750dc4918`; MTK #45384680997, NVRAM #45384681048, AI #45384680982 and XY-C1 #45384680986 passed Build/TRX/artifact. Physical L4 unverified.
- HTTPD protocol-selector guard is L3 VERIFIED on exact source SHA `659b91b497b5cf8073b54feab849a91623092c70`: HTTPD #45388114470, AI #45388114468, Process #45388114507, XY-C1 #45388114515 all PASS including Build/TRX/artifact. Physical L4 unverified.
- WAN source PHY selector range guard is L3 VERIFIED on exact production source SHA `d3733ecbec2e0cf652c00eadf1db4645ae5ff13e`: detect-link #45390205127 host regression initially surfaced a checker-only assertion later corrected in #45390666226; AI #45390666278, Process #45390666391, NVRAM #45390666354 PASS on descendant `c954e19`; XY-C1 #45390205170 on the exact production SHA d373 completed PASS including Build XY-C1, TRX verification and artifact upload. On c954 the XY-C1 job #45390666372 was host-only and skipped Build/TRX/artifact because c954 changed only tests/workflow/docs. Physical L4 unverified.

## Active task — Do not publish an initial unknown WAN sample as link-down

- In `detect_link.c`, `dl_status_wan` starts at zero while `dl_status_wan_nvram_old` starts at -1. If the first switch PHY query fails, the caller correctly retains the cached status and sets refresh retry pending, but it still called `dl_handle_link_wan()`; that published the synthetic initial zero to NVRAM before any successful physical sample. This can expose a false initial link-down state while the actual status is unknown.
- THIS COMMIT introduces a tri-state known flag driven only by nonnegative PHY results. WAN handler/NVRAM publication is deferred until the first successful sample; once known, later read failures keep the last valid state and existing retry behavior. WISP, LAN and USB processing remain independent.
- A GNU89/Werror/ASan/UBSan regression in `detect_link_nvram_policy.c` covers unknown retention for negative samples, transition to known on valid down/up samples, and persistence of known state across later errors. The detect-link source contract verifies successful-sample update and handler gating.
- The WAN selector guard remains L3 verified on d373 as above; this new production change needs its own exact-head detect-link and XY-C1 Build/TRX/artifact checks.

## Verification boundary

- THIS COMMIT contains source, policy, regression, integration contract and handoff/log update in one atomic tree.
- Verify exact-head detect-link, Process, NVRAM, AI docs and XY-C1. L3 for this first-sample guard requires same-SHA Build, TRX and artifact PASS.
- Physical router L4 remains unverified.

## Exact first action for the next GPT

1. Fetch current `main` and query push Actions by its exact SHA; inspect detect-link first-sample policy regression/source contract, Process, NVRAM, AI docs and XY-C1 Build/TRX/artifact.
2. Fix any first causal exact-head failure atomically; no L3 claim while any firmware step is pending or skipped.
3. Continue the next highest-priority independent stability/security issue; do not repeat completed MTK, HTTPD or WAN-selector slices absent new evidence.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures, or a changed first action. Do not manufacture documentation churn or no-op commits.

A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Recover once from live repository state and execute development on `main`. Never create or use a development branch or PR; the frozen backup is immutable.

For each multi-file logical slice, create all blobs, one complete tree based on the latest main tree and one commit with the exact main parent, then update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit. Never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue implementation rather than repeating completed audits or stopping at planning. PASS advances; FAIL is investigated and fixed; PENDING does not block independent tasks. Preserve Linux 4.4, GNU89, MIPS/uClibc, router compatibility, security and configuration.

Before context exhaustion, finish the current safe slice and record exact latest main SHA, tests, CI states, unresolved issues and one concrete first action for the next GPT. At session end, inspect live main and exact-head push Actions; update this handoff only when active task or first action materially changes, append substantive milestones to `dev_log.md`, and update roadmap only for material module/status/priority changes. Do not create no-op documentation commits.
