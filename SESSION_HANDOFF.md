# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Continue one engineering effort across GPT conversations: avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe slices, add regression coverage, inspect exact-SHA Actions, commit atomically and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

Before context exhaustion, inspect the current source SHA, relevant exact-head Actions, validation boundary and first actionable task; then leave this handoff accurate for the next GPT.

- Repository: `Arlene206/padavan-4.4-2`; IPv6 boot-status slice base is `f7b975c6bbe3f891498602c6363017cf6e6cc4c8`; result: THIS COMMIT.
- CPU-watchdog UNKNOWN-probe correction is in product source `aeddc157`. Same-source XY-C1 run `44000632827` passed the GNU89 policy regression, Build XY-C1, TRX verification and artifact upload. Checker/docs-only child `58fe2c5a` does not change that firmware source.
- USB-modem driver readiness now uses tri-state `is_module_loaded_status()`: only exact state 1 counts as available; UNKNOWN returns the first probe errno when no alternative is confirmed; invalid positive states fail closed with EIO. Dedicated USB modem run `44002424147`, Process lifecycle `44002424268`, Process inventory `44002424300`, HTTPD security `44002424357`, infosvr `44002424164`, and AI handoff `44002424146` passed on product-source SHA `766832746b81a67faaa5d88e3ba78cbd1228faef`. XY-C1 run `44002424380` exposed only a stale source assertion; checker correction is in `f7b975c6`. The f7 exact-head XY-C1 `44002780753` and Process `44002780748` runs were in progress before this source commit; neither validates this change.
- This source slice closes a boot error-aggregation gap: `init_ipv6()` previously logged failures from `control_if_ipv6_all()` but returned void, so `init_router()` could not add that first sysctl error to its existing `boot_services_status` accumulator. It now returns the original status and the init path merges it without changing startup ordering. An integration contract pins this production path; the existing IPv6 policy test covers all-attempt/first-error semantics.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Legacy PR #1 is closed; its head has no unmerged commits relative to its old base.

## Active task

1. Fetch live main and inspect exact-`head_sha` push Actions for this commit. Resolve any first causal Process lifecycle or XY-C1 failure; do not rely on the parent f7 runs.
2. Confirm the IPv6 policy test and new integration contract execute on the exact SHA. Confirm XY-C1 Build, TRX verification and artifact upload on this source; only then mark L3 current-source verification PASS.
3. Once this startup-status slice clears, continue the broad P1 RC lifecycle/service-start audit and select the next concrete false-success/error-loss defect across a different subsystem. Do not repeat watchdog, USB-modem tri-state, storage eject, VPN restoration, NVRAM close, or IPv6 neighbour cleanup.

## Verification boundary

- Watchdog L3: PASS for product source `aeddc157`, run `44000632827` (Build, TRX verification and artifact upload). It does not validate subsequent USB modem or current IPv6 source changes.
- USB modem L2: dedicated GNU89 policy/source regression plus Process and related workflows PASS on source `76683274`. Exact-head XY-C1 `44002424380` failed only at the obsolete contract; current checker correction and this IPv6 source change require a new same-SHA run.
- Current IPv6 boot-status integration: FIXED — NOT VERIFIED until exact-head Process/XY-C1 Actions run. Existing policy-level GNU89 test had been wired into both workflows; current execution pending.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch current `main` and its push Actions by exact SHA. Inspect Process lifecycle and XY-C1 logs/steps; fix the first causal failure and re-run on the new SHA. A green host/policy suite is not an XY-C1 Build/TRX/artifact result. Preserve the frozen backup and use one complete Git tree with a non-forced expected-SHA compare-and-swap.
