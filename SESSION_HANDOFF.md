# 2026-11-20 continuation checkpoint

## Cross-session execution mandate

- Continue development on `main` only. Each logical task is one atomic commit: create all blobs and one complete Git tree, then one commit and a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Fix real source defects with focused regressions; do not weaken checks.
- PENDING Actions do not block independent work. Diagnose the earliest causal failure log.
- Before context exhaustion, finish the current safe slice, inspect the full diff, publish atomically, and preserve the exact next action here.

## Mandatory handoff checkpoint

The checkpoint is mandatory; document edits are conditional. Inspect live `main`, verify the frozen backup SHA and check exact-SHA Actions at session end. Update this file only when the baseline, evidence, blocker or first next action materially changes. Do not make no-op commits.

## Current repository state

- Parent `main` SHA for this slice: `b2971b23154c1dac6794cf3ff1295ae0632e7cf4`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only.
- MiniUPnP exact-head run `#44874295877` PASSED on `b2971b23154c1dac6794cf3ff1295ae0632e7cf4`; its first-error errno delete contract, GNU89/Werror/ASan/UBSan policy suite and both IPTC API variants passed.
- XY-C1 validation run `#44874295896` PASSED host validation on `b2971b23154c1dac6794cf3ff1295ae0632e7cf4`, but Build XY-C1, TRX verification and artifact upload were SKIPPED. Do not claim L3 for this run. The last complete Build/TRX/artifact L3 evidence remains run `#44809783977` on product-source SHA `0b5eef8bd00fd6e732449bcf241949df84b4f40c`.
- Physical-router L4 remains NOT VERIFIED.
- New defect found in `net_wan.c`: physical WAN `udhcpc_bound()` and fast `udhcpc_renew()` wrote `wanN_dltime` at callback entry using a void setter, before validation or successful DHCP state publication. A rejected lease or failed transition could alter the timestamp while retaining the old lease. This commit moves BOUND timestamp into its compensatable 12-field lease tuple and fast RENEW timestamp after successful option reconciliation, with checked transactional publication.

## Active task

1. Validate this dltime change on its exact pushed SHA: GNU89/Werror/ASan/UBSan DHCP lease regression, source-order checker, WAN DHCP callback Action and AI documentation contract.
2. Inspect exact-head XY-C1 steps. Record L3 only if Build, TRX verification and artifact upload all PASS on the same product-source SHA.
3. After validation, continue the broader user-space reliability queue outside the recently completed MiniUPnP delete-compensation work. Prefer a concrete P1/P2 defect with a focused regression; avoid repeated audits.

## Verification boundary

- The DHCP lease tuple regression and source checker are added in this commit; exact-head Actions are pending. No local checkout/compiler execution was available.
- MiniUPnP passed on parent `b2971b23154c1dac6794cf3ff1295ae0632e7cf4` (run `#44874295877`).
- XY-C1 passed host validation only on `b2971b23154c1dac6794cf3ff1295ae0632e7cf4`; Build/TRX/artifact were skipped. Earlier full L3 evidence is limited to `0b5eef8bd00fd6e732449bcf241949df84b4f40c` / run `#44809783977`.
- Physical-router L4 is NOT VERIFIED.

## Exact first action for the next GPT

Fetch latest `main` and inspect exact-head WAN DHCP callback, AI docs and XY-C1 Actions created by this commit. Confirm the dltime regression/checker passes and inspect Build XY-C1, TRX and artifact conclusions. Repair the earliest causal failure if any; otherwise continue with the next ranked user-space reliability issue outside MiniUPnP.

## Handoff maintenance

- Keep `dev_log.md` newest-first; archive only at 120 root records, retaining the newest 20.
- Update `REFACTOR_ROADMAP.md` only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
