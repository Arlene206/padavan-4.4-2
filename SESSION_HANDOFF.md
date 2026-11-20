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

- Repository Arlene206/padavan-4.4-2; default branch main.
- Parent main at this checkpoint: `b12d6c4abb2b95f901098425229eb736243335a2`. This commit fixes the remaining duplicate inline lease-writer contract; product source is unchanged from the already pushed `35e319a57af04e113fc9a3ccaa31628764ce4cfd`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; archival and read-only.
- Product-source commit `35e319a57af04e113fc9a3ccaa31628764ce4cfd` relocates `nat_scan_completed` into the verified NAT-chain scan in `delete_redirect_and_filter_rules_ex()`, reports a missing NAT chain as failure, adds the policy header to the `iptcrdr.o` prerequisites, and guards this placement with a source checker.
- The prior exact-source XY-C1 run #44802582897 for `8f88445490dd8df6b600743ec0ff7716707df115` FAILED at MIPS compilation: `nat_scan_completed` was undeclared in `get_redirect_rule_by_index()`. The source-side correction is in 35e; same-SHA XY must prove Build XY-C1, TRX and artifact.
- Exact-head MiniUPnP run #44803257122 on 35e completed FAILURE at the final embedded writer/source-contract step only. All preceding 25 steps passed. Its stale inline assertion required direct `fwrite()` / `fclose()` inside `lease_file_add()`, although that function now calls the tested transactional append helper. This workflow-only commit corrects the duplicate inline contract.
- AliDDNS PR #1 remains closed/unmerged and its branch remains read-only. The current main already contains the stdin-only EVP_MAC HMAC helper, its process-argv/environment regression, the CA archive/bootstrap checks and synchronous boot-status propagation; no cherry-pick of its stale branch is needed.

## Active task

1. Check exact-head MiniUPnP and AI handoff Actions for THIS COMMIT. Verify the writer source contract now accepts the transactional append implementation and all subsequent checks pass.
2. Check XY-C1 run #44803257077 on source commit 35e. Count L3 only if Build XY-C1, TRX verification and artifact upload all ran and passed on that exact source SHA. If any step failed, inspect the earliest causal build log and repair it.
3. Once CI clears, add injected delete-state tests beyond the pure policy function for missing NAT chain, no matching NAT rule, NAT delete failure, filter delete failure with successful/failed NAT rollback, and peer/mangle failure. Preserve the public legacy API wrapper and aggregate failure reporting.
4. Continue another concrete stability/security issue in Padavan userland; do not restart audits already recorded complete. PR #1 does not need further porting unless a new missing current-main behavior is demonstrated by exact file comparison.

## Verification boundary

- Parent `b12d6c4abb2b95f901098425229eb736243335a2`: new MiniUPnP and AI handoff workflow runs are queued/pending until live Actions confirms them.
- Product-source parent 35e: MiniUPnP #44803257122 failed only at the stale embedded direct-stdio assertion after all prior steps passed; AI handoff #44803257093 PASS; process inventory #44803257124 PASS; NVRAM audit #44803257169 was in progress at last poll; XY-C1 #44803257077 was in progress at last poll.
- Source parent 8f884: XY-C1 #44802582897 FAIL due the misplaced/undeclared NAT scan marker; corrected in 35e, but not yet verified at the same SHA.
- Firmware L3 is NOT VERIFIED until the target build, TRX validation and artifact upload have actually passed. Physical-router L4 is NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and inspect exact-head MiniUPnP and AI handoff runs for THIS COMMIT, plus the pending XY-C1 source run for 35e. Confirm that the writer transaction contract passes end-to-end; inspect the earliest causal log if not. Ensure the XY job's Build, TRX and artifact steps all passed on 35e before reporting L3. Then continue injected delete-state regression work or another independent userland defect without waiting for unrelated PENDING Actions.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
