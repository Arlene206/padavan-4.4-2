# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- Create all changed blobs, one complete tree and one commit with the current main parent; update main with non-forced compare-and-swap using that exact SHA.
- Verify Actions by exact `head_sha`. A pending run does not block independent work; a failed relevant run must be triaged before claiming validation.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter or delete it. Develop on main.

## Current repository state

- Current main before this source slice: `437c7b289b77cf601544715525a1488dcf14575e` (`fix(rc): bound repeated service wait interruptions`), parent `16b755661ec1a33cd255dec10d4bac389816a8b3`.
- The prior procfs bounded-read source SHA `53dda2c844e18cfb879abc3bf4b96bb50d46fd2d` passed Process inventory `44632416173` and NVRAM `44632416256`; its Process umbrella failure `44632416363` was push/PR path parity and was fixed on `16b7556`, whose Process umbrella `44632969368` and AI docs `44632968248` both PASS.
- XY-C1 `44632416166` for SHA `53dda2c` was IN_PROGRESS at last check. Latest main `437c7b2` Actions are pending; first inspect Process and service-wait regression runs and XY-C1 `44633888536`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains archival at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 is closed/unmerged. No physical-router result is available.

## Active task

- The current source baseline bounds service wait EINTR loops in seconds and timespec paths. This follow-up is an independent shared process-I/O hardening slice: PID-file reads/writes, inline PID start-time reads and the legacy procps compatibility reader still had unbounded EINTR loops.
- Extended `process_io_retry_policy.h` with injectable bounded read/write wrappers and production `process_io_read()/process_io_write()`. Routed `process_pidfile_read()`, `process_pidfile_write_all()`, `process_pidfile_record_read_start_time()` and `pids.c:read_to_buf()` through these wrappers. Persistent interruption now returns `EINTR`; non-EINTR errors remain unchanged, positive short writes still advance, and EOF semantics are preserved.
- Added GNU89 callback regressions in `tests/process_pidfile.c` for read/write recovery, exhaustion, EOF, EIO preservation and invalid callbacks. Existing Process regression workflow compiles this test and already tracks the changed source/test paths.
- No local compilation, target MIPS result, or physical-router behavior is claimed.

## Exact first action for the next GPT

1. Fetch live `main` and check exact-head Process inventory/lifecycle, Process inventory and XY-C1 runs for this source commit. Inspect causal job logs for failures.
2. XY-C1 L3 requires Build XY-C1, TRX verification and artifact upload to all PASS on the same source SHA. Pending runs do not block independent work.
3. Continue another independently evidenced cross-userland reliability/security issue. Avoid repeating completed service-wait, service-lock and procfs-read audits without new evidence.

## Verification boundary

- L1: shared procfs reads are bounded in source SHA `53dda2c`; this slice adds eight-consecutive-EINTR retry caps to PID-file read/write, inline proc-stat and legacy procps read paths.
- L2: callback-level GNU89 read/write retry regressions are added to the existing `process_pidfile.c` test target; exact-head Actions for this slice are pending.
- L3: prior XY-C1 SHA `53dda2c` run `44632416166` was still pending; current source needs a same-SHA Build/TRX/artifact pass before L3 can be claimed.
- L4: MIPS/uClibc signal-storm behavior and physical-router service transitions remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, product-source baseline, exact-head Actions, module state and this first action. Update this file only when resumable facts materially change; do not create no-op SHA-refresh commits.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken regressions.
- Report exact SHA Actions and keep L1/L2/L3/L4 distinct. Host success is not firmware/hardware success.
