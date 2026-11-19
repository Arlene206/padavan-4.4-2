# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- Create all changed blobs, one complete tree and one commit with the current main parent; update main with non-forced compare-and-swap using that exact SHA.
- Verify Actions by exact `head_sha`. A pending run does not block independent work; a failed relevant run must be triaged before claiming validation.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter or delete it. Develop on main.

## Current repository state

- Current parent for this checker repair: `d7e61e0bc0cfaf2f6680763a29ed5d321d185a71` (`fix(rstats): bound interrupted persistence I/O`).
- Exact d7 SHA: Process inventory `44636788355` PASS; RStats `44636788530` PASS; AI docs `44636788278` PASS; HTTPD security `44636788170` PASS. NVRAM `44636788042` and XY-C1 `44636788140` were IN_PROGRESS at last poll.
- Process umbrella `44636788168` failed only at the embedded MTD source-contract negative check: it searched for substring `read(fd, ...)`, which also matched inside the intended wrapper call `process_io_read(fd, ...)`. This follow-up narrows the assertion to the full raw syscall assignment. Production MTD code is unchanged.
- RStats persistence now bounds EINTR on open/read/write/fsync while preserving last-good atomic publication; its exact regression passed on d7.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains archival at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 closed/unmerged. No physical-router result.

## Active task

- Checker-only repair: the Process workflow used a substring negative assertion `"read(fd...)" not in read_body`, which incorrectly matched the wrapper name `process_io_read(fd...)`. It now checks the complete raw syscall assignment `ssize_t ret = read(...)`; same correction for write. Positive assertions still require both bounded wrappers.
- No production source behavior changes in this commit.

## Exact first action for the next GPT

1. Verify Process umbrella rerun and AI-doc/NVRAM/XY-C1 exact-head outcomes after this checker correction.
2. On any new source/build failure, inspect the exact run/job log; preserve bounded I/O retry behavior, positive short-I/O progress, EOF semantics and atomic last-good publication. XY-C1 L3 requires Build, TRX verification and artifact upload PASS on the same SHA.
3. Continue the next independently evidenced cross-userland reliability/security issue; avoid repeating closed lock/wait/procfs/PID-file/MTD and RStats fixes without new evidence.

## Verification boundary

- d7 Process umbrella failed only due false-positive raw-syscall substring matching; corrected by checking the full assignment form. Process inventory, RStats, AI docs and HTTPD Actions PASS.
- d7 NVRAM and XY-C1 were pending; exact XY-C1 Build/TRX/artifact not yet confirmed.
- L4 target MIPS/uClibc and physical-router behavior remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, product-source baseline, exact-head Actions, module state and this first action. Update this file only when resumable facts materially change; do not create no-op SHA-refresh commits.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken regressions.
- Report exact SHA Actions and keep L1/L2/L3/L4 distinct. Host success is not firmware/hardware success.
