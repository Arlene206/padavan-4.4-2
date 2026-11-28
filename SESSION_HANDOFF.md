# Session Handoff — 2026-11-28

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; only `main` is authorized for development.
- Parent main SHA for this atomic lock-wait slice: `0d757a438cd424c600be87724b65b473012310c6`. The handoff, code, test, checker and log are committed together; resolve the resulting SHA from live `main` on the next recovery.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. PR #1 remains closed. No work branches or new PRs.
- Bootstrap/protocol/handoff/roadmap/dev-log recovery was completed once; do not repeat the completed audit. `REFACTOR_ROADMAP.md` remains unchanged and near its 128 KiB limit.
- No local full firmware build or physical-router validation is available.

## Completed engineering

- NVRAM device descriptor inheritance: `d8c0568b30c07a5d18d4eca786d1ed53a9d5ef41` switched all five shared raw ioctl opens to `O_CLOEXEC`. Exact descendant NVRAM audit #46127680492 and Process fail-closed #46127680468 passed on source SHA `7b56260076b8d05b521b3caf3f90afffc65642dc`.
- MSD Lite staged-config durability: `7b56260076b8d05b521b3caf3f90afffc65642dc` adds EINTR-safe file sync and parent-directory sync after rename, with EINTR/EIO fault-injection tests. Dedicated MSD Lite publication #46127680489, USB storage #46127680481, NVRAM #46127680492 and Process fail-closed #46127680468 passed; Process inventory #46127680472 passed.
- Shared config durability: Samba, DNSMasq, L2TP/Ralink, USB modem node metadata and QMI credential writers use EINTR-safe sync helpers and host regressions. Samba stale checker and header-inline issues were repaired in `cab010ea3ddefa54cfc95ce641d89dd80f8f24bb` and `385dd12da8e6f6f968d9633b82d0575ee34a2370`.
- NFS, AliDDNS and VPN wait/restart interruption hardening is documented in `3a910eecae543282b40474703c75b778323b5bc4`, `2ea79593ab9428f7439429a4d9b1e58bdb8077ea` and the subsequent VPN wait fixes. Their associated exact-SHA host regressions passed as recorded in the preceding handoff checkpoint.
- AI handoff contract restoration: exact run #46127933674 on `0d757a438cd424c600be87724b65b473012310c6` PASS, including the explicit mandatory checkpoint, cross-session execution mandate and required section headings.

- Service lifecycle wait interruption fix: `f5cd59f4e98e2b46a256acfeb39d0b89d8fb9639` changed Inadyn/USB-printer stop waits to propagate the existing EINTR-aware bounded wait policy and prevented forced KILL after a failed wait. Exact Process inventory/lifecycle #46128356507, Inadyn #46128356513, Process source inventory #46128356524 and AI handoff #46128356516 passed; XY-C1 #46128356573 was still in progress at last poll. NVRAM #46128356589 was cancelled by subsequent concurrent pushes.
## Active task

- Process inventory procfs-directory interruption handling. File reads already retry EINTR, but `opendir("/proc")` and each `readdir()` were called once; an interrupted open/getdents could turn a complete process scan into UNKNOWN.
- This atomic slice adds bounded retries around both directory-open and directory-read operations using the existing `PROCESS_IO_MAX_INTERRUPTS` policy. Persistent signal storms still fail closed with EINTR.
- New injected regressions cover retry success and retry-budget exhaustion for each operation. The process lifecycle integration checker pins both source paths and the test cases.
- Exact-head Process inventory/lifecycle and XY-C1 validation are pending; no local full firmware build or hardware result is claimed.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-SHA Actions for this procfs retry commit: Process inventory/lifecycle, Process source inventory, AI handoff and XY-C1. Do not substitute parent-SHA results.
2. If the inventory workflow fails, read job logs and repair the root cause. Confirm injected EINTR retry-success and bounded-exhaustion tests executed.
3. Refresh the previous service-lifecycle fix `f5cd59f4e98e2b46a256acfeb39d0b89d8fb9639` results if needed; its Process inventory/lifecycle, Inadyn, source inventory and AI contract checks had passed, while XY-C1 was still in progress and NVRAM was cancelled.
4. Require XY-C1 MIPS Build, TRX verification and artifact upload on an exact production-source SHA before claiming L3. L4 remains NOT VERIFIED.
5. Continue another concrete userland stability/security issue after exact checks are PASS or independently pending; do not repeat closed audits.

## Verification boundary

- L1: bounded EINTR retry paths added for procfs directory open/read; integration source assertions and injected tests added.
- L2: exact-head Process inventory/lifecycle Actions pending for this commit.
- L3: NOT VERIFIED until exact-SHA XY-C1 MIPS Build, TRX validation and artifact upload all pass.
- L4: NOT VERIFIED; no physical router was available.
- Backup remains immutable; no branches or PRs were created.


- L1: service lifecycle callback contract, error propagation and both production adapters updated; integration source assertions added.
- L2: exact-head Process lifecycle Actions pending; this session has not claimed a local full-checkout build.
- L3: NOT VERIFIED for current combined firmware until exact-SHA XY-C1 Build, TRX and artifact upload all pass.
- L4: NOT VERIFIED; no physical router was available.
- Backup remains immutable; no branches or PRs were created.


- L1: source and static contracts added; exact-head full repository checker pending.
- L2: local GNU89/Werror/ASan/UBSan interrupted-lock-wait regression PASS; exact-head CI pending.
- L3: NOT VERIFIED for the current combined firmware source until exact-SHA XY-C1 Build, TRX and artifact upload all pass.
- L4: NOT VERIFIED; no physical router was available.
- Backup remains immutable; no branches or PRs were created.

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
