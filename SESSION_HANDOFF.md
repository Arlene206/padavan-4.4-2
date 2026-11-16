# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Parent main before THIS COMMIT: `6e4511c0607b892bdc101f38f5bbf7c40b86fa49`. Exact-head Process integration run `43923656497` and AI handoff documentation run `43923656691` both passed on this exact SHA as `push` events.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- XY-C1 firmware run `43922140859` passed Build XY-C1, TRX verification and artifact upload on product-source SHA `f5536dd46d24c8d0143c94a81a24a78a4d3cb15f`. It predates THIS COMMIT's UPnP status propagation change and is not validation for it.
- WAN teardown run `43922685825` passed on `a8acbb08d8cf2d8c80713ac2f670ef8ca9dd20e5`; USB modem identity run `43921625498` passed on `b61ca225bc996bbace4000a7aa5aa6972f9cf5db`.
- PR #1 is closed/unmerged historical work. Do not use a PR or non-main branch for development.

## Active task

THIS COMMIT fixes a static-IPoE startup result gap in `trunk/user/rc/src/net_wan.c`: `start_wan()` invoked the int-returning `update_upnp()` after static WAN route setup, but only logged its failure. The failure is now merged into `wan_start_status` through `rc_first_service_start_status()`, while preserving first-error semantics and continuing independent startup work. Added a GNU89 policy regression for this failure/first-error behavior and a semantic source contract that checks production call, aggregation and diagnostic order.

## Verification boundary

- Local GNU89/Werror/ASan/UBSan smoke test of first-error aggregation passed. The full repository policy test and canonical checker were not run locally because this runtime could not resolve GitHub's raw-content host.
- Inspect exact-head `push` Actions for THIS COMMIT: Process regression compiles the full policy test and runs `scripts/check_modem_wan_start_status.py`; XY-C1 runs firmware Build, TRX verification and artifact upload; AI docs validates the handoff and roadmap. No ancestor SHA result counts for this source change.
- Existing L3 evidence applies to product-source SHA `f5536dd46d24c8d0143c94a81a24a78a4d3cb15f` only. Do not claim current L3 until THIS COMMIT's same-SHA Build, TRX verification and artifact upload pass.
- Physical-router L4 remains NOT VERIFIED. Frozen backup remains unchanged.

## Exact first action for the next GPT

1. Inspect THIS COMMIT's exact-`head_sha` Process, XY-C1 and AI docs runs. If a run fails, inspect the first causal step and logs, then repair with a new atomic main commit. If pending, continue independent work rather than waiting.
2. Confirm the UPnP reload regression and source checker pass, including retention of an earlier WAN error.
3. Continue the P1 WAN/RC call-site audit for int-returning operations whose errors are only logged or discarded; prioritize required route, interface and configured-service state transitions.
4. Keep the backup branch read-only and physical-router L4 marked NOT VERIFIED.

