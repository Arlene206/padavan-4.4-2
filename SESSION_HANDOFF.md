# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Restore exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits or stop at planning.

- Create all changed blobs, one complete Git tree and one commit with current main as parent; update main using non-forced compare-and-swap with that exact expected SHA.
- Verify Actions by exact `head_sha`. Pending runs do not block independent work; inspect and repair real failures.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded I/O and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter it. Develop on main.

## Current repository state

- Main baseline before the secure account-provisioning slice: `ae7874833c6cb870162222f415124d1db4e63dda`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; archival and untouched.
- Wireless bridge config result propagation is committed. Process `44649240357` and AI handoff docs `44649240209` PASS on `ae7874833c6cb870162222f415124d1db4e63dda`. Production source `9f565b124635a0c3d0686636b4dc0313a5f3a298` passed XY-C1 run `44648971016`, including Build, TRX verification and artifact upload (L3 for that exact SHA).
- Physical-router L4 is NOT VERIFIED.

## Active task

- P1 RC startup-result propagation: all six ignored `gen_ralink_config_2g/5g()` results inside `init_bridge()` now feed `rc_bridge_init_status_t` and PID 1 DEGRADED readiness.
- Confirmed security defect: privileged password update used predictable `/tmp/tmpchpw` with `fopen("w")`; a pre-created symlink under world-writable `/tmp` could redirect the root credential write. The helper, `recreate_passwd_unix()` and `init_router()` also discarded provisioning errors.
- This slice uses exclusive mode-0600 `mkstemp()` staging, validates username/password fields, stages passwd/group/shadow file contents before rename, keeps root locked until `chpasswd` succeeds, and propagates account bootstrap status into PID 1.
- Main parent for this slice: `ae7874833c6cb870162222f415124d1db4e63dda`. The frozen backup remains untouched.

## Exact first action for the next GPT

1. Query exact-head Process lifecycle, AI handoff docs and XY-C1 validation for the secure account-provisioning commit. Inspect source contract, temp-file policy regression, and Build/TRX/artifact steps individually.
2. Repair any demonstrated exact-SHA failure without weakening private-file mode, exclusive creation, credential field validation, root lock-on-failure, or PID 1 error propagation.
3. Review paired passwd/group replacement rollback under an injected second-rename failure; verify the old live files can be restored and no staging files leak.

## Active task

- Latest parent at task start: `ae7874833c6cb870162222f415124d1db4e63dda`. Main-only; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged.
- Completed preceding wireless slice: Process `44649240357` and AI docs `44649240209` PASS on `ae787483`; production-source XY-C1 `44648971016` PASS on `9f565b1`, including Build, TRX and artifact upload (L3 for that SHA).
- This source/test slice removes the predictable password-staging symlink vulnerability and propagates account database/password provisioning failure into PID 1. Root remains locked until `chpasswd` succeeds.
- Paired passwd/group rename rollback is the next explicit fault-injection candidate. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Check exact-head Process lifecycle, AI docs and XY-C1 Actions for current main. Inspect Build XY-C1, TRX validation and artifact upload individually.
2. Continue with injected-failure coverage for paired passwd/group rename rollback, then resume independent P1 stability/security defects.
3. Keep L1/L2/L3/L4 evidence separate; successful cross-build does not establish physical-router behavior.

## Verification boundary

- L1: PID 1 distinguishes FATAL/READY/DEGRADED and continues event processing after degraded service boot. Late CA preparation, persistent user-script and autostart launcher failures are now included in the boot aggregate.
- L2: PID 1 policy binary and `dev_init` shell regression passed on `9ad1570`; the new CA status shell regression passed on `61cbc6`. Exact-head Process, AliDDNS and Storage jobs initially failed only at three stale source-contract assertions; their corrections are in this checker follow-up. HTTPD, AI docs and all other completed current-head regression runs were PASS.
- L3: Production source `9f565b124635a0c3d0686636b4dc0313a5f3a298` passed XY-C1 `44648971016`, including input detection, Build XY-C1, TRX verification and artifact upload. Evidence applies only to that SHA; physical-router L4 remains NOT VERIFIED.
- L4: target MIPS/uClibc runtime and physical-router boot/recovery behavior remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, production-source baseline, exact-head Actions, module state and the next task. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit; do not create a no-op commit.

## Before context exhaustion

- Finish the current safe source/test slice and commit it atomically before handoff; do not publish related multi-file changes as sequential partial commits.
- Re-fetch live `main`, create one complete Git tree and one commit with that exact parent, then update main only with `force=false` and the exact parent as `expected_sha`. Never force-push.
- Query relevant Actions by exact source SHA and record PASS/FAIL/PENDING with run IDs. Pending validation does not block independent work and must not be reported as complete.
- Record the production-source baseline, test commands/results, exact CI state, unresolved limitations and the next concrete task. Distinguish host L2, firmware L3 and device L4 evidence.
- Confirm all work is committed and remotely present before claiming it is preserved across conversations.

## Development invariants

- Main-only; never alter the frozen backup or develop on a legacy PR.
- One complete tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Keep L1/L2/L3/L4 distinct. Host tests are not firmware or hardware validation.
