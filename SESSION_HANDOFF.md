# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume the exact active task; avoid repeated audits, planning-only stops and status-only commits.

- Each logical task ends in one atomic main commit. For multi-file changes create all blobs, one complete Git tree and one commit parented to live `main`, then update `main` with `force=false` and exact `expected_sha`.
- Never create/use development branches or PRs, force-push, amend published commits, or modify the frozen backup.
- Check Actions by exact `head_sha`. Diagnose FAIL from causal logs; while checks are PENDING, continue independent work.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, process identity checks, bounded operations and meaningful regression coverage.
- Do not ask the user what to do next. Before context exhaustion, finish the current safe slice and leave an accurate next action.

## Current repository state

- Parent `main` before this slice: `e387b92c16c83c3621e78e16dcdbd10707d7a2d5`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and untouched.
- On exact parent `e387b92`, Process regression `44672214134` PASS (including TCP-tweak policy and PID 1 source contract), and AI handoff docs `44672214038` PASS. XY-C1 `44672214073` was still IN_PROGRESS at last poll; do not claim L3 for that parent.
- Previous TCP source commit `e387b92`: all nine `set_tcp_tweaks()` writes are now attempted, logged and aggregated into `init_sysctl()`.
- Historical PR #1 is CLOSED/UNMERGED. CA trust-store bootstrap and AliDDNS HMAC helper/tests/workflow integration are already on `main`.
- Physical-router L4 remains NOT VERIFIED.

## Active task completed in this slice

- Fixed the remaining checked-error gap in `set_libc_gai()`. It now returns file-operation status: IPv4-first publication uses the existing `fput_string()` path (including write/flush/close error detection); override removal reports failures but treats `ENOENT` as idempotent success.
- Updated the prototype and all known production callers. PID 1's non-IPv6 `init_sysctl()` path records the result; `net6.c::init_ipv6()` and both enable/disable paths of `full_restart_ipv6()` preserve first-error status while continuing independent operations.
- Added a GNU89 regression for write failure, remove failure, ENOENT idempotence and missing callbacks; extended the Process workflow and PID 1 semantic source contract. The policy regression compiled and passed locally with `-std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined`.

## Verification boundary

- Exact parent `e387b92`: Process `44672214134` PASS and AI docs `44672214038` PASS; XY-C1 `44672214073` was IN_PROGRESS at last poll. None of these runs validates the new `set_libc_gai()` change.
- The GAI source slice is not yet verified by exact-head Actions at this checkpoint. No local full firmware build or physical-router test is claimed. L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect Actions by exact `head_sha` for this GAI slice. Confirm Process, AI docs and the new GNU89 GAI policy step. Require same-SHA XY-C1 production-input detector, Build XY-C1, TRX verification and artifact upload before claiming L3.
2. Continue P1 RC startup-result propagation from the existing roadmap queue: inspect `net_wan.c::wan_up_internal()` after the Inadyn notification result fix, trace every remaining optional-launch/start-hook result and cleanup path, and fix the first concrete discarded failure with an injected regression and source-contract coverage. Do not redo the completed Inadyn fix or speculative void-wrapper audits.
3. Keep the process lifecycle inventory and all unrelated queued checks moving while any CI remains PENDING.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts, validation state, blocker or first action materially change. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

## Evidence discipline

Keep L1 source review, L2 host/Actions, L3 MIPS build/TRX and L4 physical-device evidence separate. A skipped build is not PASS, a successful cross-build does not establish router runtime behavior, and every claim must name the exact source SHA and run.
