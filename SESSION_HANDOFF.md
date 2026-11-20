# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume the exact active task; avoid repeated audits, planning-only stops and status-only commits.

- Each logical task ends in one atomic main commit. For multi-file changes create all blobs, one complete Git tree and one commit parented to live `main`, then update `main` with `force=false` and exact `expected_sha`.
- Never create/use development branches or PRs, force-push, amend published commits, or modify the frozen backup.
- Check Actions by exact `head_sha`. Diagnose FAIL from causal logs; while checks are PENDING, continue independent work.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, process identity checks, bounded operations and meaningful regression coverage.
- Do not ask the user what to do next. Before context exhaustion, finish the current safe slice and leave an accurate next action.

## Current repository state

- Parent `main` before this slice: `a2ad2d27c24e5e060b28c6d8954f7e2674cf2429`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and untouched.
- All 11 push Actions on exact parent `a2ad2d27` passed. XY-C1 run `44669423813` completed the production-input detector, Build XY-C1, TRX verification and artifact upload successfully. This L3 evidence applies to the parent SHA only.
- Historical PR #1 is CLOSED/UNMERGED. Its CA trust-store bootstrap and AliDDNS HMAC helper/tests/workflow integration are already present on current `main`; no remaining core PR implementation needs porting.
- Physical-router L4 remains NOT VERIFIED.

## Active task completed in this slice

- Fixed PID 1 early-boot TCP sysctl status loss. `net.c::set_tcp_tweaks()` previously discarded all nine `fput_int()` results. It now runs a testable nine-setting operation table, attempts every independent write, logs each failed sysctl, and returns the first error.
- `init.c::init_sysctl()` records that returned status through the existing first-error accumulator, so incomplete TCP tuning degrades READY startup while preserving FATAL prerequisite behavior and continuing other independent sysctl operations.
- Added a GNU89 policy regression for the complete setting sequence, all-write continuation, first-error preservation and missing callback; the Process lifecycle workflow compiles/runs it with `-Wall -Wextra -Werror`, ASan and UBSan. The PID 1 source checker now verifies the production wiring and push/pull_request workflow coverage.

## Verification boundary

- Parent `a2ad2d27`: Process fail-closed regression `44669423805` PASS; Process inventory `44669423849` PASS; AI handoff docs `44669423800` PASS; XY-C1 Build/TRX/artifact run `44669423813` PASS. These checks validate the parent only, not this new TCP slice.
- The new slice has not yet been validated by exact-head Actions at this checkpoint. No local checkout/build was available; do not claim local test execution. L3 is proven for the parent SHA only. L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect push Actions by exact `head_sha` for this TCP sysctl commit. Diagnose any FAIL from the first causal log; confirm the new TCP policy and PID 1 source contract passed.
2. Require same-SHA XY-C1 production-input detector, Build XY-C1, TRX verification and artifact upload before claiming current-source L3.
3. Continue the next identified PID 1/network sysctl defect: `net.c::set_libc_gai()` is still void and drops `fopen`, `fprintf`, `fclose` and `unlink` errors. Trace its callers before changing the signature: `init.c::init_sysctl()` (non-IPv6 build), `net6.c::init_ipv6()`, and both enable/disable branches of `net6.c::full_restart_ipv6()`. Convert it to return a stable status, treat ENOENT on disable as idempotent success, propagate status into existing first-error accumulators, and add regression/source/workflow coverage. Preserve best-effort startup and existing operation order.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts, validation state, blocker or first action materially change. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

## Evidence discipline

Keep L1 source review, L2 host/Actions, L3 MIPS build/TRX and L4 physical-device evidence separate. A skipped build is not PASS, a successful cross-build does not establish router runtime behavior, and every claim must name the exact source SHA and run.
