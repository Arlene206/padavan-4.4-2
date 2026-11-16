# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task here.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Latest main parent before THIS COMMIT: `dc4630eb436381b0c19bdbcf067fc2cdc2cfa7df`. Exact-head checks at last poll: XY-C1 Firmware Validation 43915736119 in_progress; Process inventory and lifecycle fail-closed regression 43915736081 completed/failure; NVRAM transaction boundary audit 43915735838 in_progress; AI handoff documentation contract 43915735818 completed/success; Process lifecycle inventory - dc4630eb436381b0c19bdbcf067fc2cdc2cfa7df 43915735556 completed/success; USB storage hotplug stability regression 43915735430 completed/failure
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- `automount.sh` now uses checked module inventory snapshots and checked module-catalog queries; filesystem-driver selection aborts instead of guessing when `modprobe -l` fails.
- The previous mount-owner helper correctly distinguished owned/foreign/absent/unknown inventory states, but callers still gated it behind `mountpoint -q`. A generic nonzero result from that command was treated as "not mounted", bypassing the checked inventory on an uncertain mountpoint. THIS COMMIT removes that gate and uses one checked `/proc/mounts` tri-state query for preflight, revalidation, safe unmount and final device-ownership verification.

## Active task

THIS COMMIT makes mount ownership the authoritative decision for the whole automount transaction. `mount_from_device()` returns 0=owned, 1=confirmed foreign, 2=absent, 3=UNKNOWN/malformed. Callers permit fallback only for confirmed foreign ownership, abort on UNKNOWN, re-read immediately before unmount, and require the final mount-table entry to name the exact source device. The script no longer relies on `mountpoint -q` for either initial ownership or mount postcondition. The extracted regression covers owned, foreign/conflicting, absent, unreadable and malformed mount inventories.

## Verification boundary

- Parent `dc4630eb436381b0c19bdbcf067fc2cdc2cfa7df` exact-head Actions at last poll: XY-C1 Firmware Validation 43915736119 in_progress; Process inventory and lifecycle fail-closed regression 43915736081 completed/failure; NVRAM transaction boundary audit 43915735838 in_progress; AI handoff documentation contract 43915735818 completed/success; Process lifecycle inventory - dc4630eb436381b0c19bdbcf067fc2cdc2cfa7df 43915735556 completed/success; USB storage hotplug stability regression 43915735430 completed/failure
- The preceding catalog-helper host test passed under POSIX `sh` and BusyBox `ash` 1.37.0. This does not substitute for target BusyBox 1.24.2 or full firmware validation.
- Exact-head USB storage and XY-C1 Build/TRX/artifact validation for THIS COMMIT are required. Physical-router L4 remains NOT VERIFIED.
- Frozen backup remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Exact first action for the next GPT

1. Inspect exact-head USB storage; require module-load/catalog, mount-owner tri-state and source-contract checks to pass.
2. Inspect exact-head XY-C1 Build, TRX inspection and artifact upload for THIS source SHA.
3. If Actions are pending, continue an independent WAN/DHCP or modem lifecycle task rather than waiting.
4. Preserve backup and do not claim physical-router L4.
