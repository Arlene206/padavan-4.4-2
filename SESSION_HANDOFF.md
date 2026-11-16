# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task here.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Latest main commit: `8795f568cf1c53d8aa9b045b5a1074d3634e8f42`. HTTPD run `43911560358` failed at step 27 only because its integration checker still expected the old module lock redirection; THIS COMMIT aligns the checker with `) 9>>"$lock_file"`, the default lock path and fail-closed flock status. USB storage run `43911560366` failed at new regression step 24; test xtrace was added in parent `8795f568cf1c53d8aa9b045b5a1074d3634e8f42`, replacement storage run pending. XY-C1 `43911560375` was still in progress on product SHA eff340.

- Repository: `Arlene206/padavan-4.4-2`; current source parent before this slice: `e05ccbcb8c458ee2bc07c6020ed5af52b3d4cff8`.
- Mount-link errno fix, GNU89 policy regression, storage source contracts, and workflow corrections are included together in THIS COMMIT.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- Storage application startup returns first errors from all seven configured services; media/download daemon launch and readiness errors flow through the common startup result, and deferred hotplug preserves retry events until success.
- `create_mp_link()` retains its boolean API but now initializes errno, normalizes mount inventory/path/directory/symlink/cleanup failures, keeps the first actionable error, and exposes `create_mp_link_status()` before all four media callers. MiniDLNA/Firefly retain the 0→1 directory fallback; Transmission/Aria retain mode 0 only.
- USB modem hotplug preserves coalesced unplug/replug events and retries failed reconnects; WAN IPv6 and LAN/Wi-Fi startup paths propagate failures.

## Active task

Two independent checker follow-ups are open. First, USB storage `43911560366` failed step 24 in `storage_module_load_lock.sh` without enough output; parent `8795f568cf1c53d8aa9b045b5a1074d3634e8f42` enables shell xtrace for the exact command. Second, HTTPD run `43911560358` failed its cross-workflow assertion because it still expected the original `) 9>/var/lock/fs_module.lock` redirection. THIS COMMIT updates that assertion to match the parameterized `) 9>>"$lock_file"` production fix while still requiring fail-closed flock behavior. Product automount code is unchanged.
## Verification boundary

- Product source eff340: USB storage `43911560366` failed at new shell test step 24; the preceding 23 checks passed. Root cause is not yet known; xtrace is in parent 8795.
- HTTPD `43911560358` failed solely at step 27, line 69, because of the stale lock-redirection assertion. THIS COMMIT corrects it; exact-head HTTPD validation pending.
- XY-C1 run `43911560375` was in progress on product SHA eff340. Docs and Process runs on eff340 passed.
- Frozen backup remains unchanged; physical-router L4 NOT VERIFIED.
## Exact first action for the next GPT

1. Inspect USB storage run on parent `8795f568cf1c53d8aa9b045b5a1074d3634e8f42` after xtrace; identify the first failing test command and fix it.
2. Inspect exact-head HTTPD run for THIS COMMIT to verify the new cross-check assertions.
3. Inspect XY-C1 run `43911560375` for same-SHA Build, TRX verification and artifact upload on eff340.
4. Keep the backup branch unchanged and do not claim hardware L4.
