# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeated audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into half-commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`. Parent for this WAN DHCP checker repair: `44fd9bedf4b7b1ab4e9d4408a6157681158774d3`. Conntrack production implementation is `cffc1497fda9f65d9a0b163bad5acc1723c23f81`.
- Exact-head WAN route `43974652676`, Process inventory `43974652670`, AI handoff `43974652705`, NVRAM audit `43974652715`, AdGuardHome `43974653150`, AliDDNS `43974652681`, AliyunDrive `43974652664`, and infosvr `43974652631` all passed on `44fd9bed`.
- The cffc WAN DHCP callback run `43973861977` compiled and passed its injected C tests, then failed at its final source assertion because it still expected the pre-conntrack two-status return from `udhcpc_deconfig()`. Production now correctly aggregates interface, conntrack and teardown statuses. This checker-only commit updates the contract to require the conntrack writer result, ensure the write precedes cleanup, and verify the complete nested first-error return. Exact-head rerun is pending.
- Conntrack implementation `cffc149` changes `flush_conntrack_table()` to return the errno-style `fput_string()` result and propagates it through PID 1, LAN, WAN and MAN teardown while continuing independent cleanup. Its focused GNU89/ASan/UBSan injected policy regression passed.
- XY-C1 run `43973861873` for exact source SHA `cffc149` was still building at last poll; Build XY-C1 was in progress, TRX verification and artifact upload remained pending. This is the production-source SHA; later commits `c3bcb` and `44fd` only adjust test/doc contracts. Current product L3 remains **NOT VERIFIED** until that build, TRX check and artifact upload all pass.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Physical-router L4 remains **NOT VERIFIED**.

## Active task

Validate the WAN DHCP callback integration contract after updating its stale legacy assertion. Inspect the new exact-head WAN DHCP Actions job; if it fails, use the first causal step/log and fix the actual mismatch without weakening production behavior. Continue checking the same-source XY-C1 Build/TRX/artifact result.

## Verification boundary

- Exact `44fd9bed` WAN route, Process, AI handoff, NVRAM and other listed workflows passed.
- Exact cffc WAN DHCP callback run failed only at the old `udhcpc_deconfig()` return assertion after all C policy tests passed. The current commit changes only that workflow contract and handoff metadata; rerun is required.
- Exact cffc XY-C1 Build was pending at last poll; do not claim L3 until TRX verification and artifact upload also pass. Physical-router L4 remains **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch live `main`; inspect exact-head WAN DHCP callback result and XY-C1 run `43973861873`. Diagnose any first causal failure and confirm Build, TRX and uploaded artifact.
2. Once this slice is green, inspect `flush_route_caches()` and every caller. Trace the existing return contract before changing it; retain route cleanup ordering and error aggregation.
3. Keep each logical task in one complete-tree atomic commit with a non-forced expected-SHA main update; do not touch the frozen backup or claim hardware L4 without device evidence.
