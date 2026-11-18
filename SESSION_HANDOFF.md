# 2026-11-18 continuation checkpoint — surface USB SCSI CD eject/unload failures

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS USB SCSI hotplug fix: `2d91171b217e6f42a96fa5af7da3cdcacc499b3f`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; untouched.
- Exact parent `2d91171b` HTTPD Security `44211273335` PASS, AI handoff `44211273436` PASS, infosvr `44211273238` PASS. XY-C1 `44211273337` remains IN_PROGRESS at `Build XY-C1`; TRX and artifact upload not reached. NVRAM audit `44211273258` remains IN_PROGRESS at raw userland ioctl/kernel caller audit. AliDDNS, AdGuardHome, Process inventory and AliyunDrive were IN_PROGRESS at last poll.
- Exact parent `01ce19d4` WAN route `44210860686` PASS; its WISP return/PID1 aggregation source contract is covered. Latest follow-on HTTPD contract failure was repaired in `2d91171b`; exact HTTPD rerun on that SHA PASSed.
- Found a false-success path in `mdev_sr_main()`: SCSI CD eject `_eval()` and `module_smart_unload("sr_mod", 1)` results were discarded, then the handler printed success and returned zero. THIS slice captures/logs both statuses. It still attempts unload after eject failure. Because `module_smart_unload()` returns zero both on a failure and when its refcount file is unavailable (including an already-absent module), the new code uses `is_module_loaded_status()` to distinguish known absence (safe/idempotent completion) from a still-present or unknown module (fail closed). Positive unload status preserves the helper's documented successful-unload/in-use-no-op behavior.
- Added GNU89/Werror/ASan/UBSan policy coverage for successful unload, in-use no-op, already-absent no-op, present module after failed unload, unknown presence and eject errors. The USB modem workflow pins caller status capture, tri-state probing, diagnostics and operation ordering.
- No current-source MIPS firmware build or physical-router L4 claim.

## Active task
- Prevent SCSI CD event handling from claiming success after eject or safe-unload failure; treat a module proven absent as already complete, but fail closed when it remains present or presence cannot be determined.

## Verification boundary
- THIS source/policy/workflow slice is not yet verified on its exact new SHA; the USB modem WDM/NDIS workflow is the primary L2 gate.
- Exact parent `2d91171b` HTTPD Security and AI handoff PASS; XY-C1 `44211273337` and NVRAM `44211273258` remain pending as above.
- L3 requires same-SHA Build, TRX verification and artifact upload all PASS. Physical-router L4 NOT VERIFIED.

## Exact first action for the next GPT
1. Inspect exact-head USB modem workflow; require the new SCSI eject/unload GNU89 sanitizer test and source contract to PASS.
2. Inspect exact-head XY-C1 Build, TRX verification and artifact upload; no L3 claim until all three pass on one SHA.
3. Inspect NVRAM raw IOCTL/kernel caller audit and the remaining lifecycle/security Actions on the same source SHA.
4. Continue another independent P1 storage/service/security defect while pending runs execute. Keep the frozen backup untouched; do not repeat bootstrap recovery.

Every logical multi-file change is published as one atomic main commit built from one complete Git tree and updated with a non-forced expected-SHA compare-and-swap.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional.
At session end inspect live main, relevant exact-SHA Actions, current handoff/roadmap, validation boundary and first actionable task. If all remain accurate, do not create a no-op documentation commit.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, implement concrete defects, test and commit atomically; do not wait idly on pending CI.

Every logical slice is one atomic main commit: create all blobs, one complete tree and one commit parented to freshly fetched main, then publish with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs, or modify frozen backup.

Before context exhaustion, finish a safe slice, inspect exact-SHA Actions and leave the next GPT a precise first action.
