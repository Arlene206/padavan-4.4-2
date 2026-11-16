# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Main parent for this service-action fix: `e09e044195fd936142ed0b42ead1bd73f6262c02`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`, unchanged/read-only.
- The strict module-state probe hardening is on main at `4c43578509463afc2d25c05892ff39075245f173`: tri-state `is_module_loaded_status()`, legacy boolean wrapper, verified-unload error propagation and two injected failure-path tests.
- Exact-head 4c435 Actions at last poll: 38 of 39 completed runs passed, none failed; XY-C1 `43953778444` was still in progress. Firewall Startup `43953778572`, Process integration `43953778499`, HTTPD `43953778353`, AI handoff `43953778474`, and Process inventory `43953778187` passed.
- A further RC notification defect is confirmed: `RCN_RESTART_UPDATEADB` called a void `update_adb()` wrapper that discarded the `eval()` wait status. This slice changes it to return the status and merges it into the RC action result.

## Active task

Finish the Adbyby rule-update result path. The `RCN_RESTART_UPDATEADB` notification previously invoked a void wrapper that called `eval("/usr/bin/adbyby.sh", "updateadb")` and discarded its wait status. `update_adb()` now returns the status, the header matches, and the RC notification branch merges it with `rc_notify_merge_action_result()`.

The preceding strict module-probe change is under exact-SHA verification; keep its results tied to SHA `4c43578509463afc2d25c05892ff39075245f173`.

## Verification boundary

- Exact-head 4c435 Process integration `43953778499`, Firewall Startup `43953778572`, HTTPD `43953778353`, AI handoff `43953778474`, and Process inventory `43953778187` passed. Overall 38/39 Actions were complete and green at last poll; XY-C1 `43953778444` remained in progress.
- Strict-probe initial/final error tests are included in 4c435 and passed in Process and Firewall Startup workflows. Firmware Build/TRX/artifact on 4c435 is still pending until the XY-C1 job completes.
- The Adbyby update status fix in this commit has not yet been built or tested on its new exact SHA. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Check exact-SHA Process integration, HTTPD, Firewall Startup and XY-C1 for the Adbyby update action status fix; inspect Build/TRX/artifact before claiming current firmware verification.
2. Recheck the 4c435 XY-C1 Build/TRX/artifact completion, then continue auditing demonstrated dropped RC service-action results across the remaining notifier handlers.
3. Main-only atomic complete-tree commits; expected-SHA CAS, no force push. Frozen backup read-only; physical-router L4 NOT VERIFIED.
