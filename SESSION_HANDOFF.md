# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task here.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Latest main commit: `eff340da753ab117ee243d1451d2c5080d99e046`. USB storage `43911560366` passed the preceding 23 checks but failed at new module-loader shell test step 24 with exit 1 and insufficient trace; THIS COMMIT enables xtrace to expose the exact failing command. Docs `43911560363` and Process lifecycle `43911560429` PASS; XY-C1 `43911560375` was IN_PROGRESS at last poll. HTTPD `43911560358` independently failed at step 27; inspect its own log.

- Repository: `Arlene206/padavan-4.4-2`; current source parent before this slice: `e05ccbcb8c458ee2bc07c6020ed5af52b3d4cff8`.
- Mount-link errno fix, GNU89 policy regression, storage source contracts, and workflow corrections are included together in THIS COMMIT.
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- Storage application startup returns first errors from all seven configured services; media/download daemon launch and readiness errors flow through the common startup result, and deferred hotplug preserves retry events until success.
- `create_mp_link()` retains its boolean API but now initializes errno, normalizes mount inventory/path/directory/symlink/cleanup failures, keeps the first actionable error, and exposes `create_mp_link_status()` before all four media callers. MiniDLNA/Firefly retain the 0→1 directory fallback; Transmission/Aria retain mode 0 only.
- USB modem hotplug preserves coalesced unplug/replug events and retries failed reconnects; WAN IPv6 and LAN/Wi-Fi startup paths propagate failures.

## Active task

Module-loader production fix is in parent `eff340da753ab117ee243d1451d2c5080d99e046`, but exact-head USB storage `43911560366` failed only at its new shell regression (step 24), after source syntax passed. The test exited 1 without a diagnostic. THIS COMMIT turns on shell xtrace so the next run reveals the failing command. Do not modify product code until the root cause is visible.
## Verification boundary

- Parent eff340: USB storage `43911560366` FAILED at new module-loader test step 24; preceding 23 steps passed. Root cause not yet known; xtrace added here.
- Parent eff340: Docs `43911560363` and Process `43911560429` PASS. XY-C1 `43911560375` was IN_PROGRESS. HTTPD `43911560358` failed independently at step 27; separate diagnosis is required.
- Frozen backup SHA `72f4f157a084f8184b9597ca5bf334b8080e5807` unchanged. Physical-router L4 remains NOT VERIFIED.
## Exact first action for the next GPT

1. Inspect exact-head USB storage test trace for THIS COMMIT and repair the first failing test command while keeping the production fix only if it remains justified.
2. Check XY-C1 Build, TRX verification and artifact upload on product SHA eff340 `eff340da753ab117ee243d1451d2c5080d99e046`.
3. Inspect HTTPD run `43911560358` step-27 assertion separately; it is outside the automount files in this change.
4. Keep backup branch frozen; L4 remains unverified.
