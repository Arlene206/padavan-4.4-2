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

- Main at the start of this numeric-compatibility slice: a27d1c9b91e31f173bcf36860029a3e16a77af77.
- Exact-head DNSMasq #44918016034, Samba #44918015899, AI docs #44918016020, Inadyn #44918016009, Process inventory #44918015910, miniupnpd #44918015959, WAN route #44918015928, WAN DHCP #44918015926 and HTTPD security #44918015945 all passed on bd0a0252.
- Storage hotplug checker correction is committed at a27d1c9b; its exact-head workflow #44918173009 and XY-C1 #44918172983 were starting at last poll.
- This follow-up retains the prior `nvram_get_int("wins_enable")` nonzero-integer semantics rather than conflating it with flags whose legacy implementation was exact string matching.


## Active task

1. Inspect exact-head DNSMasq integer WINS policy, Samba and XY-C1 Actions for this compatibility fix.
2. Inspect storage hotplug checker result for a27d1c9b; fix any subsequent distinct failure.
3. Require Build XY-C1, TRX and artifact upload to PASS on the same exact product-source SHA before marking L3.
4. Continue broader userland reliability/security work without waiting for pending actions.


## Verification boundary

- L1: DHCP WINS option preserves nonzero integer semantics of the former `nvram_get_int()` while checked backend errors abort before preparation side effects; absent key remains 0.
- L2: Integer-boolean GNU89 regression and exact source contract are wired; results pending. Other bd0a exact-head DNSMasq/Samba/HTTPD/WAN/Process checks passed.
- L3: NOT VERIFIED until Build XY-C1, TRX and artifact upload all pass on the same latest source SHA.
- L4: Physical router DHCP/WINS and Samba runtime NOT VERIFIED.


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

Fetch live main; inspect the exact-head numeric WINS regression, storage-hotplug check and XY-C1 Build/TRX/artifact results. Then continue the next independent whole-userland stability issue.
Fetch live main; inspect exact-head storage-hotplug, XY-C1 Build/TRX/artifact and other affected Actions for the checked-mode wrapper contract correction. Then continue the next independent whole-userland stability issue.
Fetch live main; inspect the exact-head DNSMasq WINS source-order regression, Samba regression and XY-C1 Build/TRX/artifact runs for this patch. Then continue the next independent whole-userland stability issue.
Fetch live main; inspect the fresh Samba run from the strengthened WINS-header contract and XY-C1 #44917653757 (Build/TRX/artifact) against the complete restored tree. Then assess the independent DNSMasq WINS read and continue broader userland reliability work.
Fetch live main; inspect exact-head Samba source-contract and XY-C1 Build/TRX/artifact runs for this WINS-header patch. Then assess the independent DNSMasq DHCP WINS NVRAM read and continue the next userland reliability issue.
Fetch live `main`; inspect exact-head WAN callback, WAN route, AI handoff and XY-C1 Build/TRX/artifact runs after THIS COMMIT. Then continue the Samba checked-NVRAM fail-closed implementation while any firmware Actions are pending.
