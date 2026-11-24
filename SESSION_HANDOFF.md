## Current repository state

- Development target: `main` only. Source parent before this procfs fallback checkpoint: `9e602b00973a2c41c109fc47dc50b57f43ecf7ba`; the current atomic commit updates the process helper, regression, source checker and handoff together. Re-read the live `main` ref before the next write.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is read-only.
- Legacy PR #1 is closed and unmerged. The AliDDNS HMAC helper/test and CA trust-store bootstrap are already present in `main`.

## Active task — validate SmartDNS procfs-root fallback

- The SmartDNS lifecycle helper verifies executable, argv and kernel start_time before signalling; the regression now covers PID replacement and keeps the replacement process unsignalled.
- A strict-shell edge remained: `smartdns_process_state()` and `smartdns_process_identity()` initialized `/proc`, then expanded `$SMARTDNS_PROC_ROOT` without a default. An unset override aborts in callers using `set -u`; the confirmed-absent lookup needs `/proc/1/stat` even when no candidate identity function has run.
- The helper now uses `${SMARTDNS_PROC_ROOT:-/proc}` in both functions. The test explicitly unsets the override, expects a confirmed absent result via the host `/proc`, then creates its isolated fake proc tree. The Python checker pins both safe defaults and that regression.

## Verification boundary

- Parent `9e602b00973a2c41c109fc47dc50b57f43ecf7ba`: AI handoff documentation #45420686264 PASS; DNSMasq lifecycle #45420685909 QUEUED at last observation.
- Parent DNSMasq job #45420427031 failed only because its source checker expected an obsolete test phrase; that checker was corrected in `9e602b0`. On the preceding run, all runtime SmartDNS process, DNSMasq preservation, Adbyby ownership and atomic configuration tests passed before the stale assertion.
- Require fresh exact-head DNSMasq/Process/AI/XY-C1 Actions for the safe-default change. L3 requires Build, TRX verification and artifact upload PASS on the same source SHA; physical-router L4 remains unverified.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head DNSMasq and Process regression results for the safe procfs fallback; fix any actual test/source failure without repeating already-passing test suites.
2. Check the exact-head XY-C1 Build/TRX/artifact state; do not claim L3 unless all three pass on that SHA. When firmware CI is pending, continue independent work.
3. Proceed to the separate RAETH `RAETH_SET_LAN_IP` lock-balance issue: after validating a copied buffer, the invalid-input branch can break before reacquiring `page_lock`, while the common epilogue unconditionally unlocks. That source fix remains uncommitted.

## Mandatory handoff checkpoint

Before handoff, inspect live `main`, relevant exact-head push Actions, source baseline, validation boundary, blockers and the precise first action. Update only when the resumable facts change.

## Cross-session execution mandate

Recover once, then implement. Use source-led bounded fixes, meaningful regressions, one complete Git tree/commit, and a non-forced expected-SHA update of `main`. Do not force-push, use work branches/PRs, wait idly on pending workflows, or overstate host/firmware/hardware verification.
