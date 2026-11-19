# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- Create all changed blobs, one complete tree and one commit with the current main parent; update main with non-forced compare-and-swap using that exact SHA.
- Verify Actions by exact `head_sha`. A pending run does not block independent work; a failed relevant run must be triaged before claiming validation.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter or delete it. Develop on main.

## Current repository state

- Current main before this source slice: `5c557db717ec173ef151fb6e473e44a676924234` (`fix(shared): bound MTD flash I/O EINTR retries`). The MTD production change bounds flash partition read/write retries and updated Process/XY-C1 path triggers; exact-head CI is in progress.
- The preceding exact parent `342631b4efb09b4877dd944d74f03d20da7b4164` passed Process inventory `44635384252`, Firewall Startup `44635384398`, FULLCONENAT `44635384256`, WAN route `44635384253`, NVRAM `44635384340`, and AI docs `44635384260`. Its HTTPD `44635384245` and WAN DHCP `44635384375` failed only in embedded source-contract assertions; logs confirmed no C compile or policy-binary failure. The current parent commit adds those checker corrections.
- RStats persistence still contains unbounded EINTR loops in exact-read, atomic-write, fsync and history-file open; this slice adds a shared bounded policy and deterministic regression tests. Frozen backup remains archival and untouched.

## Active task

- RStats persistence previously retried `EINTR` indefinitely in `rstats_file_read_exact_with()`, `rstats_file_write_atomic_with_ops()`, `rstats_file_fsync_retry()` and the `f_read()` open path. Repeated signals could stall the stats daemon while loading or publishing history.
- Added an eight-consecutive-interruption ceiling and callback-based bounded `open(O_RDONLY)`. Positive read/write progress resets the retry streak; EOF, partial-read and non-EINTR error semantics remain unchanged. Atomic publication retains the prior target if retries exhaust.
- Added GNU89 callback coverage for open/read/write/fsync recovery, retry exhaustion, progress reset, EOF and EIO propagation. RStats workflow assertions now require the bounded helper in production callsites. This source slice awaits exact-head CI and firmware verification.

## Exact first action for the next GPT

1. Inspect exact-head RStats workflow, Process umbrella and AI docs for this source commit; read and correct any causal job failure without weakening the tests.
2. Check parent `5c557db717ec173ef151fb6e473e44a676924234` XY-C1 run `44636408751` through Build, TRX verification and artifact upload. This RStats production-source slice needs its own same-SHA XY-C1 Build/TRX/artifact result.
3. Resolve the separate HTTPD/WAN-DHCP source-contract failures inherited from `342631b` if they remain after the `8987975` checker repair, then continue the next independent stability/security fix.

## Verification boundary

- L1: RStats history file open, exact-read, atomic-write and fsync now stop after eight consecutive EINTR results. Successful read/write progress resets the interruption streak; fail-closed errors preserve EINTR and old published data.
- L2: callback tests cover open/read/write/fsync recovery, retry exhaustion, read/write progress resets, EOF and EIO preservation. Exact-head Actions pending for this source SHA.
- L3: parent `5c557db717ec173ef151fb6e473e44a676924234` XY-C1 run `44636408751` was pending at last check. The new RStats source change needs Build/TRX/artifact all PASS on its exact source SHA.
- L4: MIPS/uClibc signal storms, storage-fault behavior on router and physical-router results remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, product-source baseline, exact-head Actions, module state and this first action. Update this file only when resumable facts materially change; do not create no-op SHA-refresh commits.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken regressions.
- Report exact SHA Actions and keep L1/L2/L3/L4 distinct. Host success is not firmware/hardware success.
