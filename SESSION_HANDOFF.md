# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Restore exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits or stop at planning.

- Create all changed blobs, one complete Git tree and one commit with current main as parent; update main using non-forced compare-and-swap with that exact expected SHA.
- Verify Actions by exact `head_sha`. Pending runs do not block independent work; inspect and repair real failures.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded I/O and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter it. Develop on main.

## Current repository state

- Parent for this MTD correction: `dfe8eb6d95bd5297612c41e06412e17c1a9434fe` (`ci(process): correct MTD raw syscall assertions`).
- Parent exact-head Actions for dfe were Process fail-closed regression `44637246133` and AI docs `44637246124`, both IN_PROGRESS at last poll.
- Earlier d7 Actions: Process inventory `44636788355`, RStats persistence `44636788530`, HTTPD security `44636788170`, and AI docs `44636788278` PASS. Process fail-closed regression `44636788168` failed in the MTD source contract; the substring false-positive was corrected in dfe.
- The original MTD migration introduced a deeper issue: outer `if (errno == EINTR) continue;` branches in `read_safe()`/`write_safe()` restart the bounded wrapper and reset its interruption counter. This commit removes the unbounded restart and extends source assertions to pin the invariant.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 closed/unmerged. No physical-router result is available.

## Active task

- `process_io_read()` and `process_io_write()` cap consecutive EINTR attempts at eight, then return `-1/EINTR`.
- MTD `read_safe()` and `write_safe()` previously caught that terminal EINTR and looped again, resetting the wrapper's local retry counter. This defeated the bound under persistent signal storms.
- Current source change propagates terminal wrapper failures directly. Positive short reads/writes still advance the buffer; read EOF behavior and zero-progress write `EIO` remain unchanged.
- Process workflow source contract asserts bounded wrapper use, no direct syscalls, no outer EINTR/continue restart, and preservation of progress/EOF/zero-write semantics. Shared callback test coverage lives in `trunk/user/shared/src/tests/process_pidfile.c`.

## Exact first action for the next GPT

1. Check exact-head Process fail-closed regression and AI-doc Actions for this source correction. Read the exact job log if either fails.
2. Check NVRAM and XY-C1 statuses from d7/dfe; L3 requires Build, TRX verification and artifact upload all SUCCESS on the same current source SHA.
3. Continue with independently evidenced issues. A known remaining integration gap is miniupnpd's generic `is_valid_ipv4(lan_mask)` before `ip2class()` in `services_ex.c`; strict contiguous-mask validation currently guards firewall code only.

## Verification boundary

- L1: shared I/O wrapper's GNU89 regression covers eight recoverable EINTRs and terminal ninth EINTR. MTD terminal error now propagates instead of restarting.
- L2: d7 Process inventory, RStats, HTTPD and AI-doc Actions PASS; Process fail-closed regression failed checker substring and the dfe checker-only correction is pending validation. Current source correction awaits its own exact-head Process/AI/NVRAM runs.
- L3: d7 XY-C1 and NVRAM Actions were in progress; current source SHA needs exact-head Build/TRX/artifact verification.
- L4: physical XY-C1 MTD signal-storm behavior remains NOT VERIFIED.

## Mandatory handoff checkpoint

Every session MUST perform a handoff checkpoint before ending. Compare live main, product-source baseline, exact-head Actions, module state and next action. Update this file only when resumable facts materially change; do not create no-op SHA-refresh commits.

## Development invariants

- Main-only; never alter the frozen backup or develop on a legacy PR.
- One complete tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Keep L1/L2/L3/L4 distinct. Host tests are not firmware or hardware validation.
