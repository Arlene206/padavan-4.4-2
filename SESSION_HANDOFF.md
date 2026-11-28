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

## Active task

- Shared process lifecycle timeout EINTR safety. Confirmed defect in `service_lifecycle_wait_for_exit()`: the callback returned void and the loop treated every call as a full elapsed second, while the two production adapters (Inadyn and USB printer shutdown) used raw `sleep(seconds)` and discarded the unslept remainder.
- A signal could shorten graceful shutdown and make the policy hit its timeout early, incorrectly advancing to SIGKILL or reporting a timeout before the intended wait elapsed.
- This atomic slice changes the callback to return status; production adapters call the bounded EINTR-aware `service_wait_seconds()`; callback failure yields `SERVICE_LIFECYCLE_FAILED`, preserving errno and blocking force-kill/follow-up.
- New regression injects EINTR, checks one TERM / zero KILL and preserves the error. The process lifecycle integration checker now requires the policy and both adapter changes.
- Full exact-head process workflow is pending; no local full firmware build or physical-router test is claimed.

## Exact first action for the next GPT

1. Fetch live `main` and inspect GitHub Actions for that exact SHA. First check NVRAM transaction-boundary audit and AI handoff documentation; then Process inventory/lifecycle and XY-C1. Do not substitute parent-SHA results.
2. If any exact-head job fails, fetch its job steps/log and fix the causal defect directly on `main`. PENDING does not block independent work.
3. Require XY-C1 MIPS Build, TRX verification and artifact upload on a production-source SHA before claiming L3. L4 physical-router validation remains NOT VERIFIED.
4. After this EINTR-wait slice passes, continue another concrete cross-userland stability/security defect. Do not repeat completed NVRAM checks without new evidence.

## Verification boundary

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
