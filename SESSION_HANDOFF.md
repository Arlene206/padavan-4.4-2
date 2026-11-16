# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task here.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Latest main parent before THIS COMMIT: `1dbfc53c7d9e37fb4a6a5e8358644652a6b3dbfa`. Exact-head checks at last poll: XY-C1 Firmware Validation 43914350624 in_progress; AI handoff documentation contract 43914350195 completed/success; AdGuardHome config security regression 43914350664 completed/success; infosvr packet and signal lifecycle stability regression 43914350210 completed/success; HTTPD Authentication and Security Regression 43914350403 completed/success; AliDDNS signed API transport security regression 43914350230 completed/success; Process lifecycle inventory - 1dbfc53c7d9e37fb4a6a5e8358644652a6b3dbfa 43914350160 completed/success; AliyunDrive WebDAV lifecycle stability regression 43914350198 completed/success; Process inventory and lifecycle fail-closed regression 43914350286 completed/success; USB storage hotplug stability regression 43914350905 completed/success; NVRAM transaction boundary audit 43914350178 completed/success
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- `automount.sh` already uses checked `/proc/modules` snapshots and tri-state `/proc/mounts` ownership. Parent `1dbfc53c7d9e37fb4a6a5e8358644652a6b3dbfa` USB storage run 43914350905 was completed/success.
- A separate review found four `modprobe -l | grep` pipelines for vfat, antfs/ufsd, hfsplus and ext2/ext3. Pipeline status hides a failing BusyBox `modprobe -l`; THIS COMMIT captures the catalog command and propagates its status before selecting a kernel driver.

## Active task

THIS COMMIT closes a module-catalog error-propagation gap: FAT, NTFS, HFS+ and ext2/ext3 selection previously used `modprobe -l | grep`. POSIX pipeline status reflected only grep, so a failing BusyBox `modprobe -l` could be mistaken for a missing driver and lead to fallback-driver selection. Added `func_read_module_catalog()` to capture `modprobe -l` separately and preserve its error status, plus a constrained membership predicate. All four filesystem-family selection groups now refuse to mount if the catalog cannot be read. The regression covers present/missing module names and a nonzero catalog status; the workflow requires four checked call sites and forbids the old masked pipeline.

## Verification boundary

- Parent `1dbfc53c7d9e37fb4a6a5e8358644652a6b3dbfa`: USB storage 43914350905 completed/success; XY-C1 43914350624 in_progress.
- Local POSIX shell helper test and BusyBox ash 1.37.0 helper test passed; this is not firmware BusyBox 1.24.2 and not a full repository test/build.
- Exact-head USB storage and XY-C1 Build/TRX/artifact validation for THIS COMMIT must establish the source boundary. Physical-router L4 remains NOT VERIFIED.
- Frozen backup remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Exact first action for the next GPT

1. Inspect exact-head USB storage; require module-load, module-catalog and mount-owner regressions plus source contracts to pass.
2. Inspect exact-head XY-C1 Build, TRX verification and artifact upload on THIS product SHA.
3. While either run is pending, continue an independent userland stability/security task instead of waiting.
4. Keep backup read-only and physical-router L4 marked NOT VERIFIED.
