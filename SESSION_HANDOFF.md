# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeated audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into half-commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Parent for this conntrack-flush slice: `1f8299371114d2fb846ec1c47a0116afd584668c`. Strict HW NAT/FTP/SFE tri-state source changes are in `1554a5f3`; checker/handoff repairs are in `99d8562d` and `1f829937`.
- Exact-head `1f829937` Process `43971856210`, Firewall Startup `43971856255` and AI handoff `43971856365` passed. WAN/XY and other Actions were still running at last poll; inspect their exact-head status.
- The new source slice changes `flush_conntrack_table()` from `void` to errno-style `int`, returning shared `fput_string()` status. It covers all four callers: PID 1 netfilter notification, LAN teardown, WAN teardown and MAN DHCP deconfig. An injected regression verifies the write path and errno.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at its recorded SHA `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. Never modify it. Physical-router L4 remains **NOT VERIFIED**.

## Active task

Complete and verify conntrack flush failure propagation. Compile/run the injectable policy test with GNU89/ASan/UBSan, confirm all callers aggregate or return the status without short-circuiting later cleanup, then inspect exact-head WAN/Process/Firewall/XY/AI Actions.

## Verification boundary

- `fput_string()` returns errno-style status from its checked stream write/close path; the previous `flush_conntrack_table()` discarded it.
- Previous exact `1f829937` Process, Firewall Startup and AI handoff passed but do not cover this new conntrack source change. XY-C1 was building at last poll; no current-source L3 claim.
- The new exact-head C regression and source-contract run have not yet executed. Physical-router L4 remains **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch live `main`; inspect exact-head WAN route, Process lifecycle, Firewall Startup, XY-C1 and AI handoff runs for the conntrack change. Fix real failures from their job logs and confirm same-SHA firmware Build/TRX/artifact.
2. Continue auditing RC/network result-value loss only where an actual operational error is discarded.
3. Keep one complete-tree atomic main commit per logical slice with non-forced expected-SHA CAS; keep the backup read-only and do not claim physical-router L4 without hardware evidence.
