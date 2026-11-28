## 2026-11-28 continuation checkpoint

- Parent live `main` for this slice: `ebf6e9abf641b18fe19c359e17553c40ad6f1e79`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; it was rechecked unchanged before this write.
- Previous exact-head NVRAM transaction-boundary run #46129727106 on `ebf6e9abf641b18fe19c359e17553c40ad6f1e79`: COMPLETED/SUCCESS, including source audit step 18. AI handoff documentation run #46129727111 on the same SHA: COMPLETED/SUCCESS.
- OpenVPN config publication review found three unbounded EINTR retry loops in one durability path: shared `config_file_sync_fd()` and two direct staged-file `fsync()` loops. A persistent signal storm could keep a service process stuck indefinitely. The current slice applies one shared eight-retry budget to both staged-file fsyncs and the containing-directory fsync, and adds deterministic injected EINTR/exhaustion tests plus a source contract.
- This is an OpenVPN publication-stability slice; it does not alter the frozen backup, create a branch or PR, or claim a firmware build. Exact-head OpenVPN regression and AI docs checks must be read after publication.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; only `main` is authorized for development.
- This OpenVPN fsync retry slice is parented to `ebf6e9abf641b18fe19c359e17553c40ad6f1e79`; fetch the live `main` ref to resolve the resulting commit SHA.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. PR #1 remains closed and unmerged.
- Startup protocol/handoff recovery was completed once. Do not repeat completed governance audits or restore the deliberately skipped XY-C1 discussion file.
- NVRAM exact-head source audit and AI docs passed on parent SHA `ebf6e9ab...`. No local full firmware build or physical-router validation has been performed.

## Active task

- Bound all OpenVPN config durability fsync retries to a shared eight-interruption budget.
- Before this slice, `config_file_sync_fd()` retried every EINTR forever; `openvpn_config_file_publish()` duplicated that unbounded loop for both staged-file fsyncs and used the shared helper only for the post-rename directory fsync.
- The current atomic change routes both staged-file sync points through the shared helper, caps retries at eight after the initial call, and adds GNU89/ASan/UBSan regression coverage for successful EINTR recovery and persistent-signal exhaustion.
- Changed files: `config_file_sync.h`, `openvpn_config_file.h`, `tests/openvpn_config_file.c`, `scripts/check_openvpn_client_secret.py`, `SESSION_HANDOFF.md`, `dev_log.md`.

## Exact first action for the next GPT

1. Fetch live `main` and confirm the exact SHA produced by this commit; verify frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
2. Inspect exact-SHA push Actions for `OpenVPN client credential staging regression` and `AI handoff documentation contract`. Confirm the GNU89 ASan/UBSan OpenVPN config regression and source contracts pass.
3. If a job fails, use its exact job logs, fix the causal defect, and publish a separate atomic correction with expected-SHA non-force update.
4. After PASS, continue the highest-priority concrete stability/security defect from the existing roadmap without repeating closed audits. PENDING is not a reason to wait.
5. L3 requires a complete exact-source XY-C1 MIPS build/TRX/artifact pass; L4 requires a physical router. Neither is currently claimed.

## Verification boundary

- L1: source change routes all three OpenVPN configuration durability sync operations through one shared bounded EINTR helper; maximum is eight retries after the initial attempt.
- L2: regression now injects three EINTR returns across staged-file data sync, post-mode sync and parent-directory sync, and verifies persistent EINTR terminates after nine attempts with EINTR. Exact-head OpenVPN/AI Actions are required; no local compile was available.
- L3: NOT VERIFIED for this slice until exact-source XY-C1 MIPS Build, TRX validation and artifact upload pass.
- L4: NOT VERIFIED; no physical router validation was performed.
- Previous parent-SHA NVRAM and AI documentation checks passed (#46129727106 and #46129727111). Backup remains immutable.

## Mandatory handoff checkpoint

Every session MUST inspect the live repository state and compare it with this handoff before ending. The checkpoint is mandatory; a document edit is conditional. Confirm the active main SHA, immutable backup, resumable task, exact-head Actions, validation boundary and precise first next action. If all facts are already accurate, conclude the checkpoint without a no-op commit.

# Cross-session execution mandate

- Continuous execution across GPT conversations: treat this repository as one ongoing engineering engagement, not a fresh analysis exercise each session.
- Recover once, then execute. Do not repeat completed governance audits or spend the session summarizing already closed work.
- An audit is a means, not the default endpoint. When a concrete defect is confirmed, change production source, add a regression, run the relevant checks and continue.
- One logical slice, one atomic commit. For multi-file changes use Git data APIs to create all blobs, one tree and one commit; publish one atomic main commit using non-forced compare-and-swap. Never publish per-file intermediate states or force-push.
- Inspect GitHub Actions for the exact commit SHA. PASS permits the next slice; FAIL requires causal repair; PENDING permits independent work.
- Checkpoint before context exhaustion. Before context exhaustion, finish the current safe slice, commit it, verify its exact-SHA state and update this handoff with the accurate first next action.
- Do not ask the user what to do next or request repeated authorization. Preserve safety, architecture, compatibility and necessary tests.
