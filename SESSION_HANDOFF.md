## 2026-11-28 continuation checkpoint

- Parent live `main` for this DHCPv6 slice: `02dfbf9198cd7b5de10c4c902501249683ef26ef`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; verified unchanged before publication.
- OpenVPN bounded-fsync slice is committed at parent SHA `02dfbf9198cd7b5de10c4c902501249683ef26ef`; its exact-head OpenVPN, Process, mdev, USB modem, NVRAM, DHCPv6 IA, AI docs, Process inventory, Samba, MSD Lite and XY-C1 Actions were queued at checkpoint. They must be inspected by exact SHA, not waited on.
- Review found the RC DHCPv6 IA configuration writer still had its own unbounded `fsync()` EINTR loop, despite including the shared `config_file_sync.h`. The current slice removes that duplicate adapter, routes file sync through the shared eight-retry helper, and adds persistent-EINTR regression coverage that verifies no rename occurs and the last-good config remains active.
- NVRAM transaction-boundary and AI documentation checks on the preceding SHA `ebf6e9ab...` were COMPLETED/SUCCESS. No full firmware build or physical-router validation is claimed.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; only `main` is authorized for development.
- DHCPv6 config durability slice is parented to `02dfbf9198cd7b5de10c4c902501249683ef26ef`; resolve the resulting SHA from the live main ref.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. PR #1 remains closed and unmerged.
- Recovery/audit completed once. Do not repeat it. `REFACTOR_ROADMAP.md` remains unchanged due its near-hard-cap size.
- Parent NVRAM and AI docs checks passed on `ebf6e9ab...`; exact checks for the OpenVPN and DHCPv6 source slices are required. No local full firmware build or physical-router validation has been run.

## Active task

- Reuse the shared bounded fsync EINTR policy for DHCPv6 IA config staging.
- Before this slice, `dhcp6c_config_file_publish()` used a duplicate `do/while(errno == EINTR)` loop with no interruption ceiling while `config_file_sync.h` already supplied an injectable, bounded policy.
- This slice removes the duplicate fsync macro and loop, calls `config_file_sync_fd(fileno(stream))`, adapts the existing host regression to inject through the shared macro, and verifies persistent EINTR fails after nine total attempts before rename, removes the stage, and preserves the active config.
- Changed files: `dhcp6c_config_file.h`, `tests/dhcp6c_config_file.c`, `.github/workflows/dhcp6c-config-publication.yml`, `SESSION_HANDOFF.md`, `dev_log.md`.

## Exact first action for the next GPT

1. Fetch live `main` and confirm the exact SHA of the DHCPv6 bounded-fsync slice and the immutable backup SHA.
2. Inspect exact-SHA `DHCPv6 client configuration durability regression`, `OpenVPN client credential staging regression`, `AI handoff documentation contract`, and any other exact-head Actions; confirm the new persistent-EINTR test and shared source contract pass.
3. On failure, fetch only the failed exact-SHA job logs and repair the causal issue in a separate atomic commit with expected-SHA, non-force update.
4. After PASS, continue another independent P1 stability/security defect from the existing roadmap; do not re-audit completed startup work and do not wait on unrelated PENDING runs.
5. XY-C1 MIPS/TRX/artifact remains NOT VERIFIED until a complete exact-source target run passes; physical L4 remains NOT VERIFIED.

## Verification boundary

- L1: DHCPv6 IA config writer now uses the common bounded fsync policy already used by OpenVPN publication; max eight EINTR retries after initial attempt.
- L2: Regression covers two transient EINTR returns followed by success, persistent EINTR exhaustion at nine total calls, stage cleanup and preservation of the prior active file, plus existing EIO/rename/parent-sync/symlink cases. Exact-head CI is pending.
- L3: NOT VERIFIED; this session has not run a complete XY-C1 MIPS Build, TRX check and artifact upload for the new source.
- L4: NOT VERIFIED; no physical router validation was performed.
- The frozen backup remains unchanged; parent NVRAM and AI docs checks passed.

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
