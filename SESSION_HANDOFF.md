# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Parent main before THIS COMMIT: `a8acbb08d8cf2d8c80713ac2f670ef8ca9dd20e5`. It contains the NDIS required/optional control-status API and the line-wrap-resilient modem source checker.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- USB modem identity run `43921625498` PASS on source parent `b61ca225bc996bbace4000a7aa5aa6972f9cf5db`.
- WAN teardown run on parent `a8acbb08d8cf2d8c80713ac2f670ef8ca9dd20e5`, `43922685825`, PASS after its workflow contract was aligned. Process regression on ancestor `f5536dd...`, `43922140695`, failed because its shared integration checker still expected the old NDIS stop-call arity and raw status condition. THIS COMMIT fixes that checker. Its push and PR path filters already cover the script.
- XY-C1 run `43922140859` on `f5536dd...` was at Build XY-C1 / in_progress at last poll.

## Active task

THIS COMMIT aligns `scripts/check_process_lifecycle_integration.py` with the NDIS stop contract. It checks the whitespace-normalized four-argument call, `control_required` initialization and `modem_ndis_control_failed()`. No production source behavior changed.

## Verification boundary

- Parent WAN teardown `43922685825` PASS. Earlier USB modem identity `43921625498` PASS.
- Process integration `43922140695` failed at the stale NDIS stop-call assertion; THIS COMMIT triggers a new Process run.
- The XY-C1 checker prefix correction on `f5536dd...` still requires exact-head result inspection. Do not mark firmware L3 until the semantic checker, Build XY-C1, TRX verification and artifact upload all pass on a single source SHA.
- No local test execution claimed. Hardware/router L4 remains NOT VERIFIED. Frozen backup remains unchanged.

## Exact first action for the next GPT

1. Inspect THIS COMMIT's exact-head Process regression and AI docs run. Require all integration contracts to pass, including the updated NDIS stop assertion.
2. Check XY-C1 run `43922140859` for the earlier checker fix; if failed or pending, diagnose exact logs and rerun only with a justified validation-triggered source/workflow change.
3. Confirm WAN teardown `43922685825` remains PASS; then continue one independent P1 WAN/RC startup-result propagation issue while pending checks run.
4. Keep frozen backup read-only and physical-router L4 NOT VERIFIED.
