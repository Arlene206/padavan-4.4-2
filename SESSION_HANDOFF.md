# Cross-session execution mandate — 2026-11-21 checkpoint

## Cross-session execution mandate

- Continue development on `main` only. Each logical task is one atomic main commit: create all blobs and one complete Git tree, then one commit and a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Fix real source defects with focused regressions; do not weaken checks.
- PENDING Actions do not block independent work. Diagnose the earliest causal failure log.
- Before context exhaustion, finish the current safe slice, inspect the full diff, publish atomically, and preserve the exact next action here.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Inspect live `main`, verify the frozen backup SHA and check exact-SHA Actions at session end. Update this file only when the baseline, evidence, blocker or first next action materially changes. Do not make no-op commits.

## Current repository state

- Source baseline main SHA: ab866c3e93c5131ed6723c7a2a7d3de5df0e693c; this checkpoint is a documentation-only child of that source slice.
- Frozen archival branch backup/pre-plugin-overhaul-2026-11-14 remains at 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only.
- HTTPD startup/stop/restart regression run #44875048202 PASSED on ab866c3e93c5131ed6723c7a2a7d3de5df0e693c: GNU89/Werror/ASan/UBSan policy binary and source integration/path-filter checks passed.
- HTTPD Authentication and Security run #44875048156 PASSED on ab866c3e93c5131ed6723c7a2a7d3de5df0e693c; whole-repository Process lifecycle inventory run #44875048297 PASSED; WAN route operation status run #44875048033 PASSED.
- AI handoff contract run #44875048197 FAILED on ab866c3e93c5131ed6723c7a2a7d3de5df0e693c because the handoff used “one atomic commit” rather than the exact required phrase “one atomic main commit”. This checkpoint restores the literal contract and required top-level heading.
- XY-C1 run #44875048030 was still building at checkpoint; its Build, TRX verification and artifact-upload conclusions must be inspected before any L3 claim.
- NVRAM transaction boundary audit run #44875048211 and Process lifecycle regression run #44875048100 were in progress. Other exact-head workflows may remain pending.
- Physical-router L4 remains NOT VERIFIED.

## Active task

1. Inspect the exact-head AI handoff documentation run triggered by this checkpoint and confirm the literal heading and “one atomic main commit” rule pass.
2. Finish checking exact-head Actions from source SHA ab866c3e93c5131ed6723c7a2a7d3de5df0e693c, especially XY-C1 Build, TRX verification and artifact upload, NVRAM transaction audit and whole-repository Process lifecycle regression. Diagnose any FAIL at its earliest causal step; PENDING does not block independent work.
3. Continue the userland stability queue with a concrete source defect outside the just-completed HTTPD startup-wait rollback slice; add a focused GNU89-compatible regression and commit atomically.

## Verification boundary

- HTTPD startup wait failure now attempts listener rollback under the lifecycle lock, re-scans authoritative process state, republishes httpd_started=1 when the listener may still exist, and preserves the original wait error. The policy regression covers ABSENT, RUNNING, UNKNOWN and invalid scan outcomes.
- HTTPD daemon lifecycle PASS on ab866c3e93c5131ed6723c7a2a7d3de5df0e693c (run #44875048202); HTTPD authentication/security PASS (run #44875048156); Process lifecycle inventory PASS (run #44875048297); WAN route status PASS (run #44875048033).
- AI handoff contract failed on ab866c3e93c5131ed6723c7a2a7d3de5df0e693c solely at the missing exact wording “one atomic main commit”; this checkpoint corrects the required H1 heading and phrase. Its new exact-head run is pending.
- XY-C1 Build was in progress on ab866c3e93c5131ed6723c7a2a7d3de5df0e693c. L3 is NOT established until Build, TRX verification and artifact upload all PASS on the same product-source SHA. Physical-router L4 remains NOT VERIFIED.
- No local checkout or local cross-compiler was available; runtime/build claims rely only on exact Actions evidence.

## Exact first action for the next GPT

Fetch live main and confirm the frozen backup remains unchanged. Inspect the exact-SHA AI handoff run created by this checkpoint; then finish the XY-C1 Build, TRX and artifact steps plus the pending NVRAM and Process lifecycle Actions for source SHA ab866c3e93c5131ed6723c7a2a7d3de5df0e693c. Record L3 only on a single SHA with all three firmware steps passing. Fix the earliest causal failure if any; otherwise continue with the next independent userland P1/P2 source defect outside HTTPD.

## Handoff maintenance

- Keep `dev_log.md` newest-first; archive only at 120 root records, retaining the newest 20.
- Update `REFACTOR_ROADMAP.md` only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
