# Cross-session execution mandate — 2026-11-21 checkpoint

## Cross-session execution mandate

- Continue development on main only. Each logical task is one atomic main commit: create all blobs and one complete Git tree, then one commit and a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Fix real source defects with focused regressions; do not weaken checks.
- PENDING Actions do not block independent work. Diagnose the earliest causal failure log.
- Before context exhaustion, finish the current safe slice, inspect the full diff, publish atomically, and preserve the exact next action here.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Inspect live main, verify the frozen backup SHA and check exact-SHA Actions at session end. Update this file only when the baseline, evidence, blocker or first next action materially changes. Do not make no-op commits.

## Current repository state

- Parent main SHA before this slice: `41023c0835fc399ea521e397a8fb5563dd86f02e`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only; do not move it).
- Exact-SHA AI handoff contract #44883618619: SUCCESS; Process lifecycle inventory #44883618647: SUCCESS; Process inventory and lifecycle fail-closed #44883618648: SUCCESS.
- Exact-SHA WAN route regression #44883618627: FAILURE. Its final policy binaries passed, including the detached-daemon GNU89/ASan/UBSan regression and `check_udpxy_startup.py` assertions up to rollback wiring. The first remaining error was a Python `NameError`: `igmpproxy_start` was referenced before its function-body assignment in `scripts/check_udpxy_startup.py`.
- Exact-SHA XY-C1 Firmware Validation #44883618696 and NVRAM transaction audit #44883618624 were PENDING at the last query. Inspect exact results after this commit; PENDING must not stop other independent work.
- The prior detached-daemon rollback and readiness C code is not changed in this slice. The failure is in validator statement order, not a failed production C regression.
- Earlier independent evidence remains: SCUT PID-identity #44882660640 PASS on `9df0573782afb3a3e19864ac17250d7358ef26ca`; NFS rpcbind readiness #44881990807 PASS on `5869188f5f174746cc2dea43c2938be55378c87c`; historical Storage/XY-C1 pass #44880983834/#44880983910 on `646615093f93ca01ea5cbef7dac6b4937a9a5cfa`.
- XY-C1 L3 is NOT VERIFIED unless Build, TRX validation and artifact upload all pass on one identical source SHA. Physical-router L4 remains NOT VERIFIED.

## Active task

1. Inspect the exact-SHA WAN route run for this checker-order correction; fix any remaining first-cause failure without relaxing coverage.
2. Inspect the exact-SHA XY-C1 and NVRAM results and any other newly triggered checks.
3. Once green, return to an independent production stability defect. The next candidate is remaining `kill_pidfile_s()` usage in `trunk/user/shared/src/shutils.c`, traced to its exact PID writers before making any coordinated identity-format change.

## Verification boundary

- On `41023c0835fc399ea521e397a8fb5563dd86f02e`, WAN route's C policy binaries and detached-daemon policy binary passed; the script failed only on the variable-order `NameError` recorded above.
- Process inventory and lifecycle fail-closed run #44883618648 passed on that exact SHA. AI handoff run #44883618619 also passed.
- This checker-order follow-up requires exact-head WAN route success. Firmware Build/TRX/artifact status is a separate XY-C1 L3 gate; hardware L4 is not verified.

## Exact first action for the next GPT

Fetch live main and verify the frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. Resolve this commit SHA and inspect the exact-head WAN route Action first, then XY-C1 and NVRAM. If all relevant checks pass, continue with one focused production fix outside the completed IPTV readiness work; avoid repeating this checker-order issue.

## Handoff maintenance

- Keep `dev_log.md` newest-first; archive only at 120 root records, retaining the newest 20.
- Update `REFACTOR_ROADMAP.md` only when module status, evidence or priority materially changes.
- Keep the frozen backup permanently unchanged.
- Host regression, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
