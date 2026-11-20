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
- Parent main at this checkpoint: `64d76a2c98a4fb0ed98f8aba899a71c44750f407`. This commit corrects the MiniUPnP status-aware delete transaction and its stale source contracts.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is archival and read-only.
- The latest source change before this patch was `8f88445490dd8df6b600743ec0ff7716707df115`, titled `fix(miniupnpd): persist primary redirect deletion state`. It introduced a separate `primary_removed` output so lease persistence follows primary DNAT/filter state even when secondary peer SNAT/DSCP cleanup fails.
- Exact-head MiniUPnP run #44802582606 on 8f884 failed at IPTC delete-handle ownership because the checker had not yet been retargeted from the compatibility wrapper; commit `64d76a2c98a4fb0ed98f8aba899a71c44750f407` corrected that checker. Its exact-head MiniUPnP rerun #44802827202 passed the ownership check and then failed at the next stale peer checker, which is corrected here.
- Source review found a scan-state placement defect in the 8f884 implementation: the completion marker was outside its owning delete transaction and the local flag was never set. This patch moves it into the verified NAT-chain enumeration branch, makes missing-chain status fail, adds a regression contract and repairs the incremental Makefile dependency.

## Active task

1. Verify this commit's exact-head MiniUPnP workflow from the start through peer SNAT/DSCP, primary-delete/lease-state contract, lease persistence, and daemon start-gate contract. If any checker fails, diagnose its first causal assertion.
2. Verify exact-head XY-C1 Build XY-C1, TRX validation and artifact upload on THIS source SHA. Parent source Action #44802582897 was still in progress at last poll and cannot validate this corrected product source.
3. If the source and CI pass, review delete failure-state cases beyond the pure policy function: missing NAT chain, no NAT match, NAT delete failure, filter delete failure with successful/failed rollback, and independent peer/mangle cleanup. Add injected integration tests before any further behavior changes.
4. Continue whole-userland stability/security work. The closed/unmerged AliDDNS PR #1 remains only a source for read-only comparison; do not develop on or reopen its branch.

## Verification boundary

- Parent main `64d76a2c98a4fb0ed98f8aba899a71c44750f407`: MiniUPnP run #44802827202 failed at `scripts/check_miniupnpd_iptcrdr_peer_delete.py` after the IPTC handle-ownership step passed; the checker targeted the old compatibility wrapper.
- Source parent `8f88445490dd8df6b600743ec0ff7716707df115`: MiniUPnP run #44802582606 failed at the now-corrected handle-ownership wrapper mismatch; XY-C1 #44802582897 was still in progress at last observation. No target L3 result was available for the source.
- The new exact-head MiniUPnP and XY-C1 runs are pending for THIS COMMIT. Until the same-SHA build, TRX and artifact steps pass, firmware L3 is NOT VERIFIED. Physical-router L4 is NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and inspect exact-head MiniUPnP and XY-C1 push runs for THIS COMMIT. Confirm that the scan-state source contract, IPTC handle-ownership contract, peer SNAT/DSCP contract, primary-delete/lease-persistence contract, and every subsequent C regression pass; diagnose the first failing step if not. Count L3 only when Build XY-C1, TRX verification and artifact upload actually ran and passed on the identical SHA. Then continue the remaining independent stability/security work without waiting for PENDING Actions.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
