# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Continue one engineering effort across GPT conversations: avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe slices, add regression coverage, inspect exact-SHA Actions, commit atomically and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

Before context exhaustion, inspect the current source SHA, relevant exact-head Actions, validation boundary and first actionable task; then leave this handoff accurate for the next GPT.

- Repository: `Arlene206/padavan-4.4-2`; this source slice is based on parent `6697ca6020b261725655e12c3e5a5252ffb0e9c6`.
- Safe-storage-eject rollback is implemented: failed stop preflight restores previously stopped storage apps while retaining the original stop failure and refusing unmount. Exact-main storage run `44000009287` PASS; Samba source-contract run `44000009517` PASS; the exact-main Process inventory/lifecycle run `44000009230` PASS.
- XY-C1 run `43998879438` passed the watchdog policy and full Build XY-C1, TRX verification and artifact upload on product-source SHA `10380657db522193a781d7e3068c0c2f02a8b78f`. Product firmware source did not change in the later checker-only commits `20a1266e` and `6697ca60`; this L3 evidence does not cover the watchdog strict-probe change in the current source slice.
- A CPU watchdog reliability defect is corrected in this slice: the old module-state callback used Boolean `is_module_loaded()`, collapsing a negative sysfs probe error into false/ABSENT. A disable restart could therefore report success without verifying module absence. The callback now uses tri-state `is_module_loaded_status()` and returns probe errno; boot startup uses the same strict probe. UNKNOWN and invalid values have regression coverage.
- Exact-main AI handoff run `44000009196` PASS. NVRAM transaction audit `44000009260` was still in progress at the last check; inspect its result only if it remains relevant after this source slice.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Legacy PR #1 is closed; its head has no unmerged commits relative to its old base.

## Active task

1. Verify exact-head push Actions for this watchdog tri-state source slice: Process lifecycle regression, XY-C1 Firmware Validation and AI handoff documentation contract. Confirm the run event is `push` and each run's `head_sha` equals live main.
2. Confirm GNU89/`-Werror`/ASan/UBSan policy tests cover UNKNOWN after both load and unload, invalid probe values, and existing loaded/absent/busy cases; inspect the integration checker for strict startup and restart wiring.
3. Confirm same-SHA XY-C1 Build, TRX verification and artifact upload. The earlier `10380657` build predates the `services.c` strict-probe change.
4. If exact-head checks pass, continue the P1 RC startup/service lifecycle audit and look for other lossy Boolean wrappers around APIs that expose tri-state/error status. Do not repeat the storage eject, VPN restoration, NVRAM close, or IPv6 neighbour cleanup work.

## Verification boundary

- Storage eject rollback: exact-main storage-hotplug, Samba, Process lifecycle and AI handoff regressions PASS on main `6697ca6020b261725655e12c3e5a5252ffb0e9c6`.
- Previous product-source `10380657`: same-SHA XY-C1 Build, TRX verification and artifact upload PASS in run `43998879438`. This source did not include the watchdog strict-probe fix in this commit.
- Current watchdog tri-state source slice: exact-head L2/L3 Actions pending until inspected; no L3 claim is carried forward from the earlier SHA.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and query its `push` Actions by exact `head_sha`. Inspect Process lifecycle, XY-C1 Build/TRX/artifact and AI handoff runs for this watchdog source change. Fix the first causal failure and re-verify its new SHA; if all pass, continue the broad RC service lifecycle review. Preserve the frozen backup and use a complete Git tree/commit with a non-forced expected-SHA compare-and-swap.
