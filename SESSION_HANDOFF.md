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

- Parent main SHA before the SCUT stale-PID serialization slice: `9ff6c64b9731a848ed07c0a08500d2c8608b09e7`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not move or modify it.
- Parent core RC rollback Action runs queued/in progress at the last snapshot: WAN route #44884091464; Process fail-closed #44884091402; XY-C1 #44884091334; AI handoff #44884091323; Storage #44884091287; NVRAM #44884091423. Many independent product regressions were also queued/in progress on the same exact SHA.
- This source slice closes a race between SCUT stale PID equality validation and `rm -f`: overlapping cron invocations could both validate the same stale PID, then one could unlink a PID file just claimed by the other. A persistent adjacent lock inode and `flock -n 9` serialize the complete check/reclaim/publish transaction.
- FD 9 is explicitly closed immediately after the PID claim completes, before NVRAM, ping, authentication or daemon-restart commands; this prevents child processes from inheriting and accidentally extending the lock. The lock file itself is never unlinked; symlink, non-regular and unavailable lock paths fail closed.
- BusyBox 1.24.x `CONFIG_FLOCK` is enabled by default. The SCUT workflow checks that Kconfig dependency and host flock; XY-C1 trigger/test coverage includes the BusyBox Kconfig source because firmware availability is a production dependency.
- Regression pauses the first process inside `rm` after stale-PID comparison, then launches a competing invocation. Exactly one health/NVRAM run must occur. Lock-file symlink refusal is also tested.
- Independent evidence: SCUT PID-identity #44882660640 PASS on `9df0573782afb3a3e19864ac17250d7358ef26ca`; NFS rpcbind readiness #44881990807 PASS on `5869188f5f174746cc2dea43c2938be55378c87c`. Historical Storage/XY pass on `646615093f93ca01ea5cbef7dac6b4937a9a5cfa` does not validate this source tip.
- XY-C1 L3 requires Build, TRX verification and artifact upload to pass on one identical source SHA. Physical-router L4 remains NOT VERIFIED.

## Active task

1. Inspect exact-SHA SCUT watchdog, AI handoff and XY-C1 Actions for this serialization commit; repair any first-cause failure without relaxing tests.
2. While PENDING Actions run, continue independent userland stability work.
3. Next candidate: remaining `kill_pidfile_s()` consumers in `trunk/user/shared/src/shutils.c`; trace each to its PID producer and coordinate identity-bound format changes, especially BusyBox udhcpc and PPPD numeric PID files.

## Verification boundary

- The new lock covers only PID claim/reclaim and is released before health work, avoiding descriptor inheritance by daemon children while preventing overlapping stale-PID unlink transactions.
- BusyBox `flock` source and Kconfig were previously verified: `util-linux/flock.c` accepts an existing FD with `-n`, and `util-linux/Config.src` defaults `CONFIG_FLOCK` to y. This change still requires exact-head SCUT regression and XY-C1 verification.
- No host test or cross-compile was run locally in this connector-only session. Exact-SHA CI is authoritative; XY-C1 L3 and physical L4 are separate.

## Exact first action for the next GPT

Fetch live main and verify the frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. Inspect exact-SHA SCUT, AI handoff and XY-C1 Actions for the lock-scope commit. If SCUT passes, begin a focused shared PID-file producer/consumer review in `trunk/user/shared/src/shutils.c`; do not repeat completed NFS readiness or SCUT PID-identity work.

## Handoff maintenance

- Keep `dev_log.md` newest-first; archive only at 120 root records, retaining the newest 20.
- Update `REFACTOR_ROADMAP.md` only when module status, evidence or priority materially changes.
- Keep the frozen backup permanently unchanged.
- Host regression, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
