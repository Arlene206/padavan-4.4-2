# 2026-11-18 continuation checkpoint — add SCSI policy paths to pull_request filter

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS workflow-filter repair: `2fc75549023388f2e90da2c324c372873cee7ee9`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged.
- Exact parent USB modem workflow `44211715745` compiled and ran the new SCSI eject/unload GNU89/Werror/ASan/UBSan policy regression successfully, then passed the new source-function assertions. It failed only at its final path-filter assertion: the policy header and test were present under `push` but omitted from `pull_request`.
- Exact parent AI handoff `44211715782` PASS. HTTPD Security `44211715867`, NVRAM audit `44211716167`, Process inventory `44211715739`, and other security/lifecycle checks were pending at last poll.
- THIS workflow-only change adds both policy paths to the pull_request filter; both files are already present once in the push filter. Production behavior and regression assertions are unchanged.
- Production SCSI CD fix `2fc75549` captures/logs eject and unload statuses. If unload helper returns zero, the tri-state `is_module_loaded_status()` probe treats confirmed absence as idempotent completion, while a present or unknown module remains a failure. Positive unload status preserves success/in-use no-op.
- No exact-current-SHA XY-C1 Build/TRX/artifact evidence or physical-router L4 claim.

## Active task
- Ensure the SCSI result-policy regression and caller source contract run on both push and pull_request events.

## Verification boundary
- Exact parent USB modem GNU89/Werror/ASan/UBSan policy test PASS; inline source assertions PASS; final path-filter assertion FAIL due missing two PR path entries.
- THIS filter-only commit's exact-head USB modem rerun pending.
- Exact parent AI handoff PASS; HTTPD Security, NVRAM audit and Process inventory were pending. Firmware L3 and physical L4 NOT VERIFIED.

## Exact first action for the next GPT
1. Inspect exact-head USB modem workflow; require SCSI policy test, WDM identity assertions, source contract and both push/PR path filters to PASS.
2. Inspect HTTPD Security, NVRAM audit and Process inventory exact-SHA results.
3. Inspect same-SHA XY-C1 Build/TRX/artifact if a run exists; L3 only after all three pass.
4. Continue independent P1 reliability/security work while pending Actions run. Keep backup unchanged; do not repeat bootstrap recovery.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional.
At session end inspect live main, relevant exact-SHA Actions, current handoff/roadmap, validation boundary and first actionable task. If all remain accurate, do not create a no-op documentation commit.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, implement concrete defects, test and commit atomically; do not wait idly on pending CI.

Every logical slice is one atomic main commit: create all blobs, one complete tree and one commit parented to freshly fetched main, then publish with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs, or modify frozen backup.

Before context exhaustion, finish a safe slice, inspect exact-SHA Actions and leave the next GPT a precise first action.
