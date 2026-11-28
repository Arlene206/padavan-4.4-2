## 2026-11-28 continuation checkpoint

- Parent live `main` before this atomic NVRAM flock/unlock slice: `5401539ce7c4af2f5b22267915c93b23db3c865d`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (must remain unchanged).
- Procfs directory EINTR retry slice `3fb57acc623b3fd227c6d0aa3646733cbcb6dce2`: inventory-only workflow #46128583255 PASS; its Process fail-closed #46128583279 first failed on an over-broad checker assertion. Correction `1151dd8237ea16e079d9989d3ba4f760ad783894` removed only that assertion; exact Process fail-closed #46128917625 and AI docs #46128917616 completed/success.
- NVRAM bounded wait fix `ac141ff53f1d29e25f6279d762c064de123092dc` initially failed NVRAM #46128697949 and HTTPD #46128697956 due signedness Werror and malformed checker syntax. `656733c2e9a7299494e69312217016fbeba64b2e` corrected both; its Process source inventory #46128844154, HTTPD #46128844113 and AI docs #46128844188 completed/success. NVRAM #46128844097 was still in progress at last poll.
- Review of the NVRAM lock primitive found both `flock(LOCK_NB)` and `flock(LOCK_UN)` still had unbounded EINTR retry loops. This commit uses one bounded helper for both and adds injected success/exhaustion tests; exact-head NVRAM audit is required.
- L3 XY-C1 production Build/TRX/artifact and physical-router L4 remain unverified unless a single exact-source run and hardware session demonstrate all checks. No local full firmware build/hardware run.
- First next action: fetch this commit's live main SHA and inspect exact-head NVRAM boundary, HTTPD security and AI documentation Actions; continue any pending Process validation without repeating completed audits.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; only `main` is authorized for development.
- This atomic source slice is parented to `5401539ce7c4af2f5b22267915c93b23db3c865d`; resolve its resulting SHA from live `main` after publication.
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

- NVRAM transaction-lock signal hardening. The prior nanosleep remainder loop now caps interruptions, but the nonblocking `flock(fd, LOCK_NB)` acquisition and `flock(fd, LOCK_UN)` release loops still retried EINTR without limits, allowing a signal storm to pin acquisition/release.
- This slice adds a shared eight-interruption counter and an injectable bounded flock wrapper; both production operations now fail with EINTR after exhaustion. Acquisition follows the existing descriptor cleanup path. A fake-flock regression tests retry success and bounded exhaustion; the checker requires both production call sites to use the helper.
- Exact-head NVRAM boundary, HTTPD and AI Actions are pending for this change.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-SHA NVRAM transaction-boundary, HTTPD Authentication/Security and AI handoff Actions for this flock/unlock retry commit.
2. Confirm GNU89/Werror and ASan/UBSan lock tests pass, including successful acquisition and unlock retries, bounded flock exhaustion, and bounded nanosleep interruption.
3. Confirm NVRAM static gate parses and validates both acquisition and release helper call sites.
4. If any exact-head job fails, inspect its job log and fix only the causal defect. A pending job does not block independent work.
5. Require exact-SHA XY-C1 MIPS Build, TRX validation and artifact upload before claiming L3; L4 remains NOT VERIFIED.

## Verification boundary

- L1: bounded flock/unlock EINTR helper, production call-site wiring, fake-flock regression and static assertions committed atomically.
- L2: exact-head NVRAM/HTTPD/AI Actions pending.
- L3: NOT VERIFIED until exact-SHA XY-C1 MIPS Build, TRX validation and artifact upload all pass.
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
