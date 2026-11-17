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
- Watchdog Process lifecycle `44001230583`, AI handoff `44001230567`, and NVRAM audit `44001230573` PASS on `58fe2c5a`. Same-watchdog-source XY-C1 run `44000632827` on `aeddc157` subsequently PASSed the watchdog policy, XY-C1 Build, TRX verification, and artifact upload; `58fe2c5a` differs only in checker/docs from that production source.
- USB modem tri-state change on `766832746b81a67faaa5d88e3ba78cbd1228faef`: dedicated USB modem run `44002424147`, Process lifecycle `44002424268`, inventory `44002424300`, HTTPD security `44002424357`, infosvr `44002424164`, and AI handoff `44002424146` PASS. Exact-SHA XY-C1 run `44002424380` failed at `scripts/check_usb_modem_driver_config.py` because a legacy assertion still required the old `return available > 0 ? 0 : ENODEV;` policy and Boolean probe callback. This follow-up aligns the source contract with the new tri-state implementation; Build had not started on 7668.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Legacy PR #1 is closed; its head has no unmerged commits relative to its old base.

## Active task

1. Re-run XY-C1 Firmware Validation after updating the stale `check_usb_modem_driver_config.py` assertions; inspect all steps through Build XY-C1, TRX verification and artifact upload. The USB modem-specific GNU89 regression and checker passed on the exact previous source SHA `76683274`.
2. Update current validation records with the completed watchdog Build/TRX/artifact PASS from run `44000632827` on source `aeddc157`; note production firmware C sources remained identical in the checker-only child `58fe2c5a`.
3. Continue the repository-wide RC lifecycle audit for another concrete false-success path only after this source slice clears; do not repeat storage eject, VPN restoration, NVRAM close, or IPv6 neighbour cleanup.

## Verification boundary

- Storage eject rollback: exact-main storage-hotplug, Samba, Process lifecycle and AI handoff regressions PASS on main `6697ca6020b261725655e12c3e5a5252ffb0e9c6`.
- Previous product-source `10380657`: same-SHA XY-C1 Build, TRX verification and artifact upload PASS in run `43998879438`. This source did not include the watchdog strict-probe fix in this commit.
- Current watchdog tri-state source slice: exact-head L2/L3 Actions pending until inspected; no L3 claim is carried forward from the earlier SHA.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and query its `push` Actions by exact `head_sha`. Inspect Process lifecycle, XY-C1 Build/TRX/artifact and AI handoff runs for this watchdog source change. Fix the first causal failure and re-verify its new SHA; if all pass, continue the broad RC service lifecycle review. Preserve the frozen backup and use a complete Git tree/commit with a non-forced expected-SHA compare-and-swap.
