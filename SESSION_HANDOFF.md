# 2026-11-18 continuation checkpoint — Adbyby detached-test cleanup race

## Current repository state
- Repository Arlene206/padavan-4.4-2, branch main. Live main before THIS COMMIT: 8df3cc2f11aa9e040588d28844952005aaed9126.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains read-only at 72f4f157a084f8184b9597ca5bf334b8080e5807. PR #1 remains closed/unmerged. Do not modify backup, PR #1 or GPT Session Rotator.
- Product firmware validation: source SHA 648e7353b58c1f33fddc4d6ba8bb65e195f6695f completed XY-C1 Build, TRX verification and artifact upload (run 44329872151). This evidence is limited to that exact product SHA.

## Active task
- Exact 8df3cc2 Process regression run 44333373061 failed at the Adbyby detached-start shell test after all assertions passed; its EXIT trap reported a temporary-directory rm -rf cleanup error.
- Production correctly launches Adbyby asynchronously. The test fake daemon writes the RUNNING marker and exits quickly, but the test did not reap that background child before resetting shared fixtures / deleting its temporary directory.
- THIS COMMIT adds a test-only status-preserving run_and_reap_detached_test_child() wrapper, uses it for all Adbyby start/rule-update test invocations, and adds a checker assertion. Production startup semantics are unchanged.
- The Process regression workflow's explicit path allowlist includes both modified test and checker files.

## Verification boundary
- On exact 8df3cc2, AI handoff 44333370564 PASS. Process regression 44333373061 FAIL due to the single Adbyby test temp cleanup race; all preceding suite steps and the Adbyby readiness/restart assertions passed.
- The exact-head rerun for THIS COMMIT must prove the test reaper avoids the post-assertion cleanup failure and all Process lifecycle assertions remain green.
- No local checkout-based test was run in this connector-only session. Physical WAN/LAN/watchdog/AP-client/iNIC runtime and hardware L4 remain NOT VERIFIED.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main and exact-head Actions before ending. Leave this handoff unchanged if active task, verification boundary and first next action remain accurate.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head Process lifecycle and AI-doc Actions for THIS COMMIT. If Process fails, inspect its first failing step/log and fix the cause without weakening assertions.
2. Confirm run_and_reap_detached_test_child() is exercised for both adbyby_start and adbyby_uprules, and that the parent shell preserves each original return status.
3. If Process and AI docs pass, continue the AdGuardHome privileged workdir hardening candidate; reject symlink/foreign-owned /tmp/AdGuardHome before root daemon writes, and add race/ownership regressions.
4. Do not repeat completed trigger-scope, PPPoE-MWAN or Shadowsocks directive audits.

## Cross-session execution mandate
One logical slice is one atomic main commit. Build the complete Git tree from the exact parent and update main only with a non-forced expected-SHA compare-and-swap. Continue independent work while Actions are pending. Before context exhaustion, record exact first action, commit SHA, tests and CI boundary for the next GPT. Leave the backup immutable.

## Before context exhaustion
Recheck live main and exact-head Actions, complete the safe slice and preserve an accurate first action and unverified boundary for the next GPT.

## Whole-repository priorities
Stability defect discovery and repair remains primary, with security review in parallel. Prioritize severe system faults and major security vulnerabilities across the full userland while preserving architecture, compatibility and required tests.
