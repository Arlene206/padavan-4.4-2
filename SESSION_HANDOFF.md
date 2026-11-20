# 2026-11-20 continuation checkpoint

# Cross-session execution mandate

- Continue main-only development. Every logical source slice is one atomic main commit: create all blobs, one complete Git tree and one commit, then update main using a non-forced compare-and-swap against the exact parent SHA.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup branch.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Implement actual fixes with focused regressions, then inspect Actions for the exact pushed SHA.
- A PENDING Action does not block independent work. Diagnose failures from causal job logs; do not weaken tests to obtain a pass.
- Before context exhaustion, finish a safe slice and leave a factual next task. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

At the end of every session, inspect live main, verify that the frozen backup is unchanged, and check the exact-head Actions relevant to the latest source commit. Update this file only when the active task, source SHA, verification evidence, or first next action has materially changed. If the checkpoint is already accurate, leave it untouched and do not create a no-op commit.

## Current repository state

- Current main before this checker-only correction: `2ffb8479a33ad55a4138b3384a31510f5b7c534e`. The preceding atomic source commit `2ffb8479a33ad55a4138b3384a31510f5b7c534e` added fail-safe allocation checks to MiniUPnP iptables rule construction.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only; unchanged).
- On exact SHA `2ffb847`, MiniUPnP Action #44789544201 compiled and passed all existing policy tests and the new GNU89 ASan/UBSan allocation-cleanup test; its source-contract step failed because the checker expected a multiline cleanup call as a contiguous string. No C sanitizer or compile failure occurred.
- AI handoff documentation #44789543996 and Process lifecycle #44789544079 passed on `2ffb847`; NVRAM transaction #44789543972 was still running at last poll.
- This source-checker correction normalizes function whitespace before asserting cleanup arguments; it does not weaken guard ordering or change production code.
- Physical-router L4 remains NOT VERIFIED.

## Active task

Make MiniUPnP legacy iptables rule construction fail safely under memory pressure:

- Guard all seven `calloc()`-backed match/target factories before their first dereference; restore allocator errno after diagnostic logging.
- Guard all five rule builders (DNAT, optional MASQUERADE, peer SNAT, peer DSCP and filter ACCEPT). An entry, match, target or final rule-buffer allocation failure must release all prior components, preserve errno and return before iptc append/commit.
- Add GNU89 ASan/UBSan regression coverage for partial-component cleanup and errno preservation, plus a Python source contract for every factory and builder.
- Extend MiniUPnP push/PR path filters and workflow steps so the production and new checks cannot silently skip verification.

After exact-head MiniUPnP and XY-C1 validation, continue reviewing remaining netfilter rule installation/deletion transactions. Do not repeat the completed leasefail/NAK/release or AP restore audit.

## Verification boundary

- Product source fix: `2ffb8479a33ad55a4138b3384a31510f5b7c534e`; entry/match/target/realloc failures release owned components and preserve errno.
- L2 sanitizer test on `2ffb847`: GNU89 `-Wall -Wextra -Werror` with ASan/UBSan and leak detection PASS.
- L2 source-contract step: FAILED only on multiline whitespace matching in the new checker; exact-head rerun required after whitespace normalization.
- AI handoff documentation #44789543996 and Process lifecycle #44789544079 PASS. XY-C1 L3 Build/TRX/artifact for the source SHA is not yet confirmed. Physical hardware L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and confirm frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
2. Check the exact-head MiniUPnP rerun after the whitespace-normalized source checker, then inspect XY-C1 Build/TRX/artifact and remaining exact-head Actions. Diagnose causal failures from logs.
3. If MiniUPnP source contract passes, continue the netfilter rule installation/deletion and partial-commit audit. Keep physical-router L4 unverified.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
