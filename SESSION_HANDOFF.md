# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify live main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and the first next action. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, do not create a no-op commit.

Before context exhaustion, finish the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue as one cross-chat engineering effort: recover once, avoid repeat audits, do not wait on pending CI when independent work is available, and do not ask the user for the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into half-commits, create work branches/PRs, or modify the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Parent at this checkpoint: `b6ff015c1a70e9b61bebfbcf33aae9d1de113946`. Commit `b6ff015c` completes strict tri-state module-load callbacks across NAT, boot hardware, wireless drivers and LAN bridge.
- Exact-parent Firewall Startup `43966109926` passed. WAN route `43966109986` failed at a whitespace-sensitive IPv6 cleanup call assertion; XY-C1 `43966109916` failed at a stale checker that expected the old Boolean policy; AI handoff `43966109747` found that root `dev_log.md` had only one retained entry. This follow-up normalizes the call check, upgrades the boot checker to tri-state semantics and restores the 109-entry historical developer log.
- Process regression `43966109822`, XY-C1 `43966109916`, NVRAM `43966109901`, GPIO `43966109653`, notification queue `43966109981`, AliDDNS `43966109957` and USB storage `43966109731` were in progress/pending at last poll. Recheck exact-head statuses; other completed b6ff runs passed apart from the failures above.
- Earlier source `eb566f346f63d3d6ed295c08170ac6d834ff7f04` passed XY-C1 Build, TRX verification and artifact upload. That predates the later IPv6 cleanup and module-load changes. The b6ff XY-C1 run did not pass its source checker and did not reach Build/TRX/artifact. Newer L3 remains NOT VERIFIED.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains recorded at `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. Never alter it. Physical-router L4 remains **NOT VERIFIED**.

## Active task

Close exact-head CI contracts for the strict module-state migration: check the multiline IPv6 cleanup helper after removing all whitespace; validate the updated tri-state verifier plus strict boot/wireless callbacks in `check_rc_boot_module_loads.py`; and preserve the root developer-log history window of 20–119 complete newest entries. The production wireless callback fix is already included in parent `b6ff015c`.

## Verification boundary

- Exact-parent WAN route `43966109986` failed only at the whitespace-sensitive call check after its compiled C policy tests passed.
- Exact-parent XY-C1 `43966109916` failed the outdated assertion `return ops->is_loaded(module_name) ? 0 : EIO;`; the revised contract now tests negative errno propagation and strict callbacks. No Build/TRX/artifact on b6ff.
- Exact-parent AI handoff `43966109747` failed because the root log had only one entry; this update restores 109 historical entries plus the current record (110 total).
- Exact-parent Firewall Startup `43966109926` passed. The older `eb566f3` firmware result is not reused for this source. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main`; inspect exact-head WAN route, XY-C1, AI handoff, Process regression, Firewall Startup and every previously pending run. Diagnose any failure from its exact job log.
2. Confirm same-SHA XY-C1 Build, TRX and artifact upload after the source checker passes. Then audit remaining direct Boolean `is_module_loaded()` consumers only where probe uncertainty affects a state transition.
3. Keep one logical change per complete-tree atomic main commit with a non-forced expected-SHA update. Keep the backup read-only; do not claim router L4 without hardware evidence.
