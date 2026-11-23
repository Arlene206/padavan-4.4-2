# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Development branch: `main` only. Do not create feature branches or PRs; never force-push.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Preserve it permanently; do not modify, rename, delete or check it out.
- Current source checkpoint: THIS COMMIT (RC PID1 event/shutdown handler serialization); parent main SHA: `42fa5cfa6bc52db1b62434c1dc9d39947f4ae05f`. Fetch live `main` on takeover; do not chase a docs-only SHA.
- Continue implementation rather than repeating completed audits or asking what to do next.

## Active task

The actual router PID 1 is RC init, `trunk/user/rc/src/init.c` (the board BusyBox `CONFIG_INIT` is disabled). A remaining signal-priority race exists between `catch_sig_event()` and `catch_sig_shutdown()`: the event handler checked `shutdown_pending` then wrote a queued-event flag, but its `sa_mask` blocked only other event signals. A SIGTERM/SIGQUIT could interrupt between the check and write; the shutdown handler would set the higher-priority state and clear events, then the interrupted event handler could resume and republish SIGUSR1/SIGALRM work after shutdown had started.

This commit:
- Adds the full `signals_shutdown` group to the event handler's `sa_mask`, in addition to `signals_event`. Shutdown and lower-priority event state transitions can no longer interleave.
- Retains the existing shutdown handler mask, which already blocks both event and shutdown groups.
- Extends the exact PID1 workflow source contract to require both groups in the event-handler mask. The existing GNU89 state regression already asserts events are discarded after shutdown is published.

Parent main `42fa5cfa6bc52db1b62434c1dc9d39947f4ae05f` adds SIGCHLD to the main-loop wait mask, closing the recovery-shell reaper check-to-suspend lost-wakeup. Its exact-head PID1 policy tests and handoff-doc checks passed; XY-C1 was still building at the last poll.

## Verification boundary

Parent source SHA `42fa5cfa6bc52db1b62434c1dc9d39947f4ae05f` exact push results at last poll:
- XY-C1 Firmware Validation `45089123426`: IN_PROGRESS; exact head `42fa5cfa6bc52db1b62434c1dc9d39947f4ae05f`.
- NVRAM transaction boundary audit `45089123399`: IN_PROGRESS; exact head `42fa5cfa6bc52db1b62434c1dc9d39947f4ae05f`.
- PID 1 notification queue stability regression `45089123511`: COMPLETED / SUCCESS; exact head `42fa5cfa6bc52db1b62434c1dc9d39947f4ae05f`.
- Process inventory and lifecycle fail-closed regression `45089123559`: COMPLETED / SUCCESS; exact head `42fa5cfa6bc52db1b62434c1dc9d39947f4ae05f`.
- DNSMasq lifecycle stability regression `45089123342`: COMPLETED / SUCCESS; exact head `42fa5cfa6bc52db1b62434c1dc9d39947f4ae05f`.
- USB storage hotplug stability regression `45089123469`: COMPLETED / SUCCESS; exact head `42fa5cfa6bc52db1b62434c1dc9d39947f4ae05f`.
- DNS Forwarder launch path stability regression `45089123505`: COMPLETED / SUCCESS; exact head `42fa5cfa6bc52db1b62434c1dc9d39947f4ae05f`.
- PID 1 mdev hotplug configuration security regression `45089123591`: COMPLETED / SUCCESS; exact head `42fa5cfa6bc52db1b62434c1dc9d39947f4ae05f`.
- AI handoff documentation contract `45089123398`: COMPLETED / SUCCESS; exact head `42fa5cfa6bc52db1b62434c1dc9d39947f4ae05f`.
- Process lifecycle inventory - 42fa5cfa6bc52db1b62434c1dc9d39947f4ae05f `45089123554`: COMPLETED / SUCCESS; exact head `42fa5cfa6bc52db1b62434c1dc9d39947f4ae05f`.
- HTTPD Authentication and Security Regression `45089123392`: COMPLETED / SUCCESS; exact head `42fa5cfa6bc52db1b62434c1dc9d39947f4ae05f`.
- PID1 mdev/security `45089123591`: PASS, including all GNU89/ASan/UBSan PID1 policy tests and embedded source contracts.
- AI handoff docs `45089123398`: PASS.
- XY-C1 `45089123426`: IN_PROGRESS during cross-build; its final Build/TRX/artifact outcome must be fetched from that exact SHA.

These parent results do not validate THIS COMMIT. Inspect its exact-head PID1/AI/XY-C1 push runs after publication. Local checkout/compile is unavailable in this connector-only environment. Do not infer MIPS Build/TRX/artifact or hardware success from host regressions. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and verify frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not touch it.
2. Inspect exact-head PID1 mdev/security on the current source SHA. Confirm the GNU89/ASan/UBSan signal policy suite and the source contract requiring both event and shutdown signal groups in the event-handler mask pass.
3. Inspect exact-head XY-C1 validation for this same SHA. Verify Build XY-C1, TRX validation and artifact upload actually execute and pass; host-only validation is not L3.
4. If exact-head checks pass, continue the focused PID1 signal/lifecycle review. If any run fails, inspect its first causal step/log and repair that on main before claiming the slice verified.
5. Keep atomic main commits with a complete tree, fresh parent, `force:false` and exact `expected_sha`.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every continuation must leave the next GPT an accurate current SHA, exact verification boundary, first actionable task, and test/CI state. Do not churn documentation when the existing handoff remains accurate.

# Cross-session execution mandate

Continue the same development effort across conversations. Use one atomic main commit per logical source task, with complete Git trees and a non-forced compare-and-swap ref update. Before context exhaustion, close the current safe slice, record the actual commit SHA and exact Actions state, and leave the next GPT a concrete first task. Do not ask the user what to do next or wait on pending workflows when independent work can proceed.

## End-of-session accuracy checkpoint

- Current implementation checkpoint: THIS COMMIT; parent main SHA `42fa5cfa6bc52db1b62434c1dc9d39947f4ae05f`.
- PID1 event-handler masks now block both event and shutdown groups, preventing a higher-priority shutdown from being interrupted by an event handler that republishes deferred work.
- Parent `42fa5cfa6bc52db1b62434c1dc9d39947f4ae05f` PID1 mdev `45089123591` and AI docs `45089123398` passed. XY-C1 `45089123426` was cross-building at last poll; verify its final result.
- New exact-head PID1/AI/XY-C1 Actions are required for THIS COMMIT. Local compile/test NOT RUN; current MIPS Build/TRX/artifact and hardware L4 remain NOT VERIFIED.
- Parent-SHA PASS results never validate descendant commits.
