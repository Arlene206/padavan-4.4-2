# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; develop on `main` only.
- Main at the start of this slice: `bc3cceebfcc4423d03acd30a5604e01c63b13419`. Samba checked NVRAM gates were added in this commit; their first source-contract run failed on a multiline assertion and is corrected in THIS COMMIT. The WAN DHCP NVRAM state publisher separates publication failure from compensation failure; its guarded fresh-transaction retry runs only when compensation itself failed.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Never modify, checkout, merge, reset, rename, prune or delete it. Only main and backup branches are authorized.
- Legacy PR #1 remains closed and unmerged; do not reopen or develop through its branch.
- Protected reliability work includes checked optional-daemon settings, ttyd/SSH mode wrappers and detached-child probes, WAN DHCP compare-and-publish, stale-writer guarded recovery and validate-first reverse tuple construction for post-commit 6rd failure.

### Current source slice

- Parent main SHA: `ae4ae4a57b2c34d6dccf2541161a51606170047c`; current source contains Samba checked global-enable/mode/WINS state and guarded WAN DHCP publication/rollback.
- FTP enablement now uses a checked NVRAM read, strict integer parsing and explicit 0/1 validation while preserving the historical missing-key default of enabled. Both `run_ftp()` and `restart_ftpd()` refuse side effects after an unreadable enable flag; restart checks before inventory and stop. This commit adds GNU89 policy tests and source-contract coverage.
- Recovery commit `954d1dc4d11e5518e6e69e86a167fcfd0e5efd33` restored the full repository tree after a transient concurrent partial-tree update. Main remains the only development branch; backup is immutable.
- This source commit requires same-SHA XY-C1 Build + TRX verification + artifact upload. L3/L4 are NOT VERIFIED until their full gates pass.


## Active task

1. Verify storage-hotplug source contract after all seven captured-mode wrappers are represented.
2. Inspect exact-head DNSMasq/Samba and XY-C1 Build/TRX/artifact Actions for 985e86b3; require Build + TRX + artifact on a single latest SHA for L3.
3. Continue the next distinct stability/security item outside these completed wrapper contracts.


## Verification boundary

- L1: Seven restart wrappers use checked preflight configuration, preserve captured startup mode and refuse launch after failed stop; source behavior was inspected for all seven.
- L2: Storage-hotplug checker follow-up is pending. DNSMasq integer WINS test and source contracts are wired but exact-head results pending.
- L3: NOT VERIFIED until Build XY-C1, TRX and artifact upload all pass on the same SHA.
- L4: Physical router USB eject/reinsert, DHCP and WINS runtime NOT VERIFIED.


## Workflow and preservation rules

- Main only. One logical slice, one atomic main commit: create all blobs, one tree based on current main, one commit with current main as parent, then non-forced update_ref using the exact expected_sha. No force push, amend, history rewrite, branches, PR-based development or partial per-file commits.
- Re-fetch main immediately before writing. On a moved ref, discard the stale prepared tree and rebuild from the new parent.
- Inspect push Actions only when head_sha exactly matches the commit. Diagnose failures from their logs. PENDING does not block independent work.
- Preserve security, architecture, ABI, GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Never weaken useful tests or focused workflow path coverage.
- Do not touch GPT Session Rotator workflow/state or replay SHA 9d5a5cb. The backup branch is permanent immutable recovery data.

## Cross-conversation documentation governance

- GitHub is the durable cross-conversation record; the one-line user entrypoint remains sufficient.
- dev_log.md stays newest-first with 20–119 complete records. Before record 120, archive the oldest 100 complete records verbatim into docs/dev-log/YYYY/MM/part-NNN.md in one batch, leaving 20 newest entries.
- Rotate early only above the 384 KiB target and never exceed 512 KiB. Preserve archive/index bytes. Git history is the version archive for handoff and roadmap; routine snapshots and SHA-chasing commits are prohibited.
- Update dev_log.md for real milestones, this handoff when first-next-action facts materially change, and REFACTOR_ROADMAP.md only when repository-wide module status/priorities materially change.
- Before context exhaustion, finish the safe slice, commit atomically, inspect exact-head Actions and leave the next precise task.

## Exact first action for the next GPT

Fetch live main; inspect the exact-head storage-hotplug run for the seven mode wrappers, DNSMasq/Samba policy Actions and XY-C1 Build/TRX/artifact results. Continue broader userland reliability work.
Fetch live main; inspect the exact-head numeric WINS regression, storage-hotplug check and XY-C1 Build/TRX/artifact results. Then continue the next independent whole-userland stability issue.
Fetch live main; inspect exact-head storage-hotplug, XY-C1 Build/TRX/artifact and other affected Actions for the checked-mode wrapper contract correction. Then continue the next independent whole-userland stability issue.
Fetch live main; inspect the exact-head DNSMasq WINS source-order regression, Samba regression and XY-C1 Build/TRX/artifact runs for this patch. Then continue the next independent whole-userland stability issue.
Fetch live main; inspect the fresh Samba run from the strengthened WINS-header contract and XY-C1 #44917653757 (Build/TRX/artifact) against the complete restored tree. Then assess the independent DNSMasq WINS read and continue broader userland reliability work.
Fetch live main; inspect exact-head Samba source-contract and XY-C1 Build/TRX/artifact runs for this WINS-header patch. Then assess the independent DNSMasq DHCP WINS NVRAM read and continue the next userland reliability issue.
Fetch live `main`; inspect exact-head WAN callback, WAN route, AI handoff and XY-C1 Build/TRX/artifact runs after THIS COMMIT. Then continue the Samba checked-NVRAM fail-closed implementation while any firmware Actions are pending.
