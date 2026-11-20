# 2026-11-20 continuation checkpoint

## Cross-session execution mandate

- Continue development on main only. Every logical task MUST be one atomic main commit: create all blobs and one complete Git tree, then one commit and a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Fix actual source defects with focused regressions; never weaken tests.
- PENDING Actions do not block independent work. Diagnose the earliest causal log for exact-SHA failures.
- Finish a safe slice and leave a factual first action for the next GPT.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Inspect live main, verify frozen backup SHA and check exact-SHA Actions at session end. Update this file only when task, source baseline, evidence or first next action changes materially. Do not make no-op commits.

## Current repository state

- Repository Arlene206/padavan-4.4-2; parent for this harness compile correction: 0b5eef8bd00fd6e732449bcf241949df84b4f40c.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only.
- Product source 4a7d3c3e3b1966d1574f703ffbfc498bce292542 fixed the UDP peer-cleanup tuple mismatch: the DSCP peer scan now compares the internal source port returned by get_udp_match() rather than the destination port. It has a dedicated policy regression and source checker.
- Shared cleanup policy now includes miniupnpd_record_first_failure_errno(), preventing a later NAT/filter or peer/mangle failure from overwriting the first errno in its phase. The top-level selector keeps primary errno ahead of secondary cleanup errno.
- scripts/test_miniupnpd_iptc_errno.py extracts actual delete_rule_and_commit() and insert_rule_and_commit() implementations and runs both legacy and IPTABLES_143 variants with errno-clobbering mocks. This commit adds syslog.h to its generated C preamble so LOG_ERR is defined.
- Exact-source firmware L3 for the current product revision is NOT VERIFIED until Build XY-C1, TRX validation and artifact upload all pass on one identical source SHA. Historical 30af417 and eccdab L3 evidence does not apply to later source.

## Active task

1. Inspect exact-head MiniUPnP, AI, Process and NVRAM Actions after this test-harness correction. Both harness API variants must compile and pass under GNU89/Werror/ASan/UBSan.
2. Check exact-head XY-C1. Count L3 only if Build XY-C1, Verify TRX output and artifact upload all complete SUCCESS on the same product-source SHA.
3. Once the injected helper test is green, extend tests to transaction-level first-error and NAT/filter rollback behavior, or move to another concrete userland stability/security issue.
4. Continue source → regression → atomic commit → exact-SHA Actions without waiting on unrelated PENDING jobs.

## Verification boundary

- This commit changes only the test runner and handoff/log. It does not change firmware source.
- The errno harness's exact result on its previous commit must not be assumed. Verify the new exact-head MiniUPnP step for both API variants.
- Product firmware L3 is NOT VERIFIED for the latest source until the actual Build/TRX/artifact gates pass. Physical-router L4 is NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and inspect exact-head MiniUPnP, AI, Process, NVRAM and XY-C1 Actions. Read the actual errno harness step output if either compiler variant fails; confirm both legacy and IPTABLES_143 pass after the syslog.h fix. Separately verify Build XY-C1, TRX validation and artifact upload on the same product-source SHA. Then continue transaction-level failure/rollback tests.

## Handoff maintenance

- Keep dev_log.md newest-first; archive only at the 120-record threshold, retaining the newest 20.
- Update REFACTOR_ROADMAP.md only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical-router L4 are separate verification levels.
