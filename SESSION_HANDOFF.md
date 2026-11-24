# Session Handoff

## Current repository state

- Parent main for this atomic source/checker/handoff commit: `8f141576497dd6c04938cf89cd95bcc5b29de333`. Latest production-source SHA before this commit: `f7bd670c88520298c8c4628b1ff06e0c95251f12`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Process lifecycle run #45323579958 **PASS** on `24d204e991e1ea1b2f1d1af6bb1c96d327e37ca6`. Exact AI handoff #45324509191 **PASS** on `81b97858247bdb0fb479d88cde8e1ad317b1e212`.
- The `8f141576497dd6c04938cf89cd95bcc5b29de333` documentation follow-up #45324629954 failed only because its appended duplicate Roadmap snapshot made the file 132155 UTF-8 bytes. This commit removes the duplicated appended snapshots and restores the Roadmap below the 131072-byte validator ceiling.
- Exact production-source SHA `f7bd670c88520298c8c4628b1ff06e0c95251f12`: Switch CLI #45323341869 **PASS**, NVRAM #45323341860 **PASS**, and XY-C1 #45323341920 **PASS**. XY-C1 completed Build, TRX verification and artifact upload. No physical-router L4 result is claimed.
- This commit fixes MT7622 GSW preinit: checked platform-device lookup and drvdata, balanced the acquired device reference, made voltage/enable failures fatal, recorded only successful regulator enables, rolled back partial enable on GPIO/MCM setup errors, and made shutdown disable only rails actually enabled. Current exact-SHA Actions have not yet run.

## Active task

Audit the remaining RAETH probe failure labels for complete reverse-order cleanup of clock, MMIO and partially-created device resources. Review each acquisition in `rather_probe()` against every `goto err_*` path, including clock enable, frame-engine/GMAC mappings, system-controller mapping, descriptor setup and optional QDMA helper-device creation. Add an executable source contract for the cleanup order and fix any concrete leaks before moving to a different subsystem.

## Completed development slices

- RAETH DMA/RX and allocation lifecycle: PDMA/QDMA RX partial allocation rollback, DMA map accounting/unmapping; RSS 4-ring/2-ring and HW-LRO ring rollback; QDMA TX/FQ/SFQ staged-allocation cleanup; staged DMA initialization unwind.
- RAETH IRQ and netdev lifecycle: tracked partial IRQ requests and release; disabled GSW interrupt source before handler release; ordered worker shutdown, reset, DMA teardown and IRQ release; safe pseudo-netdev registration/uninit and NAPI removal.
- RAETH switch setup: MT7621 GP0 clock-enable error propagation; MT7622 open-failure cleanup; this commit now checks switch platform-device references, regulator voltage/enable results and records main/B3V enable state before disabling.
- Shared PID-file safety: no-follow open plus regular-file, effective-UID, single-link and no group/world write policy; strict GNU89-compatible predicate; complete metadata regression binding.

## Verification boundary

- MT7622 preinit source commit `615d6be0f8db31905bda066be38804490b02e45f`: AI docs #45325334351 **PASS**. Its initial Switch CLI #45325334356 failed solely because the new source checker incorrectly counted the NULL-pdev return as a post-acquisition return; checker-only commit `65103b60d8267e769efbc7aff5395f2662a7d83e` fixed the boundary and Switch CLI #45325386250 **PASS**. NVRAM #45325334304 **PASS**; XY-C1 #45325334328 was still running at last poll.
- Previous production-source SHA `f7bd670c88520298c8c4628b1ff06e0c95251f12`: Switch CLI/NVRAM **PASS**, and XY-C1 completed Build, TRX verification and artifact upload. This is not evidence for newer source changes.
- The current probe/netdev lifecycle source changes in this commit are not yet verified. Poll the exact new SHA for Switch CLI, NVRAM and XY-C1 Build→TRX→artifact. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main`; poll the exact-head Switch CLI, AI handoff, NVRAM and XY-C1 Actions, checking the XY-C1 Build, TRX and upload steps separately.
2. If the new probe/netdev lifecycle checker or build fails, diagnose the exact failed step and fix it before moving on.
3. Audit remaining `rather_probe()` error labels for reverse-order clock/MMIO/helper-device cleanup; continue with a different userland module after the RAETH probe lifetime is closed.


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
