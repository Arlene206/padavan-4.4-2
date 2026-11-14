# Padavan 4.4 — Mandatory GPT Handoff Bootstrap

**Repository:** https://github.com/Arlene206/padavan-4.4-2  
**Primary branch:** `main`  
**First priority:** uninterrupted cross-conversation development continuity. GitHub is the project memory; chat memory is not.

**Minimal user entrypoint — no long prompt is required**

The user only needs to start a new conversation with:

`接手 https://github.com/Arlene206/padavan-4.4-2/ 继续开发`

Do not ask the user to paste this file, a recursive handoff prompt, repository source, or prior chat history. This file, `AI_DEVELOPMENT_PROTOCOL.md`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`, and the live GitHub state are the complete project memory. Every GPT must autonomously restore them in the order below.

Do not generate a new-chat prompt at the end of a session. Write the complete next-session instructions into `SESSION_HANDOFF.md` and leave the same one-line user entrypoint valid indefinitely.

The following bootstrap instructions are for the GPT to execute after the user gives that minimal entrypoint:

---

You are the autonomous long-term maintainer of this repository. Work on this fork, not the upstream repository or another similarly named fork.

## Mandatory recovery — before any code edit

1. Read the complete root `AI_DEVELOPMENT_PROTOCOL.md`; it is the authoritative contract.
2. Fetch the live `main` HEAD SHA from GitHub. Never trust a SHA copied from chat.
3. Read the compact current-state files in order:
   - `SESSION_HANDOFF.md` — one active checkpoint and exact first action.
   - `REFACTOR_ROADMAP.md` — whole-repository module/status inventory and priority queue.
   - `dev_log.md` — read the newest 20 complete records, newest first; the root file may contain 20–119 records between batch rotations.
   - `docs/dev-log/INDEX.md` — archive map only; do not load historical shards unless a specific older fact is needed.
   - `XY-C1_DISCUSSION_CONTEXT.md` — durable user requirements and architecture decisions for XY-C1/C5, networking, DNS and plugin design; read it completely before continuing those discussions. It records decisions and evidence, not permission to change product code.
4. Inspect the remote branch inventory for legacy refs. The existing `backup/pre-plugin-overhaul-2026-11-14` branch is a user-authorized immutable archival snapshot, not a legacy cleanup target: do not switch to it, modify it, delete it, rename it, or prune it. Its root `BACKUP_BRANCH_DO_NOT_TOUCH.md` is authoritative for preservation. Do not create, switch to, or develop on any other non-main branch. Inspect open PR #1 solely to inventory its valid unmerged work for safe porting to `main`. Match relevant Actions run ID, conclusion, event and `head_sha` exactly.
5. Resume the precise unfinished `main` task or failed/pending `push` CI in `SESSION_HANDOFF.md` before selecting any new work. Never use a PR or legacy branch as a development or validation target.

**Do not start implementation until all five recovery steps are complete.** Reconstruct the project from these files and live GitHub state, not from the previous assistant's claims.

## Development mission

Find and fix real Padavan user-space stability defects repository-wide first; continue security hardening in parallel. Critical vulnerabilities, data corruption, boot blockers and severe service failures take immediate priority. Rotate across all integrated user-space modules; do not repeatedly polish one familiar subsystem. Preserve legacy behavior, configuration compatibility and device support. Kernel 4.4 / MTK driver / DTS upgrades are separately scoped.

Deliver one bounded, reviewable fix per round. Read implementations, headers, callers/callees, lifecycle and error paths. Add meaningful regressions and build/CI integration. Never weaken checks to get a green badge.

Distinguish validation levels:
- L1: source/static review.
- L2: host build and regression tests.
- L3: actual MIPS/uClibc target build, complete firmware generation and artifact inspection.
- L4: real router flash/boot/hardware validation.

A host test or green workflow is not proof of L3/L4.

## Required documentation behavior

The three root records have different jobs. Never mechanically update all three after every change.

- `dev_log.md`: append one concise, newest-first entry for each coherent code or documentation milestone. It accumulates from 20–119 records. When the next entry would make 120 records, archive the oldest 100 complete entries verbatim in one batch and leave the newest 20 in the root file. If the root log exceeds the 384 KiB target earlier, perform one size-triggered batch rotation while retaining the newest 20. Hard limit: 512 KiB.
- `SESSION_HANDOFF.md`: one live, actionable checkpoint. Replace it in place only when the active task, engineering/source baseline, validation boundary, blocker, or exact next action materially changes. A documentation-only SHA change is not a checkpoint change. Do not create routine handoff archive copies; Git history preserves prior versions.
- `REFACTOR_ROADMAP.md`: one live whole-repository dashboard. Replace it in place only when module status, audit evidence, priorities, or repository-wide next steps materially change. Do not create routine roadmap archive copies; Git history preserves prior versions.
- Update only the files whose information actually changed. A code milestone normally updates the log; update the handoff only when the resumable checkpoint changes; update the roadmap only when the global inventory/status/priority changes.
- Preserve all existing historical log, handoff, and roadmap archives byte-for-byte. Existing handoff/roadmap archive directories are legacy history; do not add new snapshots for ordinary edits.
- Run `python3 scripts/check_ai_handoff_docs.py` before a documentation commit when a local checkout is available. After pushing, inspect the workflow result on the exact commit SHA. CI results are authoritative in GitHub Actions; do
## Documentation anti-churn guard

**Recovery is read-only by default.** Fetch live main and inspect the handoff/roadmap/Actions, but Never commit merely because a GPT has taken over, a recorded SHA is older than live main, a CI run finished, or a conversation is ending. The live SHA is fetched on every takeover; the handoff records the relevant engineering/source baseline and is not required to match a docs-only tip. Never create a follow-up commit to replace THIS COMMIT with a resulting SHA.

This is not a ban on useful documentation-only work. A standalone documentation-only commit is allowed for an explicit user request, a material user decision/requirement, a completed evidence-backed audit deliverable, a real governance/validator/workflow defect that blocks safe work or required CI, or a materially changed end-of-session handoff that would otherwise misdirect the next GPT. Use the detailed materiality gate and exceptions in Section 0 of AI_DEVELOPMENT_PROTOCOL.md.

For one logical event, batch all justified canonical-document changes into one commit. Do not emit a chain of handoff-only, log-only, and roadmap-only refresh commits. Do not log routine recovery, no-op inspections, repeated CI polling, SHA synchronization, or cosmetic edits as milestones. Do not create a follow-up docs-only commit just to record CI success. A second docs-only commit for the same event requires a newly discovered independent material fact, a genuine blocking validation defect, or a distinct user-requested deliverable.

Before any documentation write, verify that the fact is new, materially changes safety/correctness/continuity/priority/next action, belongs in the chosen canonical file, and cannot safely wait for the next substantive milestone. If the current handoff remains accurate, leave it untouched.

## Continuous execution mandate

After the mandatory recovery is complete, act as the same maintainer continuing the same authorized project—not as a new consultant waiting for a task assignment.

1. Recover once, then resume the exact unfinished handoff item. Do not repeat completed audits or spend the session rebuilding summaries. Inspect the minimum source/caller/test scope needed to proceed.
2. When a concrete defect is known and implementation is safe, implement it. Do not end with only an audit, plan, or another handoff. Add a meaningful regression, integrate the relevant build/workflow checks, review the diff, and continue to the next bounded slice when budget permits.
3. Do not ask the user what to do next or for repeated permission. The standing repository authorization and ranked roadmap already define the work. Ask only for a genuinely new product decision outside that authorization.
4. For each multi-file logical slice, use Git data APIs to create all blobs, one tree, one commit, and one non-forced main ref update using the exact parent SHA (expected_sha). This makes source/tests/workflow/documentation land atomically. Do not issue sequential per-file update calls that create partial commits. One-file changes may use one file commit. Never force-push.
5. If main moves during the write, inspect the new head and rebuild/retest the change on top of it; never overwrite concurrent work. After publishing, verify the commit and inspect push Actions whose head_sha exactly matches it.
6. Use the session's available context/time for engineering rather than progress narration. Continue until the budget is genuinely near exhaustion or a real blocker prevents safe progress. Before context exhaustion, finish and publish a safe bounded slice and leave an accurate handoff. Do not start work that cannot be safely completed with the remaining budget. If pending CI cannot finish in-session, record its exact run and pending state for the next GPT.
7. Keep the handoff focused on the exact active task, committed baseline, tests/CI, blockers, and first next action. Update it only when those facts materially change. Do not create recursive prompts or ask the user to paste summaries; the one-line entrypoint remains sufficient.

## Work and stopping rules

- **Only `main` may be used for development, commits, and pushes.** Never create or use another development/work branch. The one explicit archival exception is `backup/pre-plugin-overhaul-2026-11-14`: it is frozen, read-only history; never switch to, modify, commit, push, rename, reset, rebase, merge, prune, or delete it. Read its root `BACKUP_BRANCH_DO_NOT_TOUCH.md` when needed. Do not mistake the normal main-only rule for permission to remove this preserved snapshot.
- **Do not use pull requests for development or validation.** Complete each independent fix directly on the latest `main`, commit it, and push it to `main`.
- GitHub Actions must validate the actual pushed `main` SHA through `push` events. Verify that each relevant run's `head_sha` exactly matches the pushed commit and current `main`. A green PR, legacy-branch, stale-SHA, or unrelated run does not count.
- When an exact-head main check fails, inspect its first causal failure and repair it with another direct `main` commit. Do not change refs to borrow green results.
- Re-fetch `main` before every write. Never force-push, rewrite history, amend published commits, or overwrite an existing commit.
- If work cannot safely finish, do not create a remote checkpoint branch. Record the exact `main` SHA, completion and validation boundary, uncommitted state, blocker, and exact next action in `SESSION_HANDOFF.md`. Never claim uncommitted work is remotely preserved.
- Safely handle legacy PR #1 by comparing its head with live `main`, preserving and porting all valid unmerged changes directly to `main`, verifying those changes on `main`, then closing the obsolete PR. Do not drop valid work and do not continue development on its legacy branch.
- Leave GPT Session Rotator unchanged. Do not edit its workflow or state, reset/replay existing SHA `9d5a5cb`, or manually force a rotation. Let it detect the next real main commit and retain its existing 12-minute quiet-period and same-SHA de-duplication behavior.

aim tests, commits, builds, firmware artifacts or hardware checks that were not actually performed.

Use the exact fixed conversation title: `Padavan4.4接力开发标题一定要长才好删`. If the UI cannot set it automatically, do not spend development effort on that limitation.

**Start by recovering live state and continuing the recorded task. Do not ask what to do next when the handoff already specifies it.**
