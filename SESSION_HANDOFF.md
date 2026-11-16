# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task here.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Latest main parent: `a52e5c460635cd0080b1857cb04ff92a42f575f5` (includes independent SmartDNS fix `fix(smartdns): preserve unrelated dnsmasq upstreams at boot`). Parent ac012 exact-head USB storage `43913404416`, HTTPD `43913404244`, NVRAM `43913404369`, Process lifecycle `43913404160`, Process inventory `43913404056`, AI docs `43913404348`, AdGuardHome `43913404058`, infosvr `43913404025`, AliyunDrive `43913404158`, and AliDDNS `43913404157` PASS; XY-C1 `43913404095` was IN_PROGRESS at last poll and needs final Build/TRX/artifact status. THIS COMMIT adds tri-state mount ownership handling; exact-head storage and XY-C1 checks pending.

- Repository: `Arlene206/padavan-4.4-2`; current source parent before this slice: `e05ccbcb8c458ee2bc07c6020ed5af52b3d4cff8`.
- Mount-link errno fix, GNU89 policy regression, storage source contracts, and workflow corrections are included together in THIS COMMIT.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- Storage application startup returns first errors from all seven configured services; media/download daemon launch and readiness errors flow through the common startup result, and deferred hotplug preserves retry events until success.
- `create_mp_link()` retains its boolean API but now initializes errno, normalizes mount inventory/path/directory/symlink/cleanup failures, keeps the first actionable error, and exposes `create_mp_link_status()` before all four media callers. MiniDLNA/Firefly retain the 0→1 directory fallback; Transmission/Aria retain mode 0 only.
- USB modem hotplug preserves coalesced unplug/replug events and retries failed reconnects; WAN IPv6 and LAN/Wi-Fi startup paths propagate failures.

## Active task

THIS COMMIT fixes a concrete fail-open path in `mount_from_device()`: its AWK `END` clause collapsed confirmed foreign ownership, no matching `/proc/mounts` record, and an unreadable mount table into one nonzero status. The caller treated every nonzero status as foreign and could fall back to the legacy path despite UNKNOWN ownership. It now snapshots `/proc/mounts` with checked `cat`, returns `0=owned`, `1=confirmed foreign owner`, `2=UNKNOWN/inconsistent`, permits fallback only for status 1, and aborts before any unmount/new mount on status 2. `storage_mountpoint_inventory.sh` tests the exact production function for owned, foreign, conflicting, missing and unreadable records.
## Verification boundary

- Parent ac012: USB storage `43913404416`, HTTPD `43913404244`, NVRAM `43913404369`, Process lifecycle `43913404160`, Process inventory `43913404056`, AI docs `43913404348`, AdGuardHome `43913404058`, infosvr `43913404025`, AliyunDrive `43913404158`, AliDDNS `43913404157` PASS.
- Parent ac012: XY-C1 `43913404095` was IN_PROGRESS at last poll. Verify same-SHA Build, TRX verification and artifact upload.
- New current parent a52 adds an unrelated SmartDNS autostart fix; this commit is built on the new main SHA. Exact-head USB storage and XY-C1 for this mount-ownership change are pending.
- Backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; physical-router L4 NOT VERIFIED.
## Exact first action for the next GPT

1. Inspect exact-head USB storage on THIS COMMIT; the mount-owner shell regression and workflow source assertions must pass.
2. Inspect XY-C1 Build, TRX verification and artifact upload for the same SHA as the automount source change.
3. Confirm the in-progress ac012 XY-C1 run `43913404095` final status without repeating its already-passing USB storage/HTTPD/NVRAM checks.
4. Continue to the next concrete storage/hotplug defect only after exact-SHA gates.
