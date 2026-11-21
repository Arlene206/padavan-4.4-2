# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository Arlene206/padavan-4.4-2; develop on main only.
- Main at the start of this source-fix slice: 188a2b85bc0b80246bfc1a37f2030f7ceec85e29. This atomic commit restores the WAN DHCP state-publisher forward declaration and adds source-contract protection; fetch live main at takeover.
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14 remains immutable at 72f4f157a084f8184b9597ca5bf334b8080e5807. Never modify, checkout, merge, reset, rename, prune or delete it. Only main and backup branches are authorized.
- Legacy PR #1 is closed and unmerged. Do not reopen or develop through its branch.
- The previous product-source baseline was 9a25b71fb74932dc27063432b2f342d9724d60e2. Recent protected functionality includes checked optional-daemon settings, ttyd/SSH mode handling, WAN DHCP compare-and-publish and stale-callback rollback fencing.

### Relevant exact-SHA evidence

- XY-C1 run #44913834107 on product-source SHA 9a25b71: FAILURE in Build XY-C1. Compiler errors in net_wan.c: the forward declaration at line 6127 had been replaced by an extra full wan_dhcp_nvram_state_publish definition; the original implementation remained near line 7274. The duplicate caused undeclared transaction-set/restore helpers, an implicit restore declaration and function redefinition. This commit restores the declaration and retains the existing lower implementation. TRX and artifact upload were skipped on the failed parent SHA.
- WAN callback run #44914630325 on main SHA 188a2b85: FAILURE at the inline reverse-snapshot assertion because the production call spans multiple lines. This commit normalizes whitespace, fixes the stale rollback-guard assertion and asserts exactly one publisher definition plus its prototype.
- NVRAM transaction boundary audit #44913834104: SUCCESS on the prior product-source SHA 9a25b71.
- Process inventory/lifecycle regression #44914630350: IN_PROGRESS on SHA 188a2b85 at last inspection; inspect its final exact-head result and job log.
- AI handoff docs #44914630251: SUCCESS on SHA 188a2b85. The new handoff commit needs its own exact-head docs result.

## Active task

1. Inspect exact-head WAN callback, XY-C1, AI handoff and Process lifecycle Actions after THIS COMMIT. Fix the first causal failure without weakening checks.
2. Require XY-C1 Build, TRX verification and artifact upload to PASS on the same new product-source SHA before claiming L3.
3. After the build correction is validated, continue the next confirmed reliability issue. While Actions are pending, keep progressing on independent source/test work.

## Verification boundary

- L1: source review confirms the first WAN publisher is now a forward declaration and only the later implementation remains. The guarded stale-writer rollback logic is retained.
- L2: no exact-head Action result for this commit yet; WAN, Process and docs checks must rerun.
- L3: NOT VERIFIED. The parent XY-C1 build failed before TRX/artifact; the corrected same-SHA run must pass Build + TRX + upload.
- L4: physical router flash, boot and runtime validation NOT VERIFIED.

## Workflow and preservation rules

- Main only. One logical slice, one atomic commit: create all blobs, one tree based on current main, one commit with current main as parent, then non-forced update_ref using the exact expected_sha. No force push, amend, history rewrite, branches, PR-based development or partial per-file commits.
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

Fetch live main and inspect exact-head WAN callback, XY-C1 Build/TRX/artifact, Process lifecycle and AI handoff push runs. Diagnose failures from the first failing job step; continue independent reliability work if any run is pending.
