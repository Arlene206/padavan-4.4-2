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

- Repository Arlene206/padavan-4.4-2; development branch main. Parent at this source slice: 3f28c8fd9cebb2cccf28b9926a1422d3bbfed427. Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807 and read-only.
- Exact-head f1a9213a7b6eb98602dbdd55eb2bca9996ac11ed: infosvr packet/signal lifecycle #44799860384 PASS; AI handoff #44799860395 PASS; Process inventory #44799860492 PASS. XY-C1 #44799860383 PASS but explicitly used host-only validation and skipped cross-build because f1 changed only tests/checker/docs relative to its parent.
- MiniUPnP declaration fix is in 7065465d000b0bc17763b51c61533cebba243f62. The checker correction in 567d5426fc26e6cbfb6664ec197566afa403755c is covered by MiniUPnP #44799593220 PASS.
- Actual production-source XY-C1 jobs #44799072655 (7065465, MiniUPnP declaration) and #44799778171 (c4e6b53, infosvr bounded interface-name fix) were still executing Build XY-C1 at last inspection. Their final Build/TRX/artifact results must be checked; neither result transfers to subsequent source SHAs.
- Source audit found a two-file lease transaction gap during existing UPnP mapping updates: the code removed the old lease in one rename and appended the new row separately after firewall changes. THIS COMMIT replaces all matching exact-key records and appends one new row with a single staged fsync+rename, restoring the prior firewall/metadata values if publication fails.
- _upnp_delete_redir() still deletes firewall rules before lease-file removal. If the lease-file rename fails, it leaves a stale lease row after firewall deletion. Further Linux delete-function review shows primary NAT/filter rollback on filter failure and peer SNAT rollback on DSCP failure. However a secondary peer/DSCP error can be merged into the final failure result after the primary NAT/filter deletion already committed; the wrapper then retains the lease row. This requires a transaction-state fix after modeling every failure stage.
- MIPS/uClibc runtime outside full firmware image and physical router L4 remain NOT VERIFIED.

## Active task

1. Verify this commit's exact-head MiniUPnP regression, including the new stream-level rewrite tests and source-order rollback contract. Diagnose any real compile or test error without weakening assertions.
2. Model _upnp_delete_redir() with Linux delete_redirect_and_filter_rules(): enumerate primary NAT/filter and secondary peer-SNAT/DSCP combinations, and confirm when main mapping deletion can return an error after NAT/filter already committed. Add injected regression before changing behavior.
3. Check the older production-source XY-C1 runs #44799072655 and #44799778171 for terminal Build, TRX verification and artifact upload. For this lease production commit, wait for its own exact-head XY build before L3.
4. Continue independent whole-userland stability/security tasks while CI is pending; never touch the frozen backup.

## Verification boundary

- Previous exact-head f1: infosvr #44799860384 PASS; AI docs #44799860395 PASS; Process inventory #44799860492 PASS. XY-C1 #44799860383 SUCCESS was host-only and skipped actual cross-build; no L3 claim.
- MiniUPnP lease checker #44799593220 PASS on 567d. The new atomic rewrite source/helper/test requires a new exact-head MiniUPnP run.
- XY-C1 production jobs #44799072655 on 7065465 and #44799778171 on c4e were at Build XY-C1/in progress at last poll. Inspect their steps for final TRX/artifact. They do not validate this new lease source commit.
- Host tests are designed for GNU89 stream transformation. The production translation unit has not been locally target-compiled here; MIPS/uClibc runtime and physical-router L4 remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch current main and inspect this commit's exact-head MiniUPnP Actions. Fix any compile/test issue on main and rerun.
2. Check #44799072655 and #44799778171 for final Build XY-C1, TRX verification and artifact upload conclusions.
3. Finish the deletion error-state model: inspect how peer SNAT / DSCP cleanup can fail after NAT/filter removal and whether the primary mapping must be restored before the lease row is retained.
4. Implement one safe transactional delete slice with injected GNU89 regression, source-order contract and exact-SHA Actions; continue unrelated tasks while firmware CI is pending.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
