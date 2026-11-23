# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Development branch: `main` only. Do not create feature branches or PRs; never force-push.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Preserve it permanently; do not modify, rename, delete or check it out.
- Current source checkpoint: THIS COMMIT (RC PID1 console recovery shutdown/throttle integration); parent main SHA: `aa163f6cc5993029684fd55a489528f3133fbf26`. Fetch live `main` on takeover; do not chase a docs-only SHA.
- Continue implementation rather than repeating completed audits or asking what to do next.

## Active task

The actual router PID 1 is RC init, `trunk/user/rc/src/init.c` (the board BusyBox `CONFIG_INIT` is disabled). The current bounded fix addresses a shutdown race in console recovery: if SIGTERM/SIGQUIT arrived during the one-second throttle after a fast shell exit, the old loop kept sleeping until timeout and then could fork/exec another recovery shell before the outer loop processed shutdown.

This commit:
- Makes the retry wait condition observe `signal_state.shutdown_pending`, so an ordinary shutdown cancels the remaining throttle immediately.
- Breaks out of the event-wait loop before any relaunch/suspend work when shutdown is pending, and also guards the console launch/suspend branches against an already-published shutdown flag.
- Adds GNU89 policy regressions for normal delay, remaining-delay cancellation, and zero remaining time; extends the PID1 workflow source contract to pin the production signal guard and branch ordering.

The preceding recovery-shell retry implementation remains in parent `36bd2b1e01c0472b7d8fdc0a31e4a56e3b6762de`; its corrected console retry test now passed in exact-head PID1 mdev run `45087866598` on `aa163f6cc5993029684fd55a489528f3133fbf26`.

## Verification boundary

Exact push runs for parent `aa163f6cc5993029684fd55a489528f3133fbf26`:
- PID1 mdev/security `45087866598`: PASS, including the corrected console retry test compile/run and all source contracts.
- AI handoff documentation `45087866713`: PASS.
- XY-C1 host validation `45087866595`: PASS; firmware Build, TRX and artifact upload were correctly skipped because that parent commit changed only CI/docs. This is not L3 validation for the source baseline.

For THIS COMMIT, the PID1 mdev and XY-C1 firmware workflows should trigger from the RC source/header/test changes; inspect only push runs whose `head_sha` equals the fetched new main SHA. No local checkout/compile was possible in this connector-only environment. Do not claim host test, MIPS build/TRX/artifact or hardware success until those exact results are observed. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` once and confirm the frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not touch the backup.
2. Inspect the exact-head PID1 mdev run on the new source SHA. Confirm GNU89/ASan/UBSan console policy tests and the full signal/console source contract pass.
3. Inspect the exact-head XY-C1 run for the same SHA; verify whether Build XY-C1, TRX verification and artifact upload actually executed and passed. The parent docs-only result `45087866595` is host-only and cannot satisfy L3.
4. After relevant exact-head checks pass, continue the focused RC PID1 lifecycle review. Consider further child/signal edge cases only with concrete source evidence and injected regression coverage.
5. Keep each source slice atomic: create the complete Git tree and one commit on latest main, then publish with `force:false` and `expected_sha` equal to the freshly fetched parent.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every continuation must leave the next GPT an accurate current SHA, exact verification boundary, first actionable task, and test/CI state. Do not churn documentation when the existing handoff remains accurate.

# Cross-session execution mandate

Continue the same development effort across conversations. Use one atomic main commit per logical source task, with complete Git trees and a non-forced compare-and-swap ref update. Before context exhaustion, close the current safe slice, record the actual commit SHA and exact Actions state, and leave the next GPT a concrete first task. Do not ask the user what to do next or wait on pending workflows when independent work can proceed.

## End-of-session accuracy checkpoint

- Current implementation checkpoint: THIS COMMIT; parent main SHA `aa163f6cc5993029684fd55a489528f3133fbf26`.
- The console-shell retry test compiler issue was fixed and passed on parent SHA `aa163f6cc5993029684fd55a489528f3133fbf26` (PID1 mdev run `45087866598`); this commit adds shutdown-aware retry cancellation and restart gating.
- New exact-head PID1/XY-C1 Actions must be checked after the push. No local tests/compile were run; current MIPS Build/TRX/artifact and physical hardware behavior are NOT VERIFIED.
- Parent-SHA PASS results never validate descendant commits.
- Current implementation SHA: `36bd2b1e01c0472b7d8fdc0a31e4a56e3b6762de` (rapid recovery console-shell respawn throttle).
- Exact-head AI docs, HTTPD, PID1 notification queue, DNS Forwarder, DNSMasq and Process inventory Actions passed on that SHA. PID1 mdev failed only at the added console regression compile command (extra backslashes); exact-head correction pending. XY-C1, Process umbrella, NVRAM and USB storage were in progress at last poll.
- The new PID1 signal-state and console retry regression code is present, but the console policy test did not compile in that run. Existing PID1 signal policy regression did compile/pass.
- Not verified: local checkout/build; current-head MIPS firmware build, TRX, artifact, XY-C1 hardware boot or physical router behavior.
- Parent-SHA PASS results never validate descendant commits.
