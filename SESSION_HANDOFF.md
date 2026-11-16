# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Parent main before THIS COMMIT: `1b06d3178c366ae764f894d6bb15847b8f5f6e21`. The static-IPoE UPnP status propagation fix and its policy regression landed in `1b06d3178c366ae764f894d6bb15847b8f5f6e21`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- Exact-head runs on parent `6e4511c0607b892bdc101f38f5bbf7c40b86fa49`: Process `43923656497` PASS and AI docs `43923656691` PASS. Previous XY-C1 run `43922140859` passed Build, TRX verification and artifact upload on product SHA `f5536dd46d24c8d0143c94a81a24a78a4d3cb15f`; it predates the UPnP fix.
- Commit `1b06d3178c366ae764f894d6bb15847b8f5f6e21` triggered fresh Process run `43927158630`, XY-C1 run `43927159072`, and AI docs run `43927158209`. At handoff inspection these were queued/running; check exact conclusions and job logs rather than infer success.
- PR #1 remains closed/unmerged historical work. No feature work on non-main branches.

## Active task

THIS COMMIT corrects the semantic checker added with the static-IPoE UPnP status fix. Review caught a double-escaped raw Python regex: it would look for literal backslash-s rather than whitespace and fail the source assertion. The expression now uses single raw-regex whitespace escapes; production source and policy test behavior are unchanged. Commit `1b06d3178c366ae764f894d6bb15847b8f5f6e21` remains the source fix whose exact-head checks must pass.

## Verification boundary

- The production fix in `net_wan.c` adds `wan_upnp_status` to `wan_start_status` with first-error semantics; the policy test covers both a first UPnP failure and preserving an earlier route error.
- A local GNU89/Werror/ASan/UBSan smoke test of first-error aggregation passed. The full repository policy binary and canonical checker were not runnable locally because this environment could not resolve GitHub raw-content DNS.
- This checker-only correction must pass exact-head Process validation. The current main SHA must also pass XY-C1 Build, TRX verification and artifact upload before current product-source L3 can be claimed. Physical-router L4 remains NOT VERIFIED.
- Frozen backup remains unchanged.

## Exact first action for the next GPT

1. Inspect parent `1b06d3178c366ae764f894d6bb15847b8f5f6e21` Actions, especially Process `43927158630` and XY-C1 `43927159072`, to identify any failure attributable to the double-escaped checker. Then inspect THIS COMMIT's exact-head Process, XY-C1 and AI docs runs.
2. Require the corrected checker to pass and verify the full static-IPoE policy regression on the same SHA.
3. Continue the P1 WAN/RC call-site audit for int-returning operations whose errors are only logged or discarded; prioritize required route, interface and configured-service transitions.
4. Keep the backup read-only and physical-router L4 marked NOT VERIFIED.

