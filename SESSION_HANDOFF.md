# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task here.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Latest main parent before this source slice: `8fca58d5479d65fcc8811934f41a71026a240c37`. Product source `d6fac506d564e8277e6c81ec2dd18620b6509144` passed exact-head USB storage run `43904136310` and XY-C1 Build/TRX/artifact run `43904136433`. On parent 8fca, Process run `43907096476` was IN_PROGRESS at last poll; AI handoff docs `43907096238`, AliyunDrive `43907096542`, infosvr `43907096557`, AdGuardHome `43907096423`, and AliDDNS `43907096217` passed. THIS COMMIT adds targeted safe-eject startup retry and requires its own exact-head USB storage and XY-C1 validation. Physical-router L4 remains NOT VERIFIED.

- Repository: `Arlene206/padavan-4.4-2`; current source parent before this slice: `e05ccbcb8c458ee2bc07c6020ed5af52b3d4cff8`.
- Mount-link errno fix, GNU89 policy regression, storage source contracts, and workflow corrections are included together in THIS COMMIT.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- Storage application startup returns first errors from all seven configured services; media/download daemon launch and readiness errors flow through the common startup result, and deferred hotplug preserves retry events until success.
- `create_mp_link()` retains its boolean API but now initializes errno, normalizes mount inventory/path/directory/symlink/cleanup failures, keeps the first actionable error, and exposes `create_mp_link_status()` before all four media callers. MiniDLNA/Firefly retain the 0→1 directory fallback; Transmission/Aria retain mode 0 only.
- USB modem hotplug preserves coalesced unplug/replug events and retries failed reconnects; WAN IPv6 and LAN/Wi-Fi startup paths propagate failures.

## Active task


The process-lifecycle checker hardening is in parent `8fca58d5479d65fcc8811934f41a71026a240c37`; its exact-head Process run `43907096476` was IN_PROGRESS at last poll. Earlier runs exposed checker-only failures in turn (escaped-newline split, then undefined helper); the latest parent defines a local brace-aware `c_function_body()`. Revisit that run before assuming the checker slice is green.

THIS COMMIT fixes a separate P1 storage reliability gap: `safe_remove_stor_device()` previously logged a failed restart while other mounts remained but did not re-arm deferred startup, and it did not schedule a re-check when final mount inventory was UNKNOWN. The fix reuses `usb_hotplug_ms` and the five-second deferred callback, scheduling only after lock release; zero remaining mounts do not trigger a restart. The pure policy test and source contracts were extended; exact-head USB storage and XY-C1 checks are pending for THIS COMMIT.

## Verification boundary


- Product baseline `d6fac506d564e8277e6c81ec2dd18620b6509144`: exact-head USB storage run `43904136310` PASS; XY-C1 run `43904136433` PASS with Build, TRX verification and artifact upload all successful on that same SHA.
- Checker-only parent `8fca58d5479d65fcc8811934f41a71026a240c37`: Process run `43907096476` was IN_PROGRESS at last poll. AI handoff docs, AliyunDrive, infosvr, AdGuardHome and AliDDNS passed on that parent.
- Targeted eject retry source, GNU89 policy tests and source contracts are included in THIS COMMIT. Exact-head USB storage and XY-C1 Build/TRX/artifact are pending; full firmware and physical-router L4 are NOT VERIFIED on this source.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at recorded SHA `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Exact first action for the next GPT


1. Inspect exact-head USB storage and XY-C1 Actions for THIS COMMIT. Confirm the eject-retry policy regression, `scripts/check_storage_safe_remove.py`, Build XY-C1, TRX verification and artifact upload; all must be on the identical source SHA.
2. Recheck Process inventory/lifecycle on parent checker SHA `8fca58d5479d65fcc8811934f41a71026a240c37` (run `43907096476` was pending at last poll). If it failed, inspect its full job log and fix the checker before further refactoring.
3. Continue the remaining storage/hotplug call graph only after those exact-head checks; avoid repeating the completed mount-link and targeted eject-retry slices. Preserve conservative no-restart behavior when no mounts remain.
4. Keep the frozen backup branch unchanged. Physical-router L4 remains NOT VERIFIED.
