# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe source slices, add regression coverage, inspect exact-SHA Actions, commit atomically, and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development only on `main`.
- Current implementation commit: THIS COMMIT; parent main was `9116f521c171d011f45a9a19abebcf57cdbedcc9`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`. The branch inventory contained only `main` and this protected backup.
- The immediate parent `9116f521c171d011f45a9a19abebcf57cdbedcc9` had 11 completed exact-head `push` workflows, all SUCCESS. In particular XY-C1 run `44020469451` completed the cross-build, Build XY-C1, TRX verification and artifact upload; Process lifecycle `44020469301`, WAN route `44020469586`, and Firewall Startup `44020469423` also passed.
- THIS COMMIT fixes a P1 WAN-up status defect: `wan_up_internal()` previously discarded `notify_ddns_update()`. Enabled Inadyn process-inventory, daemon-start, configuration-publication and HUP failures now flow into the WAN-up first-error result. The notifier now treats DDNS-disabled and AP-mode configurations as successful no-ops before probing/starting Inadyn, avoiding a false error from `start_ddns()` when the optional service is intentionally disabled.
- Regression coverage extends `inadyn_daemon_policy.c`, adds a WAN caller-contract checker, and makes the WAN route workflow compile/run the policy test and checker. The XY-C1 and Process workflows already watch the touched RC sources/header/test paths.
- Exact-head Actions for THIS COMMIT were PENDING at commit creation. Do not claim L3 until the exact SHA has green source gates and XY-C1 Build, TRX verification and artifact upload.
- Local checkout/toolchain is unavailable; local compilation was NOT RUN. Physical-router L4 remains NOT VERIFIED.

## Active task

1. Fetch live `main` and inspect every `push` workflow whose `head_sha` equals THIS COMMIT. Resolve any FAIL on that exact SHA. Require WAN route, Process lifecycle and XY-C1 to complete; verify the XY-C1 job reaches Build, TRX verification and artifact upload.
2. Once that SHA is green, continue P1 caller-result work in `wan_up_internal()`: inspect the remaining enabled optional WAN-up launch paths (including ZeroTier and the PPPoE-MWAN detection script), determine their disabled/no-op semantics, and propagate only failures that represent real configured-service faults. Add caller tests, not helper-only checks.
3. Preserve the frozen backup; use one tree/one commit per logical slice and a non-forced expected-SHA update.

## Exact first action for the next GPT

Read the live `main` SHA and query Actions filtered by that exact `head_sha`; do not re-audit the already-green parent `9116f521`. Confirm the WAN DDNS policy/caller checker, Process lifecycle checks, and same-SHA XY-C1 Build/TRX/artifact steps. If any check is pending, work independently on the next WAN-up caller rather than waiting. Update this handoff in the next source-bearing commit with the actual commit SHA and CI outcomes.