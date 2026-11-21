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

- Product-source SHA at the start of this slice: bd63d60932e5c1d7a90cbe9a19a3d8a0c72c153c; this patch adds a checked WINS NVRAM gate to smb.conf header generation before any file output.
- Exact-head Samba regression #44917367040 passed GNU89 policy/config binaries and all existing source contracts. AI handoff docs #44917367092 passed; XY-C1 #44917367085 was still in progress at the last poll.
- This patch adds a static forward declaration for the checked WINS adapter, uses its legacy missing-key default 0 in write_smb_conf_header(), and adds source contracts for declaration/call order and absence of the unchecked read.



## Active task

1. Inspect exact-head Samba and XY-C1 push Actions for this patch. Correct real source or source-contract failures without weakening coverage.
2. Require Build XY-C1, TRX verification and artifact upload to PASS on one exact product-source SHA before marking L3 verified.
3. Assess the separate remaining unchecked nvram_get_int("wins_enable") use in DNSMasq DHCP-option generation in services_ex.c; define an explicit safe error policy before changing that independent caller.
4. Continue other independent reliability issues in the userland while Actions are pending; do not wait idly.


## Verification boundary

- L1: run_samba(), restart_smbd(), selective Samba stop and WINS startup already use checked NVRAM reads. This patch extends fail-closed behavior to global smb.conf header generation, before any file output.
- L2: parent Samba regression #44917367040 and AI handoff #44917367092 PASS. The checked-header source contract and exact-head regression for this patch are pending.
- L3: NOT VERIFIED until Build XY-C1, TRX verification and artifact upload PASS on the same source SHA.
- L4: physical router flash/boot, Samba authentication/share behavior and runtime WINS transitions NOT VERIFIED.


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

Fetch live main; inspect exact-head Samba source-contract and XY-C1 Build/TRX/artifact runs for this WINS-header patch. Then assess the independent DNSMasq DHCP WINS NVRAM read and continue the next userland reliability issue.
Fetch live `main`; inspect exact-head WAN callback, WAN route, AI handoff and XY-C1 Build/TRX/artifact runs after THIS COMMIT. Then continue the Samba checked-NVRAM fail-closed implementation while any firmware Actions are pending.
