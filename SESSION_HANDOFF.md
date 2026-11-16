# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task here.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Latest main parent before THIS COMMIT: `78f7302dad5142402a10b39c08c66a2850abe836`. Exact-head Actions at last poll: HTTPD Authentication and Security Regression 43917645930 completed/success; AdGuardHome config security regression 43917645405 completed/success; infosvr packet and signal lifecycle stability regression 43917645508 completed/success; AI handoff documentation contract 43917645337 completed/success; AliDDNS signed API transport security regression 43917645276 completed/success; XY-C1 Firmware Validation 43917645503 in_progress; AliyunDrive WebDAV lifecycle stability regression 43917645350 completed/success; Process inventory and lifecycle fail-closed regression 43917645410 completed/success; NVRAM transaction boundary audit 43917645866 completed/success
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- Storage module loading and mount ownership are fail-closed. A further review found the filesystem module catalog membership predicate matched any substring in the returned path, so a query for `fat` could match `exfat.ko`, and `vfat` could match `vfat-extra.ko`.
- NDIS bring-up status propagation landed in `b54a701c35e35549a183eca6bf48c86d5dffb855`; exact-head XY-C1 run `43917115679` was IN_PROGRESS at last poll.

## Active task

THIS COMMIT fixes false-positive filesystem-module catalog membership. `func_module_catalog_contains()` previously searched the complete multi-line path string for a substring; this could mistake `exfat.ko` for `fat`, or `vfat-extra.ko` for `vfat`. It now iterates catalog entries in POSIX shell, extracts the basename, and accepts only exact `.ko` / supported compressed-module suffixes. The regression adds exfat and similarly named vfat entries and requires both substring false-positives to remain absent. The storage workflow statically requires exact basename matching.

## Verification boundary

- Parent `78f7302dad5142402a10b39c08c66a2850abe836` exact-head Actions at commit time: HTTPD Authentication and Security Regression 43917645930 completed/success; AdGuardHome config security regression 43917645405 completed/success; infosvr packet and signal lifecycle stability regression 43917645508 completed/success; AI handoff documentation contract 43917645337 completed/success; AliDDNS signed API transport security regression 43917645276 completed/success; XY-C1 Firmware Validation 43917645503 in_progress; AliyunDrive WebDAV lifecycle stability regression 43917645350 completed/success; Process inventory and lifecycle fail-closed regression 43917645410 completed/success; NVRAM transaction boundary audit 43917645866 completed/success
- Previous NDIS source change `b54a701c35e35549a183eca6bf48c86d5dffb855` has exact-head XY-C1 run `43917115679` still IN_PROGRESS at last poll; the HTTPD umbrella stale storage assertion was corrected in parent `78f7302dad5142402a10b39c08c66a2850abe836`, whose HTTPD run was IN_PROGRESS.
- A local POSIX `sh` and BusyBox `ash` 1.37.0 helper-only test verified exact basename matching, exfat/vfat substring negatives, and `.ko.gz` / `.ko.xz` suffixes. This is not a target BusyBox 1.24.2 or full storage workflow run.
- Physical-router L4 remains NOT VERIFIED; frozen backup remains unchanged.

## Exact first action for the next GPT

1. Inspect exact-head USB storage and HTTPD umbrella workflows for THIS COMMIT; require the catalog regression, all mount-owner checks, and the updated cross-workflow source contract to pass.
2. Inspect exact-head XY-C1 Build/TRX/artifact run for NDIS source SHA `b54a701c35e35549a183eca6bf48c86d5dffb855`; do not infer a later product result from this storage-only commit.
3. Continue with an independent WAN/security issue while checks are pending.
4. Preserve frozen backup and keep physical-router L4 NOT VERIFIED.
