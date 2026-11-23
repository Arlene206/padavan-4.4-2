# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Development branch: `main` only. Do not create feature branches or PRs; never force-push.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Preserve it permanently; do not modify, rename, delete or check it out.
- Current source checkpoint: THIS COMMIT (RC PID1 SIGCHLD wait-mask lost-wakeup correction); parent main SHA: `b58eed6ae04458d8647d606a6d509da62562d87a`. Fetch live `main` on takeover; do not chase a docs-only SHA.
- Continue implementation rather than repeating completed audits or asking what to do next.

## Active task

The actual router PID 1 is RC init, `trunk/user/rc/src/init.c` (the board BusyBox `CONFIG_INIT` is disabled). The current defect is a lost-wakeup race in the main event loop: the SIGCHLD handler can reap the recovery shell and publish `console_shell_reaped` after the loop checks its flags but just before `sigsuspend()`. Because SIGCHLD was not in the blocked wait mask, the handler could run before PID 1 entered `sigsuspend()`; PID 1 could then sleep indefinitely with the one-shot completion already set and fail to respawn its recovery console.

This commit:
- Adds SIGCHLD to the checked `wait_mask` before that mask is blocked. The original mask remains the one passed to `sigsuspend()`, so SIGCHLD is atomically unblocked as the sleep begins; a signal arriving in the check-to-suspend window stays pending and wakes the loop.
- Uses the existing `init_signal_set_add()` status/error path; a failure retains the existing recovery-polling fallback instead of using a partial/uninitialized mask.
- Adds a GNU89 injected set-add regression for SIGCHLD and a production source contract requiring it to be added before the wait mask is blocked.

Parent main `b58eed6ae04458d8647d606a6d509da62562d87a` contains the shutdown-aware console retry guards. The earlier exact-head PID1 run on `d26ca8f30da14e917bd4ab08f3a6c5f37923f78b` failed only because its source contract retained one obsolete assertion for the pre-guard condition; descendant `62193804176e35820f4b61c3053424c9475c095c` removed that stale assertion, and its exact-head PID1 run passed. The subsequent `b58eed6ae04458d8647d606a6d509da62562d87a` change independently hardens Optware package-client teardown; preserve it.

## Verification boundary

Relevant exact-head parent results:
- `d26ca8f30da14e917bd4ab08f3a6c5f37923f78b` PID1 mdev/security `45088770855`: FAIL only at the embedded Python source contract's obsolete `if(!noconsole&&!signal_state.console_shell_pid)` assertion; all GNU89/ASan/UBSan compile/run steps, including the fast-exit console test, passed.
- Workflow-only correction `62193804176e35820f4b61c3053424c9475c095c`: PID1 mdev `45088928932` PASS and AI handoff docs `45088928924` PASS. Its XY-C1 run `45088928941` was IN_PROGRESS at last poll.
- Latest parent `b58eed6ae04458d8647d606a6d509da62562d87a` Actions at last poll:
- AI handoff documentation contract `45089031261`: COMPLETED / SUCCESS; exact head `b58eed6ae04458d8647d606a6d509da62562d87a`.
- XY-C1 Firmware Validation `45089031256`: IN_PROGRESS; exact head `b58eed6ae04458d8647d606a6d509da62562d87a`.
- USB storage hotplug stability regression `45089031251`: IN_PROGRESS; exact head `b58eed6ae04458d8647d606a6d509da62562d87a`.
- Process inventory and lifecycle fail-closed regression `45089031222`: IN_PROGRESS; exact head `b58eed6ae04458d8647d606a6d509da62562d87a`.
- Process lifecycle inventory - b58eed6ae04458d8647d606a6d509da62562d87a `45089031300`: COMPLETED / SUCCESS; exact head `b58eed6ae04458d8647d606a6d509da62562d87a`.

These parent results do not validate THIS COMMIT. Inspect its own exact-head PID1/AI/XY-C1 push runs after publication. Local checkout/compile is unavailable in this connector-only environment. Do not infer Build/TRX/artifact or hardware success from host regressions. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and verify frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not touch it.
2. Inspect exact-head PID1 mdev/security for the current source SHA. Confirm the GNU89/ASan/UBSan signal policy regression and source contract requiring SIGCHLD in the wait mask before blocking pass.
3. Inspect exact-head XY-C1 validation on the same SHA. Verify Build XY-C1, TRX validation and artifact upload actually execute and pass; host-only validation is not L3.
4. If exact-head checks pass, continue the focused RC PID1 signal/lifecycle review. If a run fails, inspect its first causal step/log and repair that on main before declaring the slice verified.
5. Preserve atomic main commits: complete Git tree, one commit parented to freshly fetched main, then non-forced ref update with that exact expected SHA.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every continuation must leave the next GPT an accurate current SHA, exact verification boundary, first actionable task, and test/CI state. Do not churn documentation when the existing handoff remains accurate.

# Cross-session execution mandate

Continue the same development effort across conversations. Use one atomic main commit per logical source task, with complete Git trees and a non-forced compare-and-swap ref update. Before context exhaustion, close the current safe slice, record the actual commit SHA and exact Actions state, and leave the next GPT a concrete first task. Do not ask the user what to do next or wait on pending workflows when independent work can proceed.

## End-of-session accuracy checkpoint

- Current implementation checkpoint: THIS COMMIT; parent main SHA `b58eed6ae04458d8647d606a6d509da62562d87a`.
- PID1 now includes SIGCHLD in the main-loop wait mask before blocking it, closing the reaper completion check-to-sigsuspend lost-wakeup window. A GNU89 set-add regression and production source contract are included.
- Earlier parent PID1 mdev contract failure on `d26ca8f` was corrected by `6219380`; exact mdev run `45088928932` and AI docs run `45088928924` passed there. Parent `b58eed6ae04458d8647d606a6d509da62562d87a` runs are recorded above as of the last poll.
- New exact-head PID1/AI/XY-C1 Actions are required for THIS COMMIT. Local compile/test NOT RUN. Current MIPS Build/TRX/artifact and hardware L4 remain NOT VERIFIED.
- Parent-SHA PASS results never validate descendant commits.
