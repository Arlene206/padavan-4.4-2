# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into partial commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`. Current parent: `8b0ad1bb5760c282b1b85482be6b14ba0974720f`.
- `net6.c` per-interface IPv6 address/route/neighbour flushes and all `net_wan6.c` SIT/6rd tunnel lifecycle, MTU, IPv6 routes, native static routes and SIT teardown use argv-based `eval()`; `net_wan6.c` has no remaining `doSystem()` calls.
- SIT MTU/TTL are clamped and rendered into fixed-size checked decimal buffers before tunnel side effects. `WAN6_SIT_CHECK` / `WAN6_NATIVE_CHECK` preserve fail-fast status handling and cleanup ordering.
- Exact `8b0ad1bb5760c282b1b85482be6b14ba0974720f` WAN route run `43989785616` PASS. WAN DHCP run `43989785873` failed only in the final source-contract step: adjacent Python string literals without commas concatenated multiple expected argv markers, and a route-order assertion still searched for the removed shell command. All preceding compiled regression steps passed. XY-C1 run `43989785677` remained in progress at last poll; no L3 claim is made.
- This correction restores each marker as an independent tuple item, checks both SIT route wrappers, updates route-order assertions and adds cross-workflow source assertions for the shell-free boundary.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Active task

Correct the WAN IPv6 source-contract regression exposed by exact-head run `43989785873`: restore commas separating marker strings in the SIT/native operation tuples, update the route-order assertion to match argv-based route commands, and independently assert the shell-free argv contract in the shared modem/WAN checker. Then inspect exact-head WAN DHCP, WAN route, Process, Firewall and XY-C1 Actions. Do not repeat the completed `net6.c` or `net_wan6.c` source conversion.

## Verification boundary

- Current source conversion is present and has no remaining `doSystem()` calls in `net_wan6.c`. The WAN DHCP failure is in its Python source-contract step; all earlier compiled regression steps passed.
- The XY-C1 Build/TRX/artifact run on source SHA `8b0ad1bb5760c282b1b85482be6b14ba0974720f` was pending at last poll. Product-source MIPS/uClibc L3 and physical-router L4 remain **NOT VERIFIED**.

## Exact first action for the next GPT

1. Verify the corrected workflow and shared checker on the new exact main SHA; resolve any new first causal failure from its logs.
2. Continue the remaining WAN teardown/interface provenance audit across `net_wan.c` and DHCP/PPP callbacks, preserving route cleanup, callback fences and first-error semantics; continue independent work while XY-C1 Actions are pending.
3. Keep the backup branch frozen. Do not claim L3 until Build, TRX verification and artifact upload all pass on one exact SHA.
