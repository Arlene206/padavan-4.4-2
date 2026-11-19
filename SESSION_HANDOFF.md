# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- Create all changed blobs, one complete tree and one commit with the current main parent; update main with non-forced compare-and-swap using that exact SHA.
- Verify Actions by exact `head_sha`. A pending run does not block independent work; a failed relevant run must be triaged before claiming validation.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter or delete it. Develop on main.

## Current repository state

- Current MiniUPnP lock hardening source commit: `e736dbfcff830e6ac6baa32026fff7a6334787d1` (`fix(rc): bound MiniUPnP lock EINTR retries`). Acquisition and release both cap consecutive `fcntl(F_SETLK)` EINTR retries at eight; the normal EACCES/EAGAIN timeout is unchanged.
- Exact-head Process lifecycle `44628661363`, MiniUPnP atomic config `44628661018`, and Process inventory `44628661358` PASS on `e736dbfcff830e6ac6baa32026fff7a6334787d1`; this includes the GNU89 lock regression and semantic source checker.
- AI handoff documentation run `44628661127` failed because the checker requires the literal cross-session rule `Before context exhaustion`; this docs-only follow-up restores the required sentence. Exact rerun pending.
- NVRAM audit `44628661528` and XY-C1 firmware validation `44628661156` were IN_PROGRESS at last query. XY-C1 must complete Build, TRX verification and artifact upload on this same production-source SHA before L3 is claimed.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; archival only. PR #1 remains closed/unmerged.
- No physical-router result is available. Host regressions/Actions are L2; XY-C1 Build/TRX/artifact on one exact production SHA is L3; device behavior is L4 NOT VERIFIED.
- No physical-router result is available. Host regressions/Actions are L2; XY-C1 Build/TRX/artifact on one exact production SHA is L3; device behavior is L4 NOT VERIFIED.

## Active task

- Confirmed MiniUPnP lifecycle-lock reliability defect in `trunk/user/rc/src/miniupnpd_lifecycle_lock.h`: both nonblocking `fcntl(F_SETLK)` acquisition and unlock retried `EINTR` indefinitely. The ten-second wait bounded only EACCES/EAGAIN contention; repeated signal interruptions could leave either loop spinning forever.
- This slice introduces an eight-retry cap with preserved `EINTR` error reporting. Acquisition resets the retry count after a non-`EINTR` result, retaining the existing bounded contention wait. Unlock also stops retrying after the cap and closes its descriptor.
- The GNU89 regression covers retry success, exhaustion errno and invalid arguments; the source checker requires the bound in both acquisition and release. The XY-C1 workflow now watches this production header.
- No local compile or exact-head Actions result is claimed for this slice. Process lifecycle and XY-C1 Build/TRX/artifact outcomes must be checked on the new commit.

## Exact first action for the next GPT

1. Fetch live `main` and confirm the exact new source SHA. Query exact-head Process lifecycle, AI handoff-doc, HTTPD security, Storage and XY-C1 Actions; an empty combined status is UNKNOWN, never PASS.
2. For any failed exact-head job, inspect its job log and fix the demonstrated contract/build issue without weakening lock safety or the eight-retry cap. Confirm XY-C1 Build, TRX verification and artifact upload on this same production-source SHA.
3. Continue the remaining cross-userland stability/security queue after validation. Do not repeat implemented SD/MMC notification, PPP fail-closed recovery, APCLI WAN-recovery or SCUT-crontab source audits without new evidence.

## Verification boundary

- L1: Source review found unbounded EINTR retries in MiniUPnP lock acquisition/release; both now use an eight-retry policy while preserving the existing EACCES/EAGAIN timeout.
- L2: GNU89 lock regression and MiniUPnP semantic source checker are extended; exact-head Process Actions are pending. No local compile is claimed.
- L3: XY-C1 now watches `miniupnpd_lifecycle_lock.h`; Build, TRX verification and artifact upload for the exact new source SHA remain to be checked.
- L4: target MIPS/uClibc lock behavior under real signal/load conditions, live MiniUPnP WAN transitions and physical-router behavior remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, product-source baseline, exact-head Actions, module state and this first action. Update this file only when resumable facts materially change; do not create no-op SHA-refresh commits.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken regressions.
- Report exact SHA Actions and keep L1/L2/L3/L4 distinct. Host success is not firmware/hardware success.
