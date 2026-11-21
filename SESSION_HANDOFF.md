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

- Current main at handoff preparation: 954d1dc4d11e5518e6e69e86a167fcfd0e5efd33. The WINS-header source patch is present and the repository's complete base tree has been restored.
- Important recovery: commit b53d27269e6de0a3b56f51e044714f7a4fa91dd1 accidentally had a tree containing only five changed paths because its create-tree call lacked the full base tree SHA. Its exact-head Samba build failed on missing tests/samba_config_file.c. Commit 954d1dc4d11e5518e6e69e86a167fcfd0e5efd33 is a normal non-forced child that re-applies the same five updates on the full base tree 12f4a8fe707916dc3fb2550850642b1c9c3b5dae; key repository files are restored and accessible.
- Parent code validation: Samba #44917367040 and AI handoff #44917367092 PASS on bd63d609; XY-C1 #44917367085 completed host-only PASS but Build XY-C1, TRX and artifact upload were SKIPPED because the input detector found no firmware production changes.
- On the recovery SHA, XY-C1 #44917653757 was queued at last poll. Samba's b53 job #44917536389 failed at the initial compile solely because the tree omitted existing test files; the test sources are now restored. This patch strengthens the header error-before-write contract and triggers a fresh Samba regression.



## Active task

1. Inspect exact-head Samba and XY-C1 push runs for the restored full-tree source. Verify the Samba GNU89 policy/config binaries and source contracts now run on the complete repository.
2. Require Build XY-C1, TRX verification and artifact upload to PASS on one exact product-source SHA before claiming L3; do not treat host-only SKIP as a firmware build.
3. Assess the separate unchecked nvram_get_int("wins_enable") use in DNSMasq DHCP-option generation in services_ex.c. Establish a safe failure/default policy before changing it.
4. Continue other independent userland reliability issues while Actions are pending.


## Verification boundary

- L1: Samba run/restart/selective stop/WINS start and global smb.conf header paths use checked enable/WINS state. The header returns before output if the WINS flag read is unavailable or malformed.
- L2: Parent code run #44917367040 passed existing GNU89 policy/config binaries and source contracts; the first source-patch run on b53 failed before compilation because the commit tree omitted existing test files. Full tree restored by 954d1dc4; fresh Samba rerun is pending.
- L3: NOT VERIFIED. XY-C1 #44917653757 is the exact-recovery-SHA run; only mark L3 after Build XY-C1, TRX and artifact upload all PASS on one SHA.
- L4: Physical router flash/boot, Samba authentication/share behavior and runtime WINS transitions NOT VERIFIED.


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

Fetch live main; inspect the fresh Samba run from the strengthened WINS-header contract and XY-C1 #44917653757 (Build/TRX/artifact) against the complete restored tree. Then assess the independent DNSMasq WINS read and continue broader userland reliability work.
Fetch live main; inspect exact-head Samba source-contract and XY-C1 Build/TRX/artifact runs for this WINS-header patch. Then assess the independent DNSMasq DHCP WINS NVRAM read and continue the next userland reliability issue.
Fetch live `main`; inspect exact-head WAN callback, WAN route, AI handoff and XY-C1 Build/TRX/artifact runs after THIS COMMIT. Then continue the Samba checked-NVRAM fail-closed implementation while any firmware Actions are pending.
