# 2026-11-17 continuation checkpoint — detached ttyd startup verification

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS ttyd readiness slice: `56eff0e0e7c27fd1c632eb41e96df733febd72aa`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged.
- The previous handoff's XY-C1 full-Git-diff detector task is complete. The detector uses full before/after Git tree diffs and fails safe to a full build; do not repeat that investigation.
- Parent `5ddc0319` introduced exact lower-level WAN restart failure preservation. Its first exact-head runs exposed only stale source-contract syntax; the repairs on `56eff0e0` are green: HTTPD `44236830849`, WAN route `44236830756`, WAN teardown `44236830853`, AI docs `44236830825`.
- Main parent `56eff0e0` Process inventory/umbrella checks for the WAN change passed. Its exact-head XY-C1 run for product source `5ddc0319` was still PENDING when checked; do not infer a current-source firmware result from earlier SHAs.
- Found this slice's concrete RC defect: `ttyd.sh` invokes `start-stop-daemon -S -b`, but `start_ttyd()` previously returned zero as soon as the wrapper returned. A detached child that immediately exited (for example, because it could not bind) could be reported as started. THIS slice adds a bounded child-presence postcondition and source contracts. Exact-head Process/XY-C1 evidence is pending until Actions run.
- Physical-router L4 remains NOT VERIFIED.

## Active task
- Verify the exact-head ttyd detached-child startup contract and current XY-C1 Build/TRX/artifact result. The Process checker requires the launch, initial wait, bounded inventory probes, RUNNING postcondition, fail-closed UNKNOWN handling, and timeout ordering.
- After green verification, inspect optional RC/add-on launchers for other cases where an asynchronous wrapper's zero status is mistaken for confirmed RUNNING. Trace the wrapper contract first; several add-ons already implement their own readiness checks.

## Verification boundary
- Exact WAN checker repair evidence on `56eff0e0`: HTTPD `44236830849`, WAN route `44236830756`, WAN teardown `44236830853`, AI docs `44236830825` all PASS.
- Parent source `5ddc0319`: Process inventory and Process umbrella passed; XY-C1 `44236350451` was IN_PROGRESS at last poll. That build is not evidence for the new ttyd source.
- THIS ttyd source/checker/docs slice awaits exact-head Process lifecycle, Process inventory, AI handoff and XY-C1 checks. No local production cross-build or physical-router run is claimed.

## Exact first action for the next GPT
1. Fetch the live main SHA and inspect exact-SHA Process lifecycle/inventory, AI handoff and XY-C1 Actions for THIS ttyd change. Diagnose and fix any failure at its originating SHA.
2. Confirm the XY-C1 job reached Build XY-C1, Verify TRX output and Upload firmware artifact on that same source SHA; host checks alone are L2, not L3.
3. If checks pass, audit async RC optional-app launchers, especially AdGuardHome/DDNSTO/Adbyby/AliDDNS, against their own shell readiness behavior; add a concrete regression before changing any caller.
4. Keep the frozen backup unchanged and do not rerun already completed XY-C1 detector/TUN audits absent a new source trigger.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. At session end inspect live main, exact-SHA Actions, current handoff/roadmap, validation boundary and first actionable task. If all remain accurate, explicitly conclude with no file change and no commit.

Do not create no-op documentation commits; update only at meaningful validation boundaries.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, implement concrete defects, test and commit atomically; do not wait idly on pending CI.

Every logical slice is one atomic main commit: create a complete tree and commit parented to freshly fetched main, then publish with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs, or modify frozen backup.

Before context exhaustion, finish a safe slice, inspect exact-SHA Actions and leave the next GPT a precise first task.
