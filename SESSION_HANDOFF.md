## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. Startup protocol and handoff recovery were completed once in this session; do not repeat the initial audit.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify it. Only `backup/...` and `main` exist.
- Current main before this assignment-limit fix: `2bbb25778ea99a2734e7916b4acc989ad38dc74a` (handoff-only descendant of `0827abadc76f456ba9d67003cbedbdf7854d9e89`). The 0827 exact-head NVRAM run had all policy binaries and the inventory scans in progress at its last poll; exact completion is not yet known. Source checker regex was corrected in 0827.
- PR #1 remains closed and unmerged; the CA trust-store feature is already integrated into main.

## Active task

- Fix NVRAM CLI set/settmp's undersized `char buf[1024]`: the parser and ioctl ABI allow a 63-byte key and 4095-byte value, requiring 4160 bytes including `=` and NUL. Both commands now share one `NVRAM_MAX_PARAM_LEN + NVRAM_MAX_VALUE_LEN` workspace and volatile-clear it after both successful and rejected assignments.
- Added a GNU89 policy test for the exact 63-byte name / 4095-byte value boundary, plus a source guard requiring exactly one shared 4160-byte workspace, two parser callsites and two wipe callsites. The existing NVRAM workflow already compiles/runs this test.

## Exact-SHA CI checkpoint

- Source SHA polled: `0827abadc76f456ba9d67003cbedbdf7854d9e89`.
- Poll time (UTC): `2026-11-28T12:58:21.384Z`.
- #46147228376 NVRAM transaction boundary audit: in_progress; nvram-transaction-boundary=in_progress
- #46147228338 HTTPD Authentication and Security Regression: in_progress; HTTPD authentication and disclosure policy=in_progress
- #46147228339 Process lifecycle inventory - 0827abadc76f456ba9d67003cbedbdf7854d9e89: in_progress; inventory=in_progress
- #46147228361 AI handoff documentation contract: in_progress; validate-handoff-records=in_progress

## Verification boundary

- Main `0827abadc76f456ba9d67003cbedbdf7854d9e89`: AI handoff #46147228361 PASS, inventory #46147228339 PASS, HTTPD #46147228338 PASS. NVRAM transaction-boundary #46147228376 was still in progress at last poll; all preceding CLI and NVRAM policy binaries passed, with the final raw-userland/kernel API audit still running. Its final result is UNKNOWN until the job completes.
- Parent checker-fix commit `0827` repaired the cd6 checker-literal/regex false negatives; the follow-on exact NVRAM run will establish whether the full source contract passes.
- New assignment-capacity source/test/contract change: THIS COMMIT; no local build is available. Exact-head NVRAM host regression/source checker and same-SHA XY-C1 Build/TRX/artifact remain pending.
- Earlier XY-C1 #46145600745 on production SHA `670e513d99fa0e2d48dfc382ea795ab5c8f6ba1a` passed Build, TRX and artifact upload, but predates both NVRAM CLI hardening changes and cannot validate this source. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head NVRAM transaction-boundary run for THIS COMMIT. Require `NVRAM CLI maximum assignment boundary: PASS`, secure-buffer erasure PASS, and the semantic source checker to confirm one ABI-sized shared workspace, both parser callsites and both post-attempt wipes.
2. Inspect same-SHA XY-C1 Build, TRX validation and artifact upload. The CLI binary is a production input; record L3 only when all three pass on that exact SHA.
3. If NVRAM validates, continue a source-backed stability/security fix outside this subsystem; do not repeat the broad NVRAM inventory already recorded.
4. Preserve frozen backup and main-only complete-tree commits with expected-SHA updates. Hardware L4 remains NOT VERIFIED.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. Update this file when the active task or first action materially changes; do not create a documentation-only commit merely to record routine CI transitions.

## Cross-session execution mandate

- One logical task, one atomic main commit: create all blobs, the complete tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, finish safe source slices, inspect exact-head Actions, and leave the next GPT an accurate first action and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work.
- Recover once, implement continuously, do not repeat completed audits, and do not ask what to do next.
