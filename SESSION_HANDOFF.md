# Session Handoff

## Current repository state

- Parent main for this atomic RAETH clock-lifecycle slice: 3ea3089230ea2696e518c525ec6cf1132aa1ea83. Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains read-only at 72f4f157a084f8184b9597ca5bf334b8080e5807.
- The preceding main commit 3ea3089230ea2696e518c525ec6cf1132aa1ea83 fixed switch-node, descriptor/QDMA helper, debug-proc and primary-netdev teardown ownership. Its exact Actions: AI docs #45326081842 PASS, Switch CLI #45326081376 PASS, NVRAM #45326081336 PASS. XY-C1 #45326081305 has completed all host-side checks and remains in Build XY-C1; TRX verification/artifact upload are pending. No latest-source firmware success is claimed.
- Earlier RAETH lifecycle fixes on 615d6be / d3ae666 / 3ea308 addressed MT7622 regulator state/reference handling, MTK GSW probe IRQ/pinctrl/DT cleanup, and primary netdev remove/uninit resource ownership. The previous standalone firmware success on f7bd670c88520298c8c4628b1ff06e0c95251f12 predates those later changes.
- This new source slice fixes clock setup/teardown: validate error pointers before using a clock, record only successful clk_prepare_enable operations, unwind prior clocks and PM-runtime references if a later operation fails, propagate TRGPLL rate-setting errors, avoid double-preparing a shared SGMII PLL, and release SGMII_TOP plus every GE2 SGMII1 clock. MT7621's intentionally absent clock handles remain no-op. Exact-head verification for this new slice is not yet available.

## Active task

Poll the exact new commit's AI handoff, Switch CLI, NVRAM and XY-C1 Actions. If any source checker or compile stage fails, inspect the failing step/log and correct it atomically. Once these checks advance, continue the repository stability ledger outside RAETH; prioritize clear severe faults/security issues and avoid reopening completed RAETH paths without new evidence.

## Completed development slices

- RAETH DMA/RX and allocation lifecycle: PDMA/QDMA RX partial allocation rollback, successful DMA-map accounting and teardown unmapping; RSS 4-ring/2-ring and HW-LRO ring rollback; QDMA TX/FQ/SFQ staged allocation cleanup; multi-stage DMA init unwind.
- RAETH IRQ and netdev lifecycle: partial IRQ tracking/unwind; GSW source disable before handler release; open/close worker and module-reference ordering; pseudo-netdev registration/uninit safety; NAPI removal; primary netdev unregister/free on platform remove.
- MT7622/MTK switch lifecycle: checked platform-device lookup and put, regulator voltage/enable error propagation and per-rail state, safe rollback; correct IRQ mapping ownership, pctl node of_node_put, regmap error checks, pinctrl_get ERR_PTR handling and pinctrl_put.
- Primary RAETH probe/remove ownership: switch_np ref release on failed registration and ndo-uninit; debug-proc teardown only after successful init; descriptor cleanup before QDMA helper platform_device_put; drvdata-backed primary netdev removal and MMIO unmap.
- RAETH clock lifecycle (current unverified slice): per-clock success bitmap; failure unwind; PM-runtime get/put pairing; SGMII_TOP and GE2 SGMII1 disable pairing; shared SGMII PLL deduplication.
- Shared PID-file safety: no-follow open plus regular-file, effective-UID, single-link and no group/world write policy; strict GNU89-compatible predicate and complete metadata regression binding.
- Earlier HTTPD, WAN/DHCP, NVRAM, storage and add-on stability slices remain in the Roadmap/dev-log archives; do not repeat completed audits without new evidence.

## Verification boundary

- Parent SHA 3ea3089230ea2696e518c525ec6cf1132aa1ea83: AI handoff, Switch CLI and NVRAM PASS; XY-C1 Build IN_PROGRESS, TRX/artifact still pending.
- Current clock source/checker slice has not yet had an exact-head Actions run. Do not infer test or firmware success from the parent SHA.
- Previous SHA f7bd670c88520298c8c4628b1ff06e0c95251f12 had successful Switch CLI/NVRAM and XY-C1 Build/TRX/artifact, but it predates the MT7622/MTK switch and primary netdev lifecycle changes.
- Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-SHA AI handoff, Switch CLI, NVRAM and XY-C1 runs; for XY-C1 inspect Build, TRX and artifact upload separately.
2. Fix any exact-head regression immediately; while jobs are pending, work an independent high-confidence stability/security issue elsewhere in Padavan userland.
3. Continue the repository-wide stability/security ledger rather than repeating completed RAETH DMA, IRQ, switch or netdev audits.

# Cross-session execution mandate

Treat this as continuous work across GPT conversations. Recover once, then execute on main; each logical source or CI change is one atomic main commit. Create all changed blobs, one complete Git tree and one commit before updating the branch with a non-forced expected-SHA compare-and-swap. Never force-push or modify the frozen backup.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop

- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task. Never claim unverified work.
