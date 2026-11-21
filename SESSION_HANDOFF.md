# Cross-session execution mandate — 2026-11-21 checkpoint

## Cross-session execution mandate

- Continue development on `main` only. Every logical change is one atomic main commit: create every blob, one complete Git tree, one commit, then update main without force using the exact expected parent SHA.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Add focused regression coverage for source fixes; do not weaken checks.
- PENDING Actions do not block independent work. Diagnose the earliest causal failure log.
- Before context exhaustion, finish the current safe slice, publish atomically and leave the exact first next action here.

## Mandatory handoff checkpoint — end-of-session accuracy

The checkpoint is mandatory; a document edit is conditional. At every session end, compare live `main`, relevant exact-SHA Actions, the product-source baseline, verification boundary, blockers and the first next action with this record. Update it only when one of those resumable facts materially changes; do not create a no-op commit.

## Current repository state

- Live `main` before this workflow-indentation correction: `20e139d230b73eabc5f25a2b17d56a3a5768af11`. Current production-source baseline is `8a55d24a4b797c96ca94bc8ece112966d86fa204`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- On exact SHA `20e139d2`, AI handoff `#44887656223` and Process inventory `#44887656234` completed SUCCESS. The standalone Process workflow launch record `#44887655190` failed before creating a job because the newly inserted parser step had one extra leading space, invalidating YAML structure. This correction restores the normal six-space step indentation and eight-space run key; a new exact-head Process workflow is required to validate parsing and execute the test.
- XY-C1 `#44885238962` completed SUCCESS on exact source SHA `8a55d24a4b797c96ca94bc8ece112966d86fa204`: firmware build, TRX output verification and artifact upload all passed. L3 is VERIFIED for that source SHA; physical hardware L4 remains NOT VERIFIED.
- The previous `kill_pidfile_s()` used `fgets` plus `strtoul(..., NULL, 0)`, accepted numeric prefixes with trailing garbage, and cast before range-checking. The compatibility helper remains exported for PID-only files. This source patch adds nonblocking open, regular-file validation and bounded full-record parsing while preserving historical decimal/octal/hex syntax and surrounding whitespace; malformed/NUL/overflow values are rejected before signal delivery.
- Audit coverage found no direct old-helper calls in 383 selected active-package C files and separately examined RC/shared/HTTPD code. This is not a repository-wide exhaustive result; continue remaining package scanning after validating this fix.

## Active task

1. Validate exact-head Process lifecycle Actions after this YAML correction. Confirm workflow parses, standalone GNU89/ASan/UBSan parser regression runs, and source integration checker passes; fix the first causal failure without weakening coverage.
2. Continue remaining high-value PID-file caller/producer scan in unreviewed packages (including LPRng, u2ec, DNS and other daemons). Map format, path, owner and signal semantics before migration; preserve numeric-only external records.
3. Keep XY-C1 L3 evidence tied to `8a55d24a`; no physical L4 claim.

## Verification boundary

- Source helper and parser tests were committed on `20e139d2`; the local standalone test compiled and ran under GNU89, warnings-as-errors, ASan and UBSan: PASS.
- Repository integration is not yet validated: the exact commit's standalone Process workflow failed at YAML parsing before job creation. The other exact-SHA Process inventory and AI handoff runs passed.
- XY-C1 firmware L3 is VERIFIED on source SHA `8a55d24a`; physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main` and confirm the frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. Inspect the new exact-head Process workflow run for the indentation repair; confirm the parser test and source-contract checks pass, then continue scanning remaining packages for old PID-only helper callers/producers.

## Handoff maintenance

- Keep `dev_log.md` newest-first; archive only at 120 root records, retaining newest 20.
- Update `REFACTOR_ROADMAP.md` only when module status, evidence or priority materially changes.
- Keep frozen backup permanently unchanged.
- Host regression, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
