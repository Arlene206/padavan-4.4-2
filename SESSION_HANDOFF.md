# 2026-11-18 continuation checkpoint — reduce CI trigger fan-out

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS CI workflow correction: `c500c3a48328fc54e8949a75187de648ec0f78a6`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged.
- Audited all 54 root GitHub Actions workflow files and inspected exact-head fan-out. The `7f10d225` push started 11 workflows; several unrelated module regressions ran only because canonical handoff/roadmap/log documents were listed in their path filters. The AI documentation contract also had an unrestricted push trigger.
- Removed canonical-document triggers from AdGuardHome, AliDDNS, AliyunDrive, infosvr, NVRAM and Process lifecycle regression. The AI documentation workflow now runs on main only for canonical records, log archives, its validator or its own workflow.
- XY-C1 no longer starts because the independent HTTPD Security workflow YAML changed. Its host regression/source-contract steps still run for configured paths, but `make XY-C1`, TRX verification and artifact upload now require changed firmware production inputs. Workflow/test/checker/documentation-only pushes skip only target-build stages; `workflow_dispatch` still performs a full build. Missing or truncated push file lists fail safe and retain the build.
- Added latest-push concurrency cancellation to the selected long-running/cross-cutting workflows updated here. An older run of the same workflow/ref is superseded when a new push starts.
- Preserved broad NVRAM and Process Inventory scans because their audit scripts intentionally enumerate wider source trees; preserved Storage's BusyBox/SmartDNS patterns because that workflow builds BusyBox and runs those regressions. Process lifecycle regression remains a separate cross-module suite.
- No firmware product source changed in THIS CI-only slice. Do not claim target firmware L3 or physical-router L4 from host regression results.

## Active task
- Verify this CI trigger/fan-out correction on its exact pushed main SHA, then resume validation of the NDIS metadata cleanup source change already present on parent `c500c3a48328fc54e8949a75187de648ec0f78a6`.

## Verification boundary
- Exact-head Actions for THIS correction are pending until GitHub runs are inspected.
- Expected behavior for this workflow-only commit: AI documentation validation runs; only workflows whose own configuration changed or whose explicit integration contract watches XY-C1 workflow configuration should run. XY-C1 host checks execute, while Build/TRX/artifact stages are intentionally skipped.
- Firmware L3 remains unverified until Build, TRX and artifact results are confirmed on one applicable source SHA. Physical-router L4 is not claimed.

## Exact first action for the next GPT
1. Fetch live main and query Actions by that exact SHA. Confirm AI handoff and intended workflow-config checks pass.
2. Inspect the XY-C1 run: host regressions must execute; Build, TRX and artifact must show intentional skip for this workflow-only commit. Do not mistake this for firmware L3.
3. Confirm module workflows no longer start solely because `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` or `dev_log.md` changed.
4. Inspect exact parent `c500c3a48328fc54e8949a75187de648ec0f78a6` Storage and XY-C1 results for the NDIS metadata cleanup change. Continue source validation and broaden reliability work after relevant failures are triaged.
5. Keep the frozen backup unchanged and publish any further multi-file slice atomically to main.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Do not create a no-op documentation commit when these facts remain accurate.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, implement concrete defects, test and commit atomically; do not wait idly on pending CI.

Every logical multi-file change is one atomic main commit: create all blobs, one complete tree and one commit parented to freshly fetched main, then publish with a non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs, or modify the frozen backup.

Before context exhaustion, finish a safe slice, inspect exact-SHA Actions and leave the next GPT a precise first action.
