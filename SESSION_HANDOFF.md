# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then execute the next verified engineering task; do not repeat completed audits or spend sessions on planning alone.

- one atomic main commit per logical slice. Create all changed blobs and one complete tree based on live main, parent the commit to the live SHA, and update with force=false plus exact expected_sha.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Match Actions to exact head_sha. Diagnose failures from causal logs. A pending run is not a stop condition; work on independent tasks.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, ABI/config compatibility and meaningful regressions.
- Before context exhaustion, finish the current safe slice, verify the exact SHA and leave one precise next action.

## Current repository state

- Production source baseline: `10b57237ced5487a0d17e0bb09425ea96973a78e`. Shared DHCP policy attempts all candidate writes and restores the full previous tuple; WAN DHCP renewal and PPP ip-up use it under NVRAM transactions.
- The LAN deconfiguration checker now validates full-vector rollback. Its first repair referenced an undefined variable for the new test-name check; this commit removes that redundant assertion. The WAN DHCP workflow itself checks the test source and invocation.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only; branch inventory last verified only main and this backup.
- Legacy PR #1 is CLOSED / NOT MERGED; AliDDNS HMAC/CA-bootstrap work is already present and evolved on main. Do not repeat that port audit.

## Active task

1. Poll exact-SHA WAN DHCP callback, WAN route, XY-C1, AI docs and NVRAM-boundary workflows for the validator repair.
2. Inspect failed logs and fix genuine errors without weakening attempt-all, full rollback, checked-write or WAN route-gating assertions.
3. Verify the attempt-all injected GNU89 regression and WAN renewal/PPP transaction source contracts. The LAN checker only verifies the policy loops; the WAN DHCP workflow verifies the dedicated test.
4. After this slice is green, audit `udhcpc_bound()` for unchecked/partial address-change lease tuple writes, add a focused regression, and commit the production change atomically.
5. Claim L3 only after same-production-SHA detector, MIPS build, TRX verification and artifact upload all pass; physical L4 requires device evidence.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live main before ending. Update this handoff only when resumable facts or the next task materially change. Otherwise make no no-op documentation commit.

## Evidence discipline

Keep host/source regressions, exact-head Actions, MIPS build/TRX and physical-device evidence separate. No local checkout is available here; local GNU89 tests were NOT RUN. L3 and L4 remain NOT VERIFIED until their required independent evidence is recorded.
