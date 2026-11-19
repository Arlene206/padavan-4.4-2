# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup branch `backup/pre-plugin-overhaul-2026-11-14` is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Parent main SHA for this wireless restart result-propagation slice: `5878c9d11404d1bd4a329b59430a7c1279025dd4`. SCUT crontab tri-state source fix is in `69430c238e26c0de0b9210a5d090512bb4570f50`; crond checker correction is `69d64f4bd9b70a8f32083ac84e8ebca0c8a97d9f`; Process integration checker correction is `c2fe4764d6864ee3ebab43eeb21ce74f91a2578c`; SCUT errno sentinel correction is included in the parent.
- Exact-head Process lifecycle umbrella `44622196753`, Process inventory `44622196808`, and AI handoff docs `44622196858` PASS on `52cd91c1c7f0d5c297b157a9ffb636844363f605`. NVRAM audit `44622196798` was IN_PROGRESS when this slice was prepared. Crond lifecycle `44621067912` and AI docs `44621067804` PASS on `69d64f4...`.
- Exact parent `69430c...` XY-C1 run `44620322487` was still in progress at the last poll; inspect a new current-source XY-C1 Build/TRX/artifact run after this commit rather than treating that older build as current.
- The frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify it. PR #1 is closed/unmerged and remains legacy inventory only.
- OpenVPN credential staging run `44613650285` PASS on checker/docs descendant `f4fbd093e934fda5304aa796cba30d59eb29b3a4`; VPN server lifecycle `44613173654` PASS and XY-C1 Build/TRX/artifact `44613173721` PASS on OpenVPN product source `e01f70a766c4e628593e272192723f6add59ffca`.
- Root `dev_log.md` contained 26 dated records before this entry; the new record remains well below the 120-record rotation threshold. Part-018 preserves the 101 complete records evicted in the earlier catch-up rotation.
- No physical router result is available. Host regression/Actions are L2; exact XY-C1 Build/TRX/artifact is L3; device behavior is L4 NOT VERIFIED.

## Active task

- Newly confirmed P1 failure-propagation defect: `check_apcli_wan()` ran `full_restart_wan()` when MAN changed or `try_wan_reconnect(1, 0)` after the active WISP APCLI radio cycle, but it was declared `void` and discarded either nonzero WAN result. Both `restart_wifi_wl()` and `restart_wifi_rt()` therefore returned success when guest isolation and IAPP restart succeeded even though dependent WAN recovery failed.
- THIS COMMIT makes the helper return and log WAN recovery status, then merges it into both radio restart accumulators without replacing an earlier guest-isolation error. The GNU89 policy test covers full-restart/reconnect failures and sticky first-error behavior; the Python source contract pins both bands and production callers.
- The SCUT crontab tri-state source change and follow-up checker/sentinel fixes are complete. Exact-head Process lifecycle, Process inventory and AI docs checks PASS on parent `5878c9d11404d1bd4a329b59430a7c1279025dd4`; no local compilation was performed for this wireless slice in the GitHub-only workspace.

## Exact first action for the next GPT

1. Query live `main` and exact-head Actions for THIS wireless result-propagation commit. Confirm Wireless radio and Process lifecycle, then inspect AI handoff docs, NVRAM and XY-C1 Build/TRX/artifact results on the same SHA.
2. Triage any failed job from its exact log. Do not weaken first-error semantics or treat an APCLI WAN recovery failure as a successful radio restart.
3. Continue the P1 cross-userland startup/teardown queue after validation; do not repeat the closed SCUT crontab, OpenVPN, DNSMasq resolver, `create_file()` close-status, USB modem notifier or WAN monitor-pause audits without new evidence.

## Verification boundary

- L1: source review confirms `check_apcli_wan()` now exposes failures from both `full_restart_wan()` and `try_wan_reconnect(1, 0)` to 2.4GHz/5GHz wireless restart results.
- L2: GNU89 policy and source-contract regressions were added. They have NOT been executed locally in this GitHub-only workspace; query exact-head Wireless/Process Actions after push.
- L3: exact-head XY-C1 Build, TRX verification and artifact upload must all pass on the same source SHA before firmware validation is claimed.
- L4: physical APCLI/WISP reconnection and WAN recovery on a router remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint. Compare live main, source baseline, exact-head Actions, module status and this first action. Update this file only when those resumable facts materially change; do not create a SHA-refresh or no-op commit.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; use `force=false` with the exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.
