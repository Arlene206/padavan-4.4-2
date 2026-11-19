# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning.

- Create all changed blobs, one complete tree and one commit with the current main parent; update main with non-forced compare-and-swap using that exact SHA.
- Verify Actions by exact `head_sha`. A pending run does not block independent work; a failed relevant run must be triaged before claiming validation.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter or delete it. Develop on main.

## Current repository state

- Current main parent for this lock-hardening slice: `9fd21db80a40dddbf64d24af1505d0e974375052` (`test(rc): align hotplug and restart contracts`). PPP fail-closed recovery, APCLI WAN result propagation and SD/MMC notification error propagation are already on main; do not repeat those source audits.
- Product-source SHA `3488f129babe376bccf27e132776c45910614dd7` passed XY-C1 Build, TRX verification and artifact upload in run `44625809599` (L3 for that exact source SHA only).
- Exact b4 follow-up: Wireless `44627098078` PASS. Process `44627098093` failed a stale assertion looking at an earlier APCLI status merge instead of the later IAPP failure merge. Storage `44627098102` and HTTPD security `44627098106` failed stale expectations for legacy `notify_rc()` after the checked queue change. AI docs `44627098108` failed because the mandatory checkpoint sentence had been shortened; `9fd21db` repairs these checker/docs contracts. Exact reruns on `9fd21db` remain unverified.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is archival only. PR #1 remains closed/unmerged.
- The XY-C1 path filter did not watch `miniupnpd_lifecycle_lock.h`; this slice adds it to the push filter so a production-header change triggers a full firmware validation.
- Root `dev_log.md` contains 32 dated records before this entry; it remains below the 120-record rotation threshold.
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
