# Cross-session execution mandate — 2026-11-21 checkpoint

## Cross-session execution mandate

- Continue development on `main` only. Each logical task is one atomic commit: create all blobs and one complete Git tree, then one commit and a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Fix real defects with focused regressions; do not weaken checks.
- PENDING Actions do not block independent work. Diagnose the earliest causal failure log.
- Before context exhaustion, finish the current safe slice, publish atomically and leave the exact first next action here.

## Mandatory handoff checkpoint

Inspect live `main`, verify the frozen backup SHA and inspect exact-SHA Actions at session end. Update this document only when baseline, evidence, blocker or first next action materially changes; do not create no-op commits.

## Current repository state

- Parent main SHA before this workflow-contract correction: `8a55d24a4b797c96ca94bc8ece112966d86fa204`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and must remain read-only.
- `1b25077e6e5a9f8750d4b5c76637beec5ca354d1` SCUT watchdog #44884209161, AI handoff #44884209044 and Process inventory #44884209082 passed; XY-C1 #44884209133 completed Build, TRX verification and artifact upload on that same SHA (L3 pass for those inputs).
- `f530b3a75684feae8490bb3e33044df8034e48b1` Crond #44884633405, Process inventory #44884633475, Process fail-closed #44884633485 and watchdog #44884633410 passed. Its XY-C1 #44884633378 was still building at last poll.
- Watchdog rollback source commit `8a55d24a4b797c96ca94bc8ece112966d86fa204` has exact-SHA Watchdog #44885239087, Process inventory #44885238990, XY-C1 #44885238962, DNSMasq #44885238979 and HTTPD #44885238949 running. The source/checker/workflow contract must pass; no success is inferred from PENDING.
- The first watchdog Action uses an inline workflow contract that still expected `return launch_status;`. The production source now returns through the new rollback helper. This commit updates that stale contract and includes the detached rollback policy header/test in the workflow path contract; it does not change production behavior.
- No force push, no backup mutation and no physical-router L4 claim.

## Active task

1. Inspect exact-SHA Watchdog, Process lifecycle, HTTPD/DNSMasq and XY-C1 runs for this workflow-contract commit.
2. Recheck `8a55d24a4b797c96ca94bc8ece112966d86fa204` Actions, especially whether its stale inline watchdog contract failed before this correction.
3. If all relevant checks pass, continue mapping `kill_pidfile_s()` and `kill_pidfile()` consumers to exact PID-file producers before any shared-helper migration.

## Verification boundary

- The `8a55d24` source slice's generic rollback behavior is covered by `detached_daemon_start_policy.c` tests: cleanup success, failed cleanup with child still running, UNKNOWN cleanup inventory and primary status/errno preservation. Exact production wiring is source-checked.
- The workflow-contract correction is not yet verified on its own SHA. No local compile or test run is claimed.
- XY-C1 L3 requires Build, TRX and artifact upload on one SHA. Physical hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main`, confirm the frozen backup SHA, and inspect the exact-SHA Watchdog, Process lifecycle, HTTPD/DNSMasq and XY-C1 Actions for this commit. Resolve first-cause failures; then resume the shared PID-file consumer/producer mapping in `trunk/user/shared/src/shutils.c`.

## Handoff maintenance

- Keep `dev_log.md` newest-first; archive only at 120 root records, retaining the newest 20.
- Update `REFACTOR_ROADMAP.md` only when module status, evidence or priority materially changes.
- Keep the frozen backup permanently unchanged.
- Host regression, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
