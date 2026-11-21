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

- Latest product-source SHA: `bc3cceebfcc4423d03acd30a5604e01c63b13419`; Samba runtime decision points now fail closed on checked NVRAM read errors and invalid boolean/mode values.
- Samba policy/unit tests compiled and passed in exact-head run #44916836779 before the source-check step failed at a whitespace-sensitive assertion. The contract now normalizes whitespace in THIS COMMIT.
- XY-C1 run #44916836560 passed all host-side validation and was running the cross-build for this product-source SHA; TRX verification and artifact upload were pending. Do not claim L3 until all three steps pass.
- AI handoff #44916836461, Process inventory #44916836495, WAN route #44916836585 and DNSMasq #44916836494 PASS on `bc3cceebfcc4423d03acd30a5604e01c63b13419`; NVRAM audit #44916836473 was still running.


## Active task

1. Inspect exact-head Samba share-stability, XY-C1, AI handoff and applicable Process/NVRAM push Actions after THIS COMMIT; diagnose the first causal failure.
2. Require XY-C1 Build, TRX verification and artifact upload to PASS on the same product-source SHA before claiming L3.
3. Continue the next unfinished storage/userland reliability issue while Actions are pending; do not repeat these Samba audits without fresh failure evidence.

## Verification boundary

- L1: WAN lease tuple publication remains compare-and-publish under the NVRAM transaction; stale tuple conflicts return `EAGAIN`; recovery is fenced against a newer writer and retries only after failed compensation. Samba service-state reads now fail closed on unavailable or malformed enable flags; exact-head verification is pending.
- L2: Samba unit binary passed on `bc3cceeb`; its source-contract step failed only on the multiline runtime-reader assertion and is corrected in THIS COMMIT. AI handoff, Process inventory, WAN route and DNSMasq PASS on `bc3cceeb`; NVRAM audit was pending at the last poll.
- L3: NOT VERIFIED until this production-source SHA passes Build XY-C1, TRX verification and artifact upload on the same SHA.
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
