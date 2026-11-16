# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task here.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Latest main parent before THIS COMMIT: `06bfebfa986d6b6b0c7c61a98ee389327019a7af`. Exact-head Actions at last poll: NVRAM transaction boundary audit 43916101346 queued; AliyunDrive WebDAV lifecycle stability regression 43916101333 queued; XY-C1 Firmware Validation 43916101276 in_progress; Process lifecycle inventory - 06bfebfa986d6b6b0c7c61a98ee389327019a7af 43916101950 in_progress; AdGuardHome config security regression 43916101942 completed/success; AliDDNS signed API transport security regression 43916101928 in_progress; infosvr packet and signal lifecycle stability regression 43916101447 in_progress; USB storage hotplug stability regression 43916101880 in_progress; Process inventory and lifecycle fail-closed regression 43916101921 in_progress; HTTPD Authentication and Security Regression 43916101999 in_progress; AI handoff documentation contract 43916102006 completed/success
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- `automount.sh` uses checked `/proc/modules` snapshots, checked `modprobe -l` catalog capture, and a tri-state mount-table helper. Preflight, revalidation, and post-mount checks all use `/proc/mounts`; mountpoint ownership is verified against the exact source device.

## Active task

THI## Active task

THIS COMMIT changes only the storage workflow's source-contract range: the checker now slices `mount_from_device()` through the actual caller comment immediately after the helper, rather than through an earlier device-name comment. Without this correction, the Python source checker would inspect an empty range and fail despite the intended production-source assertions.

## Verification boundary

- Parent `06bfebfa986d6b6b0c7c61a98ee389327019a7af` exact-head Actions at last poll: NVRAM transaction boundary audit 43916101346 queued; AliyunDrive WebDAV lifecycle stability regression 43916101333 queued; XY-C1 Firmware Validation 43916101276 in_progress; Process lifecycle inventory - 06bfebfa986d6b6b0c7c61a98ee389327019a7af 43916101950 in_progress; AdGuardHome config security regression 43916101942 completed/success; AliDDNS signed API transport security regression 43916101928 in_progress; infosvr packet and signal lifecycle stability regression 43916101447 in_progress; USB storage hotplug stability regression 43916101880 in_progress; Process inventory and lifecycle fail-closed regression 43916101921 in_progress; HTTPD Authentication and Security Regression 43916101999 in_progress; AI handoff documentation contract 43916102006 completed/success
- Production source change is `06bfebfa986d6b6b0c7c61a98ee389327019a7af`; its exact-head Actions are triggered by the preceding push. The workflow range correction itself requires a fresh exact-head storage regression.
- Local host POSIX `sh` and BusyBox `ash` 1.37.0 tests for the catalog helper passed; no target BusyBox 1.24.2/full firmware result is claimed.
- Physical-router L4 remains NOT VERIFIED. Frozen backup unchanged.

## Exact first action for the next GPT

1. Inspect exact-head USB storage; require module-load/catalog, mount-owner tri-state and source-contract checks to pass.
2. Inspect exact-head XY-C1 Build, TRX inspection and artifact upload for THIS source SHA.
3. If Actions are pending, continue an independent WAN/DHCP or modem lifecycle task rather than waiting.
4. Preserve backup and do not claim physical-router L4.
