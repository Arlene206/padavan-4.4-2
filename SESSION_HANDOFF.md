# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; develop on `main` only.
- Main at the start of this slice: `6882aefa2c70b7bfc53da5e770610dc45dc802ba`. The WAN DHCP NVRAM state publisher separates publication failure from compensation failure; its guarded fresh-transaction retry runs only when compensation itself failed.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Never modify, checkout, merge, reset, rename, prune or delete it. Only main and backup branches are authorized.
- Legacy PR #1 remains closed and unmerged; do not reopen or develop through its branch.
- Protected reliability work includes checked optional-daemon settings, ttyd/SSH mode wrappers and detached-child probes, WAN DHCP compare-and-publish, stale-writer guarded recovery and validate-first reverse tuple construction for post-commit 6rd failure.

### Current source slice

- Parent main SHA: `fbec957a7aa7ecb91ec538b7fa0ebe8acfd324af`; this commit implements fail-closed checked reads for Samba global enable, share mode, WINS stop policy and WINS startup.
- The mode policy validates booleans (only 0/1), share modes (0/1/3/4) and checked-read status. GNU89 tests cover backend errors, malformed values and disabled-global short-circuit.
- `run_samba()` validates runtime flags before inventory, passdb and configuration operations. `restart_smbd()` checks flags before stopping existing daemons and preflights WINS if that is the restart target. Non-forced stop refuses selective nmbd termination on failed WINS read. `start_wins()` refuses launch after failed WINS read.
- Parent `fbec957`: WAN callback #44915851413, WAN route #44915851347 and AI docs #44915851360 PASS. XY-C1 #44915851356 host-only PASS; Build/TRX/artifact were SKIPPED because that parent had no production-source changes. Current commit changes `services_stor.c` and `services_ex.c`, which XY-C1 watches; require same-SHA Build + TRX + artifact upload before L3.


## Active task

1. Inspect exact-head Samba share-stability, XY-C1, AI handoff and applicable Process/NVRAM push Actions after THIS COMMIT; diagnose the first causal failure.
2. Require XY-C1 Build, TRX verification and artifact upload to PASS on the same product-source SHA before claiming L3.
3. Continue the next unfinished storage/userland reliability issue while Actions are pending; do not repeat these Samba audits without fresh failure evidence.

## Verification boundary

- L1: WAN lease tuple publication remains compare-and-publish under the NVRAM transaction; stale tuple conflicts return `EAGAIN`; recovery is fenced against a newer writer and retries only after failed compensation. Samba service-state reads now fail closed on unavailable or malformed enable flags; exact-head verification is pending.
- L2: Parent `fbec957` WAN callback, WAN route and AI docs PASS. Its XY-C1 run passed host checks but skipped Build/TRX/artifact because no production source changed. Current Samba policy/unit/source contracts need exact-head PASS.
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
