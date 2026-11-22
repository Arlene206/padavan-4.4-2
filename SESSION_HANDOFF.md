# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository and complete an end-of-session accuracy checkpoint before ending. Record current source SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts and first action remain accurate, explicitly conclude the check with no file change and no commit. Use one atomic main commit for material changes only; never create a no-op commit. Before context exhaustion, leave the next GPT a precise first action and truthful verification status.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a fetched expected SHA.
- Check Actions for the exact source SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; branch: `main`.
- Parent main SHA for the current engineering milestone: `280c05e3f96205377b8a4515a8f62b787782b484`. The source/test milestone in the commit containing this handoff is the active product baseline; resolve its exact SHA from the live main ref.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- WAN/PPP, USB modem, Inadyn, DNSMasq and manual Wi-Fi radio checked-read fixes remain in main; do not repeat those completed slices without new evidence.

## Active task

`notify_run_detect_internet()` previously ignored the return value from `nvram_set_int_temp("di_notify_delay", delay_time)` and proceeded to process discovery, process launch or SIGUSR1 notification. For an already-running monitor, SIGUSR1 causes the daemon to read this NVRAM value; a failed write could therefore resume probing with a stale delay and still report success.

The current milestone now checks the write result and errno, normalizes an opaque failure to EIO, logs it, and returns before process inventory or launch/resume signaling. A GNU89 test pins success, errno-preserving failure, errno-less failure and positive status normalization. The existing detect-internet workflow now asserts the fail-before-side-effect ordering.

## Verification boundary

- No local repository checkout or local C/Python execution was available in this connector-only session.
- Parent SHA `280c05e3f96205377b8a4515a8f62b787782b484`: Wireless radio regression #45035999914 PASS; AI handoff #45035999840, HTTPD #45035999885, Process inventory #45035999887, Process lifecycle #45035999801, NVRAM #45035999880, watchdog #45035999886 and iNIC #45035999873 PASS on the exact SHA.
- Parent XY-C1 run #45035999874 is a push run on that exact SHA. All host/policy steps passed; `Build XY-C1` was in progress at takeover, with TRX verification and artifact upload pending. This is not an L3 pass.
- Current detect-internet regression and XY-C1 results must be checked on the exact new main source SHA. L3 requires Build, TRX verification and artifact upload all PASS on one identical SHA. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the exact-head `detect-internet signal lifecycle stability regression` and XY-C1 run for the current source SHA. Diagnose any failure; do not infer success from the parent run above.
2. Continue repository-wide stability/security implementation outside the already-completed WAN/PPP, USB modem, Inadyn, DNSMasq and manual-radio slices. Prefer the next concrete, independent caller/lifecycle failure from the roadmap.
3. Keep the backup immutable and continue all development directly on `main`.
