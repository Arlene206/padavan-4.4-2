# 2026-11-18 continuation checkpoint — include SCSI generic readiness policy

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS compile-contract correction: `bdeeee072c705524230d64986bdeffd5f41ff373`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; untouched.
- Exact parent `bdeeee07` USB modem workflow `44212660230` FAILed at inline source-contract line 95: `hotplug_usb.c` did not include `modem_sg_module_policy.h` even though the SG handler called its readiness helper. The new SCSI CD and SG module policy executables ran before this assertion; the failure is an actual missing production include, not a stale assertion. THIS commit adds the include; source contract remains.
- Exact parent `bdeeee07` AI handoff `44212660039`, Process inventory `44212660239`, AliyunDrive `44212660024`, HTTPD Security `44212660097`, infosvr `44212660165`, AliDDNS `44212660765`, AdGuardHome `44212660124`, and Process lifecycle inventory `44212660079` PASS. NVRAM audit `44212660076` remains IN_PROGRESS at last poll.
- SCSI CD eject/unload status handling is committed in `2fc75549`; SG readiness policy and caller logic are in `bdeeee07`. THIS commit closes the missing-header gap.
- No exact-current-SHA XY-C1 Build/TRX/artifact or physical-router L4 claim.

## Active task
- Complete the SG readiness integration by including the policy header required by `mdev_sg_main()`; rerun USB hotplug policy/source contracts.

## Verification boundary
- Exact parent SG/SCSI GNU89/Werror/ASan/UBSan test binaries completed before the inline source assertion; full USB workflow FAILed because of the missing include.
- Exact parent AI docs, Process inventory, AliyunDrive, HTTPD, infosvr, AliDDNS, AdGuardHome and process inventory workflows PASS.
- NVRAM audit still pending; THIS include fix's exact-head USB rerun pending. Firmware L3 and hardware L4 NOT VERIFIED.

## Exact first action for the next GPT
1. Inspect exact-head USB modem workflow; require both SCSI policy binaries, source-body assertions and WDM identity/path-filter checks to PASS.
2. Inspect NVRAM audit `44212660076` and any exact-head reruns.
3. Check whether XY-C1 Build/TRX/artifact is triggered for this source SHA; no L3 claim without all three passing.
4. Continue another independent P1 reliability/security slice while pending Actions run. Keep backup unchanged; do not repeat bootstrap recovery.

Every logical multi-file change is one atomic main commit built from one complete Git tree and updated by non-forced expected-SHA compare-and-swap.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional.
At session end inspect live main, relevant exact-SHA Actions, current handoff/roadmap, validation boundary and first actionable task. If all remain accurate, do not create a no-op documentation commit.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, implement concrete defects, test and commit atomically; do not wait idly on pending CI.

Every logical slice is one atomic main commit: create all blobs, one complete tree and one commit parented to freshly fetched main, then publish with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs, or modify frozen backup.

Before context exhaustion, finish a safe slice, inspect exact-SHA Actions and leave the next GPT a precise first action.
