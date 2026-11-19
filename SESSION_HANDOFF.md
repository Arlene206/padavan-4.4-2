# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup branch `backup/pre-plugin-overhaul-2026-11-14` is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Parent main SHA for this source slice: `930dd7ed850ef59275cc340bc7c5a36bc7ebedc9`. Its tree is the base for this atomic change; resolve live main before another write.
- Exact-head AI handoff documentation run `44617795116` PASS on `930dd7ed850ef59275cc340bc7c5a36bc7ebedc9`; the preceding missing `## Active task` contract failure is closed.
- OpenVPN credential staging run `44613650285` PASS on checker/docs descendant `f4fbd093e934fda5304aa796cba30d59eb29b3a4`; VPN server lifecycle `44613173654` PASS and XY-C1 Build/TRX/artifact `44613173721` PASS on the OpenVPN production source SHA `e01f70a766c4e628593e272192723f6add59ffca`.
- The frozen backup branch was last confirmed at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 is closed/unmerged and remains legacy inventory only.
- Root `dev_log.md` currently has 21 dated records after the 101-record archive rotation in part-018; this source slice adds one record and remains under the 120-record rotation threshold.
- No physical router result is available. Host tests are L2; exact XY-C1 Build/TRX/artifact is L3; device behavior is L4 NOT VERIFIED.

## Active task

- Confirmed SCUT boot crontab result-propagation defect: `check_crontab.sh` returns 1 to mean a successful append and 0 when unchanged, but `init_crontab()` ORed the raw `system()` wait status and `init_router()` treated every nonzero result as a change. Script errors therefore triggered MTD save/crond restart instead of entering boot error aggregation. A failed `write_storage_to_mtd()` was logged but discarded from `boot_services_status`.
- This slice establishes 0=unchanged, 1=added, 2+=error; distinguishes grep no-match from I/O failure, checks append success, rejects existing symlink/non-regular cron paths, decodes `system()` wait status and errno, persists only after a confirmed append, and aggregates crontab/MTD failures while still attempting crond activation.
- GNU89 policy tests cover normal/abnormal wait results, errno preservation, merge semantics and persistence gating. Shell tests cover add/no-op, invalid arguments, missing storage, grep error, non-regular targets and symlink rejection. Local host validation passed; exact-head Process and XY-C1 Actions are pending for this commit.

## Exact first action for the next GPT

1. Resolve live main and query Actions by exact `head_sha`. Inspect Process lifecycle regression, XY-C1 Build/TRX/artifact and AI handoff documentation contract for this source commit.
2. Triage any failed job from its exact log. Do not weaken the 0/1/error contract or the fail-closed handling of grep/append/path failures. L3 remains pending until XY-C1 Build, TRX and artifact upload all pass on the same source SHA.
3. Continue the P1 startup/teardown status-propagation queue after exact-head validation; do not repeat the already-closed OpenVPN, DNSMasq resolver, `create_file()` close-status, USB modem notifier, WAN monitor-pause, or reviewed `services_ex.c` call paths without new evidence.

## Verification boundary

- L1: SCUT crontab script and RC boot result/persistence handling corrected in this source slice.
- L2: local GNU89/Werror/ASan/UBSan policy binary PASS; shell syntax and isolated shell regression PASS. The source-order contract is wired into Process lifecycle Actions; exact-head result pending.
- L3: `rc.c` and the firmware-installed cron script changed; exact-head XY-C1 Build/TRX/artifact pending.
- L4: physical crontab persistence, crond runtime and router boot behavior NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint. Compare live main, source baseline, exact-head Actions, module status and this first action. Update this file only when those resumable facts materially change; do not create a SHA-refresh or no-op commit.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; use `force=false` with the exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.
