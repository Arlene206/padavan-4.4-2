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

- Repository Arlene206/padavan-4.4-2; main at recovery: ec42fa99a15d617e181fe366324b8a18a7ff37ba. Frozen branch backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807 and immutable.
- f1a9213a7b6eb98602dbdd55eb2bca9996ac11ed: infosvr #44799860384 PASS, AI handoff #44799860395 PASS, Process inventory #44799860492 PASS. XY #44799860383 reported success but only host-only validation; no cross-build on that test/checker/docs-only change.
- MiniUPnP declaration correction #7065465 and guard correction #567d542 are in main. MiniUPnP #44799593220 PASS on 567d.
- Commit ec42fa99a15d617e181fe366324b8a18a7ff37ba introduced an atomic rewrite for update-path lease rows and firewall rollback on file publication failure. Its exact-head MiniUPnP run #44800659014 passed the new GNU89 stream-policy test but failed on an outdated source-checker assumption that key filtering still lived in lease_file_remove(). Source review also found a duplicate static int specifier in the new rewrite helper, left by the text replacement. THIS COMMIT removes it and updates the checker.
- The exact-head XY-C1 run #44800659212 for ec42 was still in progress at last poll; that commit changes upnpredirect.c and therefore its build should include the new implementation. The new corrective source change needs its own run.
- Older production build runs #44799072655 (7065465 MiniUPnP declaration) and #44799778171 (c4e6 infosvr interface policy) were still in progress at last poll; poll exact run step completion. Hardware L4 NOT VERIFIED.

## Active task

1. Verify this commit's exact-head MiniUPnP regression and real XY-C1 build. The duplicate helper declaration must compile; do not accept only the host policy test.
2. Continue delete transaction analysis: Linux delete_redirect_and_filter_rules() can return secondary peer/DSCP failure after primary NAT/filter deletion committed, causing _upnp_delete_redir() to retain a stale lease row. Model and test the primary/secondary result-state semantics before any repair.
3. Check the older production-source XY runs #44799072655 and #44799778171 for Build, TRX verification and artifact upload conclusions.
4. Continue independent Padavan stability/security work while CI runs. Preserve main-only atomic CAS, keep the backup read-only.

## Verification boundary

- ec42fa99a15d617e181fe366324b8a18a7ff37ba: MiniUPnP #44800659014 failed after its new stream policy tests passed; failure was stale source-checker expectation. THIS COMMIT fixes the checker and a separate duplicate declaration caught during source review.
- XY-C1 #44800659212 on ec42 was IN_PROGRESS at last observation; it cannot validate a later fix. Require exact-head Build, TRX and artifact upload before L3.
- Older production-source XY jobs #44799072655 and #44799778171 were still in progress at last check. MIPS/uClibc runtime outside firmware and physical-router L4 remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch current main and inspect MiniUPnP regression + XY-C1 build on THIS exact source SHA. Diagnose any production compiler error or test contract failure, then fix atomically.
2. Check whether previous production-source XY runs #44799072655 and #44799778171 reached Build/TRX/artifact success.
3. Implement a tested delete-status contract that distinguishes primary NAT/filter deletion from optional peer/DSCP cleanup, so lease persistence cannot be left stale while the primary mapping is already gone.
4. Continue additional independent userland stability work; do not wait for queued/in-progress Actions.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
