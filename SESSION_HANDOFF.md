# Session Handoff — 2026-11-27

## Repository / branch baseline

- Repository: `Arlene206/padavan-4.4-2`, branch `main`.
- Parent main SHA for this checkpoint: `a6faece21549c01ad0161c7c18d9a26725c3a6fd`. Always read the live branch ref before the next write.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Do not alter it.
- The project startup protocol and this handoff were already recovered earlier in the current work. Do not repeat the completed broad audit.

## Current source slices

### DHCP client (latest production source SHA: db92b3164321618e559051b471f560c7a863d25d)

- Fixed successful DHCPACK event-hook runtime accounting: lease clock starts at ACK receipt (before optional ARP work and callback); callbacks crossing T1 start renewal immediately with the true remaining lease budget; a callback finishing at/after full expiry is deconfigured before discovery. Failed cleanup uses existing DECONFIG_PENDING.
- Fixed failed initial `bound` hook rollback: because the hook may have partially applied addresses/routes, run `deconfig` before rediscovery; if cleanup fails, stay in DECONFIG_PENDING and resume INIT_SELECTING only after cleanup succeeds. Failed `renew`/`rebind` hooks preserve the current active lease and retry deadline.
- Added GNU89/Werror/ASan/UBSan helper regressions and source-order checks.
- The checker-only descendants `6a89890e57e3be78d921a8fbd703ce245cf70790`, `c9b0d86bbd9444e82498df58cb68f9898d513f6c`, and THIS handoff checkpoint fix brittle multiline call matching and stale `retry_continue` references. No production source changed in those descendants.

### PID 1 watchdog (production source commit c9f5d0e1abd9df58576b18db4e8d10db0e448066)

- Watchdog readiness/start/rollback now requires both watchdog argv0 basename and the resolved `/sbin/rc`/watchdog executable identity, with PID/start-time/argv revalidation.
- Failed-start cleanup signals only verified watchdog instances (TERM, bounded KILL and final scan) instead of generic `kill_services()`. Shared generic process inventory semantics remain unchanged.
- Added GNU89/Werror/ASan/UBSan executable-identity policy tests. Process lifecycle test passed on SHA `44f801f34ef7e8d908b9a5d7f5bac5e56d0ac4d2`; watchdog-signal workflow passed on `3d6d2d712a3c5368f327810dd5adb73c8af70932`. These earlier SHA results do not replace exact-head checks.

### Optional IAPP event logger (source commit 78ff00d25b0957ecc2c148d606aee12615fbaa1d and follow-ups)

- Replaced wall-clock gettimeofday/timersub event polling with CLOCK_MONOTONIC timespec deadlines; handles backwards samples and transient clock-read failures.
- Added the monotonic interval tests and migrated existing ioctl regression to the timespec API. IAPP lifecycle run `#46095378230` passed on SHA `aeb85731f829fdcb7d52d7d359b32b79fc70987d`; later related commits changed checker/tests, so check current-head Actions.

### Storage media publisher

- Corrected `check_storage_safe_remove.py` to match lstat-based target validation and the publisher's validated `force_first_valid` forwarding.
- USB storage hotplug, AI docs and XY-C1 workflow passed on `363fd7fba6f90352d70bb581ea6d0690a573441c`; however firmware Build/TRX/artifact steps were SKIPPED by the firmware-input detector. This is not L3 build evidence.

## Verification boundary at checkpoint

- The udhcpc policy sanitizer test passed on earlier exact-head attempts. Its source checker then failed on stale assertions / missing variable names; the current `main` checker fixes those known failures, but an exact-head udhcpc rerun for `a6faece21549c01ad0161c7c18d9a26725c3a6fd` has not yet been verified here.
- AI documentation validation passed on several preceding SHAs; confirm the run for `a6faece21549c01ad0161c7c18d9a26725c3a6fd`.
- XY-C1 run `#46095697024` was still being polled on production source SHA `db92b3164321618e559051b471f560c7a863d25d`; check Build, TRX verification and artifact upload outcomes. No L3 claim is made.
- No local MIPS cross-build or physical-router test was run. Hardware/L4 remains NOT VERIFIED.

## Exact first actions for the next session

1. Query GitHub Actions for exact current head `a6faece21549c01ad0161c7c18d9a26725c3a6fd`. Confirm udhcpc integration, AI docs and any workflow triggered by the checker-only commit. If a test fails, inspect its job log and fix the actual assertion or source defect.
2. Poll XY-C1 run `#46095697024` for source SHA `db92b3164321618e559051b471f560c7a863d25d`. L3 requires Build, TRX verification and artifact upload all PASS on that same source SHA.
3. Continue the next open P1 stability/security task in an independent userland module; do not wait for pending build work and do not repeat completed storage/process/IAPP audits.

## Cross-session execution mandate

- Treat the repository as one continuous engineering effort. Recover once, resume the exact unfinished task, and do not ask the user what to do next.
- For each logical task, create all blobs, one complete tree and one atomic commit. Update `main` only with `force:false` and expected-SHA compare-and-swap. Never create partial-file commits, force-push or modify the frozen backup.
- After every commit, inspect Actions for that exact SHA. PASS permits the next task; FAIL requires diagnosis and repair; PENDING does not block independent work.
- Before context exhaustion, finish a safe bounded slice, verify its exact-SHA test/build boundary, and update this handoff with the committed baseline and precise first action.
- Keep source-verification SHA and later documentation-only SHA distinct. Never describe skipped build steps as a successful firmware build; never claim hardware/L4 without actual hardware testing.

