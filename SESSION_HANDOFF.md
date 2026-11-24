# Session Handoff

## Current repository state

- Main parent for this handoff/rolling-log commit: `7cd1ffd3eecc05786846be684053428f20e8f535`. Latest RAETH production-source baseline: `f7bd670c88520298c8c4628b1ff06e0c95251f12`. The later `24d204e` commit corrected the PID-file metadata integration fixture; `7cd1ffd` only compacted the Roadmap row and updated handoff/log state. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Process lifecycle run #45323579958 completed **SUCCESS** on exact test/source SHA `24d204e991e1ea1b2f1d1af6bb1c96d327e37ca6`; all GNU89, ASan/UBSan, parser, metadata and integration steps passed. The checker now loads metadata-policy cases from `trunk/user/shared/tests/shutils_pidfile_open.c`, while parser-only cases remain in `src/tests/shutils_pidfile_policy.c`.
- AI handoff run #45323806827 on `7cd1ffd3eecc05786846be684053428f20e8f535` passed the Roadmap size gate (the preceding commit compacted the LAN DHCP row) and failed because root `dev_log.md` had 122 records. This commit archives 102 oldest records to `docs/dev-log/2026/11/part-031.md`, leaving 20 prior newest records plus this new rotation entry; the archive index stores the exact blob SHA and byte count.
- RAETH exact-source Actions on `f7bd670c88520298c8c4628b1ff06e0c95251f12`: Switch CLI #45323341869 **PASS**; NVRAM transaction audit #45323341860 **PASS**; XY-C1 firmware validation #45323341920 remains **IN_PROGRESS** (last observed job: cross-build). TRX verification and artifact upload are not yet confirmed.
- No local MIPS firmware build, current-baseline Build/TRX/artifact completion, or physical-router L4 result is claimed. All Actions must be interpreted against their displayed exact SHA.

## Active task

Continue the focused RAETH resource-lifecycle audit in `trunk/linux-4.4.x/drivers/net/ethernet/raeth/ra_switch.c`, specifically `fe_sw_init()` and `fe_sw_deinit()`. Trace platform-device lookup/reference ownership, regulator get/enable/disable/put pairs, each clock prepare/enable/disable/unprepare path, MMIO mapping ownership, and every failure edge. Compare with the existing probe/open/close call paths and device-tree consumers. The existing `scripts/check_raeth_rx_ring_cleanup.py` checks part of `fe_sw_deinit()` and the MT7621 GP0 enable failure but does not yet fully cover all init-side resource pairs/unwinds. Do not assume a defect until the complete path is traced.

## Completed development slices

- RAETH DMA/RX and allocation lifecycle: PDMA/QDMA RX partial allocation rollback, DMA map accounting and unmapping; RSS 4-ring/2-ring and HW-LRO ring rollback; QDMA TX/FQ/SFQ staged-allocation cleanup; multi-stage DMA initialization unwind.
- RAETH IRQ and netdev lifecycle: request bitmap and partial IRQ unwind; GSW interrupt source disabled before IRQ release; checked IRQ setup failure; open/close worker ordering; pseudo-netdev registration/uninit safety; NAPI removal during uninit; MT7622 switch teardown on open failures; safe handling of an unavailable pseudo-interface during close.
- RAETH clock/work item corrections: propagate MT7621 GP0 clock-enable failure; remove cancellation of the never-initialized `reset_task` work item. Existing source contracts are in `scripts/check_raeth_rx_ring_cleanup.py` and the Switch CLI workflow.
- Shared PID-file policy: require regular file, effective-UID ownership, single hard link and no group/world write; keep the predicate strict-GNU89 compatible using `S_IFMT/S_IFREG`; make the policy header self-contained and ensure XY-C1 path filters include it. The corrected metadata fixture binding passed the full Process lifecycle run above.
- Earlier HTTPD, WAN/DHCP, NVRAM, storage and add-on lifecycle slices remain documented in the module dashboard and chronological `dev_log.md` archives; do not repeat closed audits without new evidence.

## Verification boundary

- Exact Process lifecycle run #45323579958: **PASS** on `24d204e` (all GNU89/ASan/UBSan and integration steps passed).
- Exact AI handoff run #45323806827: **FAIL** only because root log count was 122; this commit performs the required lossless rolling rotation and refreshes the required handoff sections. A new exact-head AI docs run must confirm.
- Exact RAETH source SHA `f7bd670`: Switch CLI #45323341869 **PASS**, NVRAM #45323341860 **PASS**, XY-C1 #45323341920 **IN_PROGRESS**. No Build→TRX→artifact PASS is claimed until every stage is complete on the matching source SHA.
- Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect the exact-head AI handoff result for this log rotation; confirm `part-031.md` is indexed and archive/root entry counts match.
2. Re-poll XY-C1 #45323341920 on its exact product SHA; inspect Build, TRX verification and artifact-upload job/step status, without waiting when pending.
3. Extract and trace the full `fe_sw_init()` / `fe_sw_deinit()` implementation, including platform-device references, regulator state, clock state and all `ioremap`/devm mapping failure edges; inspect callers and relevant DT resources.
4. When a concrete lifecycle defect is proved, add a focused regression/source contract, make one complete Git tree and atomic main commit via non-forced expected-SHA compare-and-swap, then inspect exact-SHA Actions. Continue independent work while jobs are pending.

# Cross-session execution mandate

Treat this as continuous work across GPT conversations. Recover once, then execute on `main`; each logical source or CI change is one atomic main commit. Create all changed blobs, one complete Git tree and one commit before updating the branch with a non-forced expected-SHA compare-and-swap. Never force-push or modify the frozen backup.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop

- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task. Never claim unverified work.
