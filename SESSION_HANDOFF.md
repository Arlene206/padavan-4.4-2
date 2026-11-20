# 2026-11-20 continuation checkpoint

## Cross-session execution mandate

- Continue development on main only. Every logical task MUST be one atomic main commit: create all blobs and one complete Git tree, then one commit and a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Fix actual source defects with focused regressions; never weaken checks.
- PENDING Actions do not block independent work. Diagnose the earliest causal log for exact-SHA failures.
- Finish each safe slice and leave a factual first action for the next GPT.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Inspect live main, verify frozen backup SHA and check exact-SHA Actions at session end. Update this file only when task, source baseline, evidence or first next action changes materially. Do not make no-op commits.

## Current repository state

- Repository Arlene206/padavan-4.4-2; parent for this errno-contract tightening: c492d33b10baf96d8a971c4adc6773b3df5808f3.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only.
- UDP peer-cleanup tuple correction in source 4a7d3c3e3b1966d1574f703ffbfc498bce292542 uses the internal source-port tuple from get_udp_match() when locating the DSCP peer rule.
- Shared MiniUPnP cleanup policy now records the first errno within primary and secondary phases, then returns the primary error ahead of peer cleanup errors.
- scripts/test_miniupnpd_iptc_errno.py compiles actual delete_rule_and_commit() and insert_rule_and_commit() source definitions under both legacy and IPTABLES_143 signatures, using syslog/iptc_strerror/iptc_free mocks that clobber errno. The generated C preamble includes syslog.h for LOG_ERR.
- This commit strengthens both the standalone and workflow-inline source contracts to reject any direct assignment to primary_failure_errno or secondary_failure_errno that bypasses the first-error policy.
- Product L3 must be verified on a single exact source SHA using Build XY-C1, TRX verification and artifact upload. Historical 30af417/eccdab results do not apply to later product code. Physical-router L4 remains NOT VERIFIED.

## Active task

1. Run exact-head MiniUPnP/AI/Process/NVRAM checks. Verify the injected IPTC helper harness passes for both handle APIs, and the first-error checker rejects any direct overwrite.
2. Inspect exact-head XY-C1; claim L3 only when Build XY-C1, Verify TRX output and artifact upload all pass on the same product-source SHA.
3. If green, extend injected tests for transaction-level primary/secondary rollback sequencing or move to another concrete reliability/security defect outside MiniUPnP.
4. Continue source → regression → atomic commit → exact-SHA Actions.

## Verification boundary

- The current errno harness and the first-error source contract require exact-head MiniUPnP verification; do not assume success from the parent revision.
- This contract-only commit does not change firmware C. A product-source XY run can certify the unchanged product SHA if its exact run completes all three gates; it cannot certify a newer source change.
- Firmware L3 for current product code and physical-router L4 remain NOT VERIFIED until those gates pass.

## Exact first action for the next GPT

Fetch live main and poll exact-head MiniUPnP, AI, Process, NVRAM and XY-C1 Actions. Inspect the logs for both compiled legacy/IPTABLES_143 errno harness variants. Confirm the source-contract step enforces helper usage for every primary/secondary errno assignment. Verify Build XY-C1, TRX and artifact upload individually on the current source SHA, then continue transaction-level failure/rollback regression or another demonstrated userland fix.

## Handoff maintenance

- Keep dev_log.md newest-first; archive only at the 120-record threshold, retaining newest 20.
- Update REFACTOR_ROADMAP.md only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
