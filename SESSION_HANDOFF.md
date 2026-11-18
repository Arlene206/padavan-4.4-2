# 2026-11-18 continuation checkpoint — cover watchdog headers in XY-C1 validation

## Current repository state
- Repository @@Arlene206/padavan-4.4-2@@; branch @@main@@. Live main immediately before THIS COMMIT: @@ecb4931e0b2b2be4e7dcbf8acaa9dbf9a82432e0@@.
- Product-source baseline: @@648e7353b58c1f33fddc4d6ba8bb65e195f6695f@@. It includes integer-return status propagation for watchdog time notifications, detect-internet notifications, and WAN/LAN/PID 1 callers.
- Frozen backup @@backup/pre-plugin-overhaul-2026-11-14@@ remains read-only at @@72f4f157a084f8184b9597ca5bf334b8080e5807@@. PR #1 remains closed/unmerged. Do not modify backup, PR #1 or GPT Session Rotator.

## Active task
- This slice closes a CI trigger coverage gap: XY-C1's push path list watches @@watchdog.c@@ but not its production policy headers @@watchdog_ntp_policy.h@@ and @@watchdog_signal_policy.h@@. A header-only behavior change could bypass target firmware validation.
- Add both header paths to XY-C1's push filter, a Python regression in @@scripts/tests/test_xy_c1_firmware_inputs.py@@, and a watchdog source-contract assertion that pins the target workflow's entries.
- The Process lifecycle integration checker has now been corrected at both stale @@notify_watchdog_time()@@ extraction sites to expect @@int@@; it also asserts policy mapping and the returned operation error.

## Verification boundary
- Product source SHA @@648e7353b58c1f33fddc4d6ba8bb65e195f6695f@@ XY-C1 run @@44329872151@@ had all host validations passed and the firmware build in progress, with TRX/artifact pending at last observation. NVRAM transaction audit @@44329871839@@ PASS.
- Checker repair commit @@2c71477765b5e11867db4eeec1a3fb20bbb80321@@ exact watchdog @@44330853992@@ PASS and AI docs @@44330853859@@ PASS; Process lifecycle @@44330853765@@ failed only because the second old watchdog void signature remained at two checker sites. Follow-up @@ecb4931e0b2b2be4e7dcbf8acaa9dbf9a82432e0@@ corrected both sites and added return-policy assertions; exact Process @@44331321091@@ and AI docs @@44331321185@@ were in progress at last poll.
- The checker-only @@ecb4931@@ update did not trigger XY-C1 because its changed paths were outside the target workflow filters; this commit changes the target workflow and its host test, so the exact-head XY-C1 host trigger contract should run. No product input is changed by this commit, so the input detector should skip the full firmware rebuild.
- No local checkout tests or physical router checks were run. Physical WAN/LAN/watchdog/AP-client/iNIC behavior and L4 remain NOT VERIFIED.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Inspect live main and exact-head Actions before ending. Leave this handoff unchanged if its active task, verification boundary and first action remain accurate.

## Exact first action for the next GPT
1. Fetch the live main SHA and inspect exact-head push runs. Confirm Process lifecycle @@44331321091@@ and AI docs @@44331321185@@ results for @@ecb4931@@; inspect this commit's XY-C1 trigger-contract and watchdog workflow runs.
2. Refresh product SHA @@648e7353b58c1f33fddc4d6ba8bb65e195f6695f@@ XY-C1 run @@44329872151@@ through Build, TRX and artifact upload before making any L3 claim.
3. If any exact-head check fails, fix its causal contract or source defect without weakening first-error, fail-closed or firmware assertions.
4. Continue the next independent stability/security slice across userland; do not repeat completed startup audits.

## Cross-session execution mandate
One logical slice is one atomic main commit. Build the complete Git tree from the exact parent and advance @@main@@ only with a non-forced expected-SHA compare-and-swap. Continue independent tasks while Actions are pending. Before context exhaustion, record the exact first action, commit SHA, tests and CI boundary for the next GPT. Leave the backup immutable.

## Before context exhaustion
Reconfirm live main and exact-head Actions, finish the current safe slice and leave an accurate first action and unverified boundary for the next GPT.

## Whole-repository priorities
Stability defect discovery and repair remains primary, with security review in parallel. Prioritize major security vulnerabilities and severe system faults. Expand coverage across Padavan userland rather than repeatedly auditing already completed subsystems. Preserve architecture, compatibility and necessary tests.
