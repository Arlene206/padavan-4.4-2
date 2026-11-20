# 2026-11-20 continuation checkpoint

## Cross-session execution mandate

- Continue development on main only. Every logical task MUST be one atomic main commit: create all blobs and one complete Git tree, then one commit and a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup branch.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Fix actual source defects with focused regressions; never weaken checks to obtain PASS.
- A PENDING Action does not block independent work. Diagnose exact-SHA failures from the earliest causal log.
- Before context exhaustion, finish a safe slice, commit it and leave an exact first action for the next GPT. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At session end inspect live main, verify frozen backup SHA, and inspect relevant exact-SHA Actions. Update this file only when task, source baseline, verification evidence or first next action materially changes. Do not create a no-op commit.

## Current repository state

- Repository Arlene206/padavan-4.4-2; parent for this normalized workflow-contract repair: 4e8bbf9be606d4a9280767146ccaae53bdbaea66.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only.
- Product source e3578d8b09eb5555aa726726184d21eafa25dbb8 preserves primary/secondary errno and 5ca967f2dd9da9ee20408dde1b413cfaeac9265c scopes the missing-chain ENOENT capture to delete_redirect_and_filter_rules_ex().
- The 5ca standalone checker failure on its NAT snapshot early-return assertion was fixed in 4e8bbf9be606d4a9280767146ccaae53bdbaea66 by requiring miniupnpd_restore_errno_on_failure(r, primary_failure_errno).
- Exact-head MiniUPnP #44808194708 on 4e failed only at the workflow's duplicate inline contract: the return expression is split across lines, but the inline check searched for a single raw line. The standalone checker uses normalized whitespace; this commit applies the same normalization to the inline assertion. The policy tests before that assertion passed.
- Prior exact-source eccdab product SHA has valid L3 (Build/TRX/artifact PASS); later source changes need new exact-SHA build evidence.

## Active task

1. Validate exact-head MiniUPnP, AI, Process, NVRAM and XY-C1 Actions for THIS COMMIT. The normalized inline assertion must pass and the compiled errno policy tests must pass.
2. Count L3 only after Build XY-C1, Verify TRX output and artifact upload all complete SUCCESS on the exact same product-source SHA.
3. If green, add injected lower-level backend tests for errno clobbering after IPTC handle release in both IPTABLES_143 and legacy branches, then cover primary/secondary error precedence for NAT/filter/peer rollback paths.
4. Continue source → regression → atomic commit → exact-SHA Actions.

## Verification boundary

- Parent 4e MiniUPnP #44808194708: compiled cleanup policy tests passed; final embedded writer/start-gate source contract failed solely on raw whitespace around the NAT snapshot early return. This workflow-only commit normalizes the assertion.
- Parent 5ca MiniUPnP #44807976684 failed earlier at a stale standalone version of the same snapshot early-return assertion; corrected in 4e.
- Parent 5ca AI, Process, NVRAM and XY results must be checked individually; none of those parent results certify the current source.
- L3 for the new source remains NOT VERIFIED until exact-head Build/TRX/artifact pass. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and inspect its exact-head MiniUPnP, AI, Process, NVRAM and XY-C1 Actions. Confirm the normalized NAT snapshot assertion passes after the compiled policy test. Inspect the first causal log for any failure. Only claim L3 when Build XY-C1, TRX verification and artifact upload all pass on the identical current source SHA.

## Handoff maintenance

- Keep dev_log.md newest-first; archive only at the 120-record threshold, retaining the newest 20.
- Update REFACTOR_ROADMAP.md only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical hardware L4 are distinct verification levels.
