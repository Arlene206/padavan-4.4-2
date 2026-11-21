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

- Main at the start of this DNSMasq slice: e9ce710c6acc5138400faf6238d85fcd0197ce23. The patch reuses the existing checked optional-NVRAM reader for the DHCP WINS option.
- The preceding complete-tree recovery restored all repository paths; tests/samba_config_file.c, README.md, services_stor.c and the top-level firmware build workflow were verified readable. Do not repeat the recovery audit.
- Samba share config exact-head regression #44917799137 passed on e9ce710; it includes GNU89 compile/policy tests and the source-contract step.
- XY-C1 recovery run #44917653757 on 954d1dc was still in progress at last poll, with Build XY-C1 active and TRX/artifact pending. That run does not include this new DNSMasq code.


## Active task

1. Inspect exact-head DNSMasq/Samba and XY-C1 push Actions for the checked DHCP WINS patch.
2. Require Build XY-C1, TRX verification and artifact upload to PASS on the same product-source SHA before marking L3 verified.
3. Continue the next independent stability issue elsewhere in the Padavan userland; do not wait for pending runs.


## Verification boundary

- L1: DHCP option 44 now consumes the checked wins_enable result only when DHCP is active and no explicit WINS address is configured; backend errors abort before generated input or staged-config changes; missing key stays disabled (0).
- L2: Existing dnsmasq_nvram_option_policy tests cover missing defaults/backend errors/output preservation; new source-order contract is wired but exact-head result is pending.
- L3: NOT VERIFIED until Build XY-C1, TRX verification and artifact upload PASS on one exact SHA.
- L4: Physical router DHCP client lease, WINS resolution and live DNSMasq restart behavior NOT VERIFIED.


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

Fetch live main; inspect the exact-head DNSMasq WINS source-order regression, Samba regression and XY-C1 Build/TRX/artifact runs for this patch. Then continue the next independent whole-userland stability issue.
Fetch live main; inspect the fresh Samba run from the strengthened WINS-header contract and XY-C1 #44917653757 (Build/TRX/artifact) against the complete restored tree. Then assess the independent DNSMasq WINS read and continue broader userland reliability work.
Fetch live main; inspect exact-head Samba source-contract and XY-C1 Build/TRX/artifact runs for this WINS-header patch. Then assess the independent DNSMasq DHCP WINS NVRAM read and continue the next userland reliability issue.
Fetch live `main`; inspect exact-head WAN callback, WAN route, AI handoff and XY-C1 Build/TRX/artifact runs after THIS COMMIT. Then continue the Samba checked-NVRAM fail-closed implementation while any firmware Actions are pending.
