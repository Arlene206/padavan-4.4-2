# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup branch `backup/pre-plugin-overhaul-2026-11-14` is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Parent main SHA for this SCUT errno-collision correction: `c2fe4764d6864ee3ebab43eeb21ce74f91a2578c`; the Process lifecycle umbrella `44621537460` and AI docs `44621537441` both PASS on this exact SHA. The corrected Crond lifecycle contract `44621067912` and AI docs `44621067804` PASS on `69d64f4...`.
- Exact-head AI handoff documentation `44620322445` PASS; Process inventory `44620322214` PASS; Firewall `44620322286`, vsftpd `44620322183`, PID-1 queue `44620322244`, ZeroTier `44620322876`, detect_internet `44620322573`, rstats `44620322742`, DNSMasq `44620322174`, detect_link `44620322410`, Wireless radio `44620322537`, NFS `44620322374`, and iNIC `44620322401` PASS on `69430c...`.
- Crond lifecycle `44620322749` exposed a stale caller assertion and C policy test passed; its corrected rerun `44621067912` PASS on `69d64f4bd9b70a8f32083ac84e8ebca0c8a97d9f`. AI handoff documentation `44621067804` PASS on that exact SHA.
- Process lifecycle umbrella `44620322615` exposed a second stale source split; after this was corrected, the exact-head Process umbrella `44621537460` PASS and AI docs `44621537441` PASS on `c2fe4764...`.
- Process umbrella `44620322615` was queued and XY-C1 `44620322487` in progress at last query. Backup remains frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged.
- Exact-head AI handoff documentation run `44617795116` PASS on `930dd7ed850ef59275cc340bc7c5a36bc7ebedc9`; the preceding missing `## Active task` contract failure is closed.
- OpenVPN credential staging run `44613650285` PASS on checker/docs descendant `f4fbd093e934fda5304aa796cba30d59eb29b3a4`; VPN server lifecycle `44613173654` PASS and XY-C1 Build/TRX/artifact `44613173721` PASS on the OpenVPN production source SHA `e01f70a766c4e628593e272192723f6add59ffca`.
- The frozen backup branch was last confirmed at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 is closed/unmerged and remains legacy inventory only.
- Root `dev_log.md` currently has 24 dated records (this errno-collision correction appends one), within the required 20–119 window; part-018 preserves the 101 complete records evicted by the preceding catch-up rotation.
- No physical router result is available. Host tests are L2; exact XY-C1 Build/TRX/artifact is L3; device behavior is L4 NOT VERIFIED.

## Active task

- Confirmed SCUT boot crontab result-propagation defect: `check_crontab.sh` returns 1 to mean a successful append and 0 when unchanged, but `init_crontab()` ORed the raw `system()` wait status and `init_router()` treated every nonzero result as a change. Script errors therefore triggered MTD save/crond restart instead of entering boot error aggregation. A failed `write_storage_to_mtd()` was logged but discarded from `boot_services_status`.
- This slice establishes 0=unchanged, 1=added, 2+=error; distinguishes grep no-match from I/O failure, checks append success, rejects existing symlink/non-regular cron paths, decodes `system()` wait status and errno, persists only after a confirmed append, and aggregates crontab/MTD failures while still attempting crond activation. A second edge case was found: Linux `EPERM` is errno 1, identical to the script’s successful-append code. `system()` failures now return negative errno, preventing `EPERM` from falsely opening the MTD/crond persistence branch; an explicit EPERM regression guards this.
- GNU89 policy tests cover normal/abnormal wait results, errno preservation, merge semantics and persistence gating. Shell tests cover add/no-op, invalid arguments, missing storage, grep error, non-regular targets and symlink rejection. Local host validation passed; exact-head Process and XY-C1 Actions are pending for this commit.

## Exact first action for the next GPT

1. Query live main and exact-head Actions for this EPERM/status-collision correction; ensure Process umbrella, Crond lifecycle, AI docs and XY-C1 Build/TRX/artifact all pass on an applicable exact source SHA.
2. Triage any failed job from its exact log. Do not weaken the 0/1/error contract or the fail-closed handling of grep/append/path failures. L3 remains pending until XY-C1 Build, TRX and artifact upload all pass on the same source SHA.
3. Continue the P1 startup/teardown status-propagation queue after exact-head validation; do not repeat the already-closed OpenVPN, DNSMasq resolver, `create_file()` close-status, USB modem notifier, WAN monitor-pause, or reviewed `services_ex.c` call paths without new evidence.

## Verification boundary

- L1: SCUT crontab script and RC boot result/persistence handling corrected in this source slice.
- L2: local GNU89/Werror/ASan/UBSan C policy binary, shell syntax and isolated shell regression PASS. Crond corrected checker PASS at `69d64f4`; Process umbrella integration checker correction is in this commit and awaits exact-head validation.
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
