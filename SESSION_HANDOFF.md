
# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and the first next action. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, do not create a no-op commit.

Before context exhaustion, finish the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue as one cross-chat engineering effort: recover once, avoid repeat audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Parent for this correction: `d5d07a7027457ecd56ec217d1f4191ab8227d7ea`. The preceding source slice is `bca48425f16b97cdb2b599401bd105fbc5e652ad`, which fixed tri-state module-load errno propagation in the shared policy, NAT, boot hardware and LAN bridge callers.
- Follow-up source review found the final missed production caller: `load_wireless_module()` still supplied Boolean `is_module_loaded()`. Exact-head bca Firewall/WAN checks caught it; this correction changes it to `is_module_loaded_status()`.
- The bca AI handoff check also caught the missing exact phrase in this execution checkpoint; this file includes the required "Before context exhaustion" rule.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only and unchanged. Main is the only development branch.

## Active task

Complete strict tri-state module-load propagation. The shared verifier maps 1=loaded, 0=absent and negative errno=probe failure; NAT, boot hardware and LAN bridge already use the strict probe. This correction migrates the final wireless-driver loader as well. Direct and required-module fault-injection tests plus Process/WAN semantic checkers cover the policy and all four actual production users.

## Verification boundary

- Exact-head bca Firewall and WAN checks failed because the module-load checker correctly found the remaining Boolean wireless callback. This correction addresses the production source defect; do not count the bca runs as green.
- Exact-head bca AI handoff failed because the "Before context exhaustion" phrase was absent; it is restored here.
- Earlier parent `eb566f3` XY-C1 Build, TRX verification and artifact upload passed, but newer IPv6 cleanup and module-load changes require their own exact-source verification.
- Fresh exact-head WAN, Firewall, Process lifecycle, HTTPD, AI handoff and XY-C1 checks must pass on the correction SHA. Firmware Build/TRX/artifact and physical-router L4 are not claimed without same-SHA evidence.
- Physical-router L4 remains **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch live `main`; inspect every relevant push run whose `head_sha` exactly matches it. Repair each failure from its job log; never reuse results from an older SHA.
2. Once strict module-load checks pass, continue evidence-backed P1 RC/service startup-result and lifecycle work across userland instead of repeating completed module-probe analysis.
3. Keep one logical slice, one atomic main commit, one complete tree, and non-forced expected-SHA compare-and-swap updates. Never force-push, create work branches/PRs, or modify the frozen backup. Do not claim physical-router L4 without hardware evidence.
