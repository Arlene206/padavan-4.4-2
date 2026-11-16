# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into partial commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`. Parent for the current route-cache result-propagation slice: `2c503c9ee1b3cda569700d3e85f614eed0317ad8`.
- Conntrack flush propagation is in `cffc1497fda9f65d9a0b163bad5acc1723c23f81`; exact-source XY-C1 run `43973861873` passed build, TRX verification and artifact upload. Product-source L3 is verified for cffc149.
- LAN router-mode teardown fix `759c18d860ba1c0cb547f43fa7ca8d21b6580987` captures/logs `clear_if_route4(IFNAME_BR)` and aggregates conntrack, route and bridge-down statuses while continuing cleanup. Its exact Process run passed.
- The corrected descendant `2c503c9` has exact WAN route `43977147385` PASS, Process `43977147335` PASS and AI handoff `43977147281` PASS. NVRAM run `43977147536` was pending at last poll.
- A new reliability defect was found: `flush_route_caches()` discarded the result of `system("ip route flush cache")`, and `full_restart_wan()` ignored its status before rebuilding switch/WAN state. This slice changes the wrapper to return `eval("ip", "route", "flush", "cache")`, updates the header prototype and aggregates global-cache status with all per-interface route flush results.
- The focused regression injects per-interface and global-cache callbacks, checks that cache cleanup still runs after a route failure, verifies first-error precedence and cache-only failure, and is compiled in WAN route CI with GNU89/ASan/UBSan. The WAN workflow source contract checks the production API, caller and restart gate.
- The exact 759c18 XY-C1 run `43976717898` was still in progress at the last poll; Build was running, TRX and artifact steps pending. The LAN and route-cache source changes need same-SHA product validation before L3 can be claimed.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; physical-router L4 remains **NOT VERIFIED**.

## Active task

Inspect exact-head WAN route, Process, AI handoff and XY-C1 Actions for the route-cache change. Resolve the first causal failure if any, and confirm Build XY-C1, TRX verification and artifact upload on the same source SHA.

## Verification boundary

- Parent `2c503c9`: WAN route `43977147385`, Process `43977147335` and AI handoff `43977147281` passed; NVRAM `43977147536` was pending.
- The route-cache source change and injected test have not yet run in CI on the new SHA. Same-SHA WAN route and Process checks are required; firmware L3 additionally requires Build, TRX verification and uploaded artifact.
- cffc149 product-source L3 is verified; later LAN and route-cache source changes require their own exact-SHA L3 evidence. Physical-router L4 remains **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch live `main`; inspect exact-head WAN route, Process, AI handoff, NVRAM and XY-C1 workflows for this route-cache commit. Read first causal logs before changing code.
2. Confirm both the 759c18 LAN teardown change and this route-cache change compile in the same exact-SHA XY-C1 build, with TRX and artifact success.
3. Continue the RC teardown audit for status-returning route/firewall/interface operations whose results are discarded; preserve attempt order and first-error semantics.
4. Keep one complete-tree atomic commit per logical change, use non-forced expected-SHA updates on `main`, and leave the frozen backup unchanged. Do not claim physical L4 without device evidence.
