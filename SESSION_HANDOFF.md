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

- Parent main SHA before this watchdog rollback slice: `e6b112eb34b7e7fc7f3571e8e823e1ee91a9ea19`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (permanently read-only).
- `1b25077e6e5a9f8750d4b5c76637beec5ca354d1`: SCUT watchdog PID safety #44884209161, AI handoff #44884209044 and Process inventory #44884209082 passed. XY-C1 #44884209133 completed Build, TRX verification and artifact upload on that same SHA; valid L3 evidence for that source revision.
- Process fail-closed #44884209089 failed on an outdated infosvr source-contract assertion. It was corrected in `7085606f16b27e7ebc693cb0c3d3af29eb8e117b`; infosvr #44884338900, crond #44884338915, Process inventory #44884338972 and AI handoff #44884338928 passed.
- On `f530b3a75684feae8490bb3e33044df8034e48b1`, Crond #44884633405, Process inventory #44884633475, Process fail-closed #44884633485, watchdog #44884633410, and independent HTTPD/DNSMasq/WAN/ZeroTier/DDNSTO/VPN/NVRAM/Storage checks passed. XY-C1 #44884633378 was still in progress at last poll.
- `start_watchdog()` launched `/sbin/watchdog` through `eval()` even though `watchdog_main()` daemonizes before publishing its PID. Post-launch failure returns previously skipped rollback, allowing a delayed daemon child to appear after the caller had received failure. This slice targets only the watchdog process, verifies post-stop inventory and preserves the original failure status/errno.
- The initial UNKNOWN process inventory still aborts before launch; an existing watchdog remains a no-op/notification path. New post-launch failure branches route through `detached_daemon_start_rollback_with_ops()`.
- No force push or backup mutation. Physical-router L4 remains NOT VERIFIED.

## Active task

1. Inspect exact-SHA watchdog signal lifecycle, Process lifecycle, AI handoff and XY-C1 Actions for this watchdog rollback commit; diagnose any first-cause failure without weakening coverage.
2. Check whether XY-C1 #44884633378 completed Build, TRX validation and artifact upload on the same `f530b3a` SHA.
3. Continue the shared PID-file audit at `trunk/user/shared/src/shutils.c::kill_pidfile_s()` only after mapping all consumers and writer formats. Do not change numeric-only external PID-file semantics speculatively.

## Verification boundary

- Exact-SHA host Actions above apply only to their named SHAs. The current watchdog rollback change has not yet been verified by its own exact-SHA workflows.
- The existing `detached_daemon_start_policy.c` regression covers cleanup success/failure, UNKNOWN cleanup inventory and primary status/errno preservation. New watchdog source contracts require all six post-launch failure paths to invoke that policy.
- No local compiler, shell or Python run is claimed; this session has no local checkout. XY-C1 L3 requires Build, TRX and artifact upload on one SHA. Physical hardware L4 is NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main` and confirm backup `72f4f157a084f8184b9597ca5bf334b8080e5807`. Inspect the exact-SHA watchdog, Process lifecycle, AI handoff and XY-C1 Actions for this commit. Resolve any first-cause failure; then resume the PID-file consumer/producer mapping in `trunk/user/shared/src/shutils.c`.

## Handoff maintenance

- Keep `dev_log.md` newest-first; archive only at 120 root records, retaining the newest 20.
- Update `REFACTOR_ROADMAP.md` only when module status, evidence or priority materially changes.
- Keep the frozen backup permanently unchanged.
- Host regression, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
