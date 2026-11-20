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

- Repository Arlene206/padavan-4.4-2; main at recovery: 9d3d8760f8111d0c0eda5244365470a110325336. Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807 and immutable.
- Exact f1a9213: infosvr #44799860384 PASS; AI handoff #44799860395 PASS; process inventory #44799860492 PASS. XY #44799860383 passed host-only validation and skipped target build for its tests/checker/docs-only delta.
- MiniUPnP lease atomic update work entered ec42fa99a15d617e181fe366324b8a18a7ff37ba. Host stream policy tests passed there, but run #44800659014 failed at stale checker assertions. Review found and removed a duplicate static declaration in 9d3d8760f8111d0c0eda5244365470a110325336; the checker in that commit still used obsolete remove-helper checks and an out-of-order variable reference.
- THIS COMMIT rewrites the MiniUPnP lease checker to cover current code: append rollback, shared exact-key stream rewriting, temp-file fsync+rename, update rollback, and the existing delete status behavior.
- Current production-source XY run for the prior head #44800659212 was still running at last observation. This new head triggers a fresh XY cross-build and MiniUPnP regression; verify exact IDs from live Actions.
- Historical production-source XY runs #44799072655 (7065465) and #44799778171 (c4e6) also need terminal Build/TRX/artifact status. Hardware L4 NOT VERIFIED.

## Active task

1. Verify this commit's exact-head MiniUPnP regression. Do not call the slice complete until the new checker and host tests both pass.
2. Verify target Build XY-C1, TRX verification and artifact upload on the latest source SHA.
3. Complete delete transaction semantics: Linux delete_redirect_and_filter_rules() can fail in secondary peer/DSCP cleanup after primary NAT/filter deletion commits, so _upnp_delete_redir() can retain a stale lease record. Add explicit state and injected tests before changing return/persistence behavior.
4. Continue whole-userland stability/security work while CI is pending; never touch the frozen backup.

## Verification boundary

- ec42fa99a15d617e181fe366324b8a18a7ff37ba: atomic rewrite host policy test passed; MiniUPnP #44800659014 failed on outdated source checker before production compile.
- 9d3d8760f8111d0c0eda5244365470a110325336: duplicate storage-class declaration corrected, but checker still had stale assertions. THIS COMMIT repairs the checker; no passing exact-head source contract yet.
- Latest XY exact-source Build/TRX/artifact status is pending new run. Non-firmware MIPS/uClibc runtime and physical-router L4 remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and inspect MiniUPnP regression and XY-C1 exact-head runs for this checker/source SHA. Diagnose any failure at its earliest causal step.
2. Check whether old production XY runs #44799072655 and #44799778171 finished Build, TRX verification and artifact upload.
3. Implement a tested delete-status contract for primary NAT/filter versus optional peer-SNAT/DSCP cleanup so main mapping and lease persistence cannot diverge.
4. Continue other stability/security work without waiting for pending Actions.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
