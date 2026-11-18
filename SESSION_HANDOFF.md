# 2026-11-18 continuation checkpoint — narrow Process regression workflow scope

## Current repository state
- Repository Arlene206/padavan-4.4-2, branch main. Live main immediately before THIS COMMIT: 26e98f4103084938307951148ea51f5b01ed3613.
- Product source SHA 648e7353b58c1f33fddc4d6ba8bb65e195f6695f has completed XY-C1 firmware Build, TRX verification and artifact upload (run 44329872151). This evidence is tied to that exact product SHA only.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains read-only at 72f4f157a084f8184b9597ca5bf334b8080e5807. PR #1 remains closed/unmerged. Do not modify the backup, PR #1 or GPT Session Rotator.

## Active task
- The process-lifecycle-regression workflow had a broad trunk/user/** push path despite also maintaining an explicit allowlist, causing unrelated userland package assets and documentation changes to run the long lifecycle regression. Its push and pull_request path lists also diverged, leaving some lifecycle source/test paths without PR validation.
- THIS COMMIT removes the catch-all, unifies both event filters to the de-duplicated union of existing explicit patterns, and preserves the AliDDNS tests exclusion. The independent process-lifecycle-inventory workflow retains its wider trigger because it re-enumerates tracked source candidates across the userland tree.
- A source-level contract checks no blanket catch-all, no duplicate entries, push/PR parity and retention of essential workflow/checker/source paths.

## Verification boundary
- Before THIS COMMIT, exact 26e98f4103084938307951148ea51f5b01ed3613 Actions all passed: AI docs 44331581877, watchdog signal 44331580939, Process lifecycle 44331581154, and XY-C1 host validation 44331580944. The XY-C1 run was correctly host-only because no product input changed.
- Product source 648e7353b58c1f33fddc4d6ba8bb65e195f6695f XY-C1 run 44329872151 completed the MIPS build, TRX verification and artifact upload successfully. This does not cover later product-source commits.
- The workflow/checker change in THIS COMMIT requires exact-head Process lifecycle and AI-doc verification. No firmware inputs changed, so no new firmware build is required. No local checkout tests or physical-router L4 checks were run.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live main and exact-head Actions before ending. Leave this handoff unchanged if the active task, verification boundary and first next action remain accurate.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head Process lifecycle and AI-doc Actions for THIS COMMIT. Diagnose the first causal log if either check fails; do not relax the trigger-parity contract.
2. Confirm the AliDDNS exclusion remains last in each path list and that both event path sets are identical.
3. Continue the next independent stability/security task, prioritizing a concrete reliability or privileged file-boundary defect. Do not repeat already completed PPPoE-MWAN or Shadowsocks directive work.

## Cross-session execution mandate
Continue across GPT conversations as the same maintainer. One logical slice is one atomic main commit. Create the complete Git tree from the exact parent and advance main only with a non-forced expected-SHA compare-and-swap. Continue independent tasks while Actions are pending. Before context exhaustion, record the exact first action, commit SHA, tests and CI boundary for the next GPT. Leave the backup immutable.

## Before context exhaustion
Recheck live main and exact-head Actions, finish the current safe slice and preserve an accurate first action and unverified boundary for the next GPT.

## Whole-repository priorities
Stability defect discovery and repair remains primary, with security review in parallel. Prioritize severe system faults and major security vulnerabilities. Expand coverage across Padavan userland rather than repeating completed subsystem audits. Preserve architecture, compatibility and necessary tests.
