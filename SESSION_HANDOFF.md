# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; develop on `main` only.
- Main at the start of this slice: `6882aefa2c70b7bfc53da5e770610dc45dc802ba`. The WAN DHCP NVRAM state publisher separates publication failure from compensation failure; its guarded fresh-transaction retry runs only when compensation itself failed.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Never modify, checkout, merge, reset, rename, prune or delete it. Only main and backup branches are authorized.
- Legacy PR #1 remains closed and unmerged; do not reopen or develop through its branch.
- Protected reliability work includes checked optional-daemon settings, ttyd/SSH mode wrappers and detached-child probes, WAN DHCP compare-and-publish, stale-writer guarded recovery and validate-first reverse tuple construction for post-commit 6rd failure.

### Relevant exact-SHA evidence

- Production-source baseline: `6882aefa2c70b7bfc53da5e770610dc45dc802ba`. It separates candidate publication failure from rollback-compensation failure. A new guarded restore is attempted only when compensation failed or the transaction close failed; successful in-transaction compensation is not redundantly replayed.
- WAN callback #44915375540: FAILURE at an erroneous source-count assertion for a literal escaped signature string. The preceding publisher-signature contract already checked the one forward declaration and unique definition; this redundant assertion is corrected in THIS COMMIT.
- WAN route #44915375609: FAILURE in `scripts/check_wan_firewall_status.py` because it expected the old `publish_attempted && status != 0` retry predicate. XY-C1 pre-build run #44915375738 failed at the same stale firewall checker before compiling firmware. The checker now requires the separated rollback status and guarded retry predicate.
- XY-C1 #44915375738 did not reach Build XY-C1; TRX verification and artifact upload did not run. The older XY-C1 #44914789805 on `2bfb2aa` had also been pending at an earlier poll and is not evidence for current source. L3 remains NOT VERIFIED.
- NVRAM transaction audit #44915375601: SUCCESS on exact `6882aefa`.
- Process lifecycle #44915375502 and process inventory #44915375496: SUCCESS on exact `6882aefa`.
- AI handoff documentation #44915375633: SUCCESS on exact `6882aefa`.
- WAN teardown #44915375932, watchdog #44915375670, DNSMasq #44915375677, detect_link #44915375582, USB modem #44915375668, miniupnpd #44915375558, Firewall #44915375647, WAN gateway ARP #44915375686, Wireless #44915375605, VPN #44915375644, router-mode NVRAM #44915375549, detect_internet #44915375619 and ZeroTier #44915375770: SUCCESS on exact `6882aefa`.
- Exact-head reruns for WAN callback, WAN route and XY-C1 are required after THIS COMMIT. Hardware L4 remains NOT VERIFIED.


## Active task

1. Inspect exact-head WAN callback, WAN route, AI handoff and XY-C1 push Actions after THIS COMMIT; diagnose the first failing step.
2. Require XY-C1 Build, TRX verification and artifact upload to PASS on the same product-source SHA before claiming L3.
3. Continue the Samba NVRAM fail-closed task while firmware Actions are pending: check enable_samba/st_samba_mode before publication and restart, and check wins_enable before selective nmbd stop.

## Verification boundary

- L1: WAN lease tuple publication remains compare-and-publish under the NVRAM transaction; stale tuple conflicts return `EAGAIN`; recovery is fenced against a newer writer and retries only after failed compensation.
- L2: NVRAM audit, Process lifecycle/inventory and AI handoff PASS on `6882aefa`. WAN callback, WAN route and the XY-C1 pre-build checker failed due to stale source-contract assertions; corrected in THIS COMMIT and require exact-head reruns.
- L3: NOT VERIFIED. The current XY-C1 run failed before firmware compilation; require Build + TRX + artifact upload to pass on one same SHA.
- L4: physical router flash, boot and runtime validation NOT VERIFIED.

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

Fetch live `main`; inspect exact-head WAN callback, WAN route, AI handoff and XY-C1 Build/TRX/artifact runs after THIS COMMIT. Then continue the Samba checked-NVRAM fail-closed implementation while any firmware Actions are pending.
