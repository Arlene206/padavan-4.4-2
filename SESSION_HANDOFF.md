# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Parent main before THIS COMMIT: `f5536dd46d24c8d0143c94a81a24a78a4d3cb15f`. It includes the NDIS control-session startup gate and the checker prefix correction for the line-wrapped `ndis_control_network()` definition.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- USB modem identity workflow on source parent `b61ca225bc996bbace4000a7aa5aa6972f9cf5db` run `43921625498` PASS. Prior XY-C1 `43921624845` and Process inventory/fail-closed `43921624682` both failed because the modem source checker did not match the wrapped function signature; the parent fixes the prefix.
- WAN teardown `43921624637` failed at an inline assertion requiring the old `if (control_status != 0)` source pattern. The stop path now legitimately uses `modem_ndis_control_failed(control_required, control_status)` so optional no-op does not become a false teardown failure. THIS COMMIT updates that assertion. NVRAM `43921624686` PASS.

## Active task

THIS COMMIT aligns the WAN teardown workflow's static contract with the required/optional NDIS control result API. It checks that the stop function initializes and passes `control_required` and gates required failures through `modem_ndis_control_failed()`. No production source changes.

## Verification boundary

- Parent modem identity regression `43921625498` PASS. Parent XY-C1 and Process regressions were blocked by the stale function-signature checker, and WAN teardown by the stale stop predicate; neither indicated a product-code failure. NVRAM `43921624686` PASS.
- THIS COMMIT triggers fresh WAN teardown, XY-C1, and Process regression validation. Exact-head results must be checked before marking these closed.
- No local compiler/firmware execution claimed; device-side modem testing and physical-router L4 remain NOT VERIFIED. Frozen backup remains unchanged.

## Exact first action for the next GPT

1. Inspect THIS COMMIT's exact-head WAN teardown run; confirm the updated optional/required control assertion passes.
2. Inspect exact-head XY-C1 and Process inventory/fail-closed runs; require the modem source checker to pass beyond NDIS extraction and verify GNU89 modem policy tests. Only mark firmware L3 if Build, TRX and artifact upload all pass on the same source SHA.
3. Continue one independent WAN/RC service-start result-propagation fix while pending checks run.
4. Keep frozen backup read-only and physical-router L4 NOT VERIFIED.
