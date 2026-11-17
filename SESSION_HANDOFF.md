# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into partial commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`. Parent for this WAN IPv6 argv slice: `05cb9ad2b672feb5ec33b62933878d8692cbb1d0`.
- `net6.c` per-interface IPv6 address/route/neighbour flushes and all `net_wan6.c` SIT/6rd tunnel lifecycle, MTU, IPv6 routes, native static routes and SIT teardown now use argv-based `eval()`; there are no remaining `doSystem()` calls in `net_wan6.c`.
- SIT MTU/TTL are clamped first and rendered into fixed-size checked decimal buffers before side effects. `WAN6_SIT_CHECK` / `WAN6_NATIVE_CHECK` continue wrapping forward setup operations; cleanup ordering/status propagation is retained.
- Exact `05cb9ad2b672feb5ec33b62933878d8692cbb1d0` Actions: WAN route `43988195896` PASS; WAN DHCP callback `43988195967` PASS; all nine discovered push workflows SUCCESS. The new source/workflow slice has focused local source contracts passing and requires fresh exact-head Actions.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Active task

Verify exact-head WAN DHCP callback, WAN route, Process, Firewall and XY-C1 workflows for the current commit; inspect any failure logs and resolve the earliest causal issue. If independent runs remain pending, continue the broader WAN teardown/route and interface provenance audit without waiting. Do not repeat the completed argv conversions in `net6.c` or `net_wan6.c`.

## Verification boundary

- Local source checks confirm all SIT/native command conversions, bounded MTU/TTL formatting and preservation of fail-fast macro wrappers. Full target compilation has not been performed locally.
- Exact-head push Actions for this source commit have not yet been observed. Product-source MIPS/uClibc L3 (XY-C1 Build, TRX verification and artifact upload on one SHA) and physical-router L4 remain **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch `/actions/runs?head_sha=<main SHA>` and inspect exact-head WAN DHCP callback, WAN route, Process, Firewall and XY-C1. Fix only the first causal failure and then re-poll the independent runs.
2. Continue the remaining WAN IPv6 teardown/interface provenance audit across `net_wan.c`, `net_wan6.c` and DHCP/PPP callbacks; preserve route cleanup, callback fences and first-error semantics.
3. Keep the backup branch frozen. Do not claim product-source L3 until XY-C1 Build, TRX verification and artifact upload pass on one exact SHA.
