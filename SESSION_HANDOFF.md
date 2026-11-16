# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task here.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Latest main commit: `393c1c489e0558d4638e37fef29ce236e1b9d0bd` (exact-head USB storage `43909013086`, Process lifecycle `43909012933`, and AI docs `43909013100` all PASS). Product parent c1aca `c1aca046300ec7738a3455c779afb988f5d71cb6` passed NVRAM `43907888090` and XY-C1 `43907888399`, with Build, TRX and artifact upload all successful on that same SHA. THIS COMMIT adds fail-closed filesystem module-load locking and its extracted-function regression; exact-head storage and XY-C1 checks are pending.

- Repository: `Arlene206/padavan-4.4-2`; current source parent before this slice: `e05ccbcb8c458ee2bc07c6020ed5af52b3d4cff8`.
- Mount-link errno fix, GNU89 policy regression, storage source contracts, and workflow corrections are included together in THIS COMMIT.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- Storage application startup returns first errors from all seven configured services; media/download daemon launch and readiness errors flow through the common startup result, and deferred hotplug preserves retry events until success.
- `create_mp_link()` retains its boolean API but now initializes errno, normalizes mount inventory/path/directory/symlink/cleanup failures, keeps the first actionable error, and exposes `create_mp_link_status()` before all four media callers. MiniDLNA/Firefly retain the 0→1 directory fallback; Transmission/Aria retain mode 0 only.
- USB modem hotplug preserves coalesced unplug/replug events and retries failed reconnects; WAN IPv6 and LAN/Wi-Fi startup paths propagate failures.

## Active task

THIS COMMIT fixes a specific automount race: `func_load_module()` previously ignored `flock 9` failure and continued to `modprobe`, so concurrent hotplug workers could load filesystem modules without serialization. It also treated an unreadable `/proc/modules` as an absent module. The helper now distinguishes UNKNOWN inventory from absence, refuses `modprobe` without the lock, rechecks under lock, propagates `modprobe` failure, and mount branches abort on error. New `storage_module_load_lock.sh` extracts and executes the production function against mock `flock`/`modprobe`, covering lock failure, the post-lock race, failure propagation and invalid inputs. The equivalent local shell harness passed; exact-head Actions are pending.
## Verification boundary

- Parent `393c1c489e0558d4638e37fef29ce236e1b9d0bd`: USB storage `43909013086`, Process lifecycle `43909012933`, AI docs `43909013100` PASS.
- Product parent c1aca: NVRAM `43907888090` PASS; XY-C1 `43907888399` PASS with Build, TRX verification and artifact upload on the same SHA.
- THIS COMMIT changes `automount.sh` and adds an extracted-function shell regression; exact-head USB storage and XY-C1 Build/TRX/artifact are pending. Physical-router L4 NOT VERIFIED.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at SHA `72f4f157a084f8184b9597ca5bf334b8080e5807`.
## Exact first action for the next GPT

1. Inspect exact-head USB storage on THIS COMMIT; the extracted-function shell regression and Python caller-graph checks must pass.
2. Inspect exact-head XY-C1 Build, TRX verification and artifact upload on the same product SHA; do not carry c1aca L3 through this automount change.
3. After those gates, continue the remaining automount/notification reliability work. The existing contracts were checked: `_eval()` is synchronous and returns child exit status when `ppid` is NULL; `notify_rc()` is a void asynchronous queue/signal API.
4. Preserve the frozen backup and keep physical-router L4 marked NOT VERIFIED.
