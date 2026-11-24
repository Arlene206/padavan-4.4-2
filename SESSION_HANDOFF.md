# Session Handoff

## Current repository state

- Parent main for this corrective commit: 49c37472f8e6c1c630382869ef0bbb4b12d492a7. Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains read-only at 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Clock-lifecycle implementation commit 49c37472f8e6c1c630382869ef0bbb4b12d492a7: AI docs #45326822074 PASS. Switch CLI #45326822109 failed in its existing inline probe/clock check because the GE2 temporary ioremap had been moved after PM-runtime start, and that test explicitly requires the mapping/ENOMEM failure before PM acquisition. This correction restores that failure boundary. NVRAM #45326822151 remains running; XY-C1 #45326822049 remains running at the IAPP firewall lifecycle validation stage. Build/TRX/artifact stages are not confirmed.
- Main source commit 3ea3089230ea2696e518c525ec6cf1132aa1ea83 before the clock changes had AI docs #45326081842, Switch CLI #45326081376 and NVRAM #45326081336 PASS; XY-C1 #45326081305 was still building at last poll. Previous firmware completion on f7bd670 predates later source changes.
- Frozen backup remains untouched. No physical-router or latest-source firmware PASS is claimed.

## Active task

Inspect the exact Actions for this correction commit. Switch CLI should now pass because the GE2 sysctl mapping is mapped, checked, programmed and unmapped before PM-runtime/clocks are acquired; an ioremap failure returns -ENOMEM with no PM or clock state to unwind. If it passes, continue the repository stability ledger outside RAETH while NVRAM/XY-C1 run. Diagnose any exact-head source or build failure and fix it atomically.

## Completed development slices

- RAETH DMA/RX/TX and allocation lifecycle: PDMA/QDMA RX partial allocation rollback and DMA-unmap accounting; RSS/HW-LRO ring rollback; QDMA TX/FQ/SFQ staged-allocation cleanup; staged DMA init unwind.
- RAETH IRQ and netdev lifecycle: partial IRQ request tracking/unwind; GSW source disable before handler release; open/close worker and module-reference ordering; pseudo-netdev safety; NAPI removal; primary netdev unregister/free on platform remove.
- MT7622/MTK switch lifecycle: checked platform-device lookup/put, regulator voltage/enable error propagation and per-rail state; correct IRQ mapping, pctl node/regmap handling, pinctrl ERR_PTR check and put.
- Primary RAETH probe/remove ownership: switch_np ref release on failed registration and ndo-uninit; debug-proc cleanup only after successful init; descriptor release before QDMA helper platform_device_put; drvdata-backed primary netdev remove and MMIO unmap.
- RAETH clock lifecycle: successful-enable bitmap, required error-pointer propagation, PM-runtime get/put balancing, partial failure unwind, shared SGMII PLL de-duplication, SGMII_TOP and GE2 SGMII1 disable pairing. The GE2 temporary register mapping is restored to the pre-PM failure boundary by this correction.
- Shared PID-file safety: no-follow open plus regular-file, effective-UID, single-link and no group/world write policy; strict GNU89-compatible predicate and complete metadata regression binding.
- Earlier HTTPD, WAN/DHCP, NVRAM, storage and add-on stability slices remain in the Roadmap/dev-log archives; do not repeat completed audits without new evidence.

## Verification boundary

- Commit 49c37472f8e6c1c630382869ef0bbb4b12d492a7: AI handoff #45326822074 PASS; Switch CLI #45326822109 FAIL at the existing inline ioremap-before-PM contract. NVRAM #45326822151 and XY-C1 #45326822049 were in progress at last poll. The corrective source change in this commit is not yet verified.
- Do not infer a firmware build/TRX/artifact result for current source from the older f7bd670 SHA.
- Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main; poll the exact-SHA AI docs, Switch CLI, NVRAM and XY-C1 runs for this correction. Inspect XY-C1 Build, TRX and artifact upload individually.
2. If Switch CLI passes, continue an independent stability/security bug outside RAETH while other jobs are pending. If it fails, inspect the exact failing step/log before changing source.
3. Maintain the main-only atomic tree/commit/CAS workflow and refresh handoff only for material changes.

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
