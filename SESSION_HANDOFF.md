# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into partial commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`. Parent for PPP control argv conversion: `71c95adc3e99b611ddfdb9f4b67014718ed4916a`.
- `net6.c` per-interface IPv6 flushes, all `net_wan6.c` SIT/6rd/native route operations, and `net_wan.c` dynamic PPP/WAN hook commands use argv-based `eval()`.
- `net_ppp.c` now invokes the fixed L2TP control command and mwan lifecycle script with argv-based `eval()`, preserving the exact command argument/lock flag and existing status checks. There are no remaining `system()` or `doSystem()` calls in this module. The shared checker now guards this.
- The earlier PPP ping checker correction is committed on parent `71c95adc3e99b611ddfdb9f4b67014718ed4916a`; that commit's exact-head runs were in progress when last checked. This commit's focused net_ppp source contracts pass locally.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Active task

Verify exact-head actions for this PPP control argv slice; inspect the first causal failure, especially WAN route, Firewall, Process and XY-C1. Continue adjacent PPP startup/stop lifecycle review while independent workflows are pending.

## Verification boundary

- No full target build was performed locally. Exact-head push Actions for this change are pending.
- Product-source MIPS/uClibc XY-C1 L3 (Build, TRX verification and artifact upload on one SHA) and physical-router L4 remain **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch `/actions/runs?head_sha=<current main SHA>`; verify the PPP source checker and workflows. Fix any first causal regression from its log, then re-poll.
2. Continue PPP lifecycle status review, including L2TP daemon and server-start interactions; preserve callback generation fencing, lifecycle locks, and failure propagation.
3. Keep the frozen backup unchanged; use complete one-tree atomic commits and non-forced expected-SHA updates.
