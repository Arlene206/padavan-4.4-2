# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup branch `backup/pre-plugin-overhaul-2026-11-14` is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Product source fixes: PPP fail-closed recovery `5878c9d11404d1bd4a329b59430a7c1279025dd4`; APCLI WAN recovery result propagation `de73223da74d29a93236f29d8214fe66221b498c`; latest checker tip before THIS correction `b50ce7d86e43e60f0885f1d849a4f0dd690cffc8`. SCUT crontab tri-state source fix is in `69430c238e26c0de0b9210a5d090512bb4570f50`; crond checker correction is `69d64f4bd9b70a8f32083ac84e8ebca0c8a97d9f`; Process integration checker correction is `c2fe4764d6864ee3ebab43eeb21ce74f91a2578c`; SCUT errno sentinel correction is included in the parent.
- Exact-head Process lifecycle umbrella `44622196808`, Process inventory `44622196753`, and AI handoff docs `44622196858` PASS on `52cd91c1c7f0d5c297b157a9ffb636844363f605`. NVRAM audit `44622196798` was IN_PROGRESS when this slice was prepared. Crond lifecycle `44621067912` and AI docs `44621067804` PASS on `69d64f4...`.
- Exact parent `69430c...` XY-C1 run `44620322487` was still in progress at the last poll; inspect a new current-source XY-C1 Build/TRX/artifact run after this commit rather than treating that older build as current.
- The frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify it. PR #1 is closed/unmerged and remains legacy inventory only.
- OpenVPN credential staging run `44613650285` PASS on checker/docs descendant `f4fbd093e934fda5304aa796cba30d59eb29b3a4`; VPN server lifecycle `44613173654` PASS and XY-C1 Build/TRX/artifact `44613173721` PASS on OpenVPN product source `e01f70a766c4e628593e272192723f6add59ffca`.
- Root `dev_log.md` contained 26 dated records before this entry; the new record remains well below the 120-record rotation threshold. Part-018 preserves the 101 complete records evicted in the earlier catch-up rotation.
- No physical router result is available. Host regression/Actions are L2; exact XY-C1 Build/TRX/artifact is L3; device behavior is L4 NOT VERIFIED.

- On exact checker SHA `ccf419d1dcff6aff70080d37c90be707dec9fe88`, WAN route `44625039862` and AI docs `44625039727` PASS. Wireless `44625039991` and Process lifecycle `44625039798` reached the guest-LAN source contract and failed only because a negative assertion treated the new assigned `wan_status = check_apcli_wan(...)` call as a forbidden standalone call; THIS COMMIT now asserts a single captured call instead.
- Exact-head checker runs on `b50ce7d86e43e60f0885f1d849a4f0dd690cffc8`: HTTPD security `44624238578` and AI docs `44624238537` PASS. WAN route `44624238559` failed because its demand-PPP inline contract still expected the direct lower-level recovery helper; Wireless `44624238741` and Process lifecycle `44624238561` failed because the APCLI contract still expected an inline call instead of the new `wan_status` local. THIS COMMIT corrects those remaining expectations.
- XY-C1 workflow `44624238619` PASSed all host contracts (including WAN firewall) but explicitly SKIPPED Build XY-C1, TRX verification and artifact upload because this checker-only event had no changed firmware production inputs; it is not L3 evidence. A fresh source-triggered build or manual workflow dispatch is still required.

## Active task

- Newly confirmed P1 failure-propagation defect: `check_apcli_wan()` ran `full_restart_wan()` when MAN changed or `try_wan_reconnect(1, 0)` after the active WISP APCLI radio cycle, but it was declared `void` and discarded either nonzero WAN result. Both `restart_wifi_wl()` and `restart_wifi_rt()` therefore returned success when guest isolation and IAPP restart succeeded even though dependent WAN recovery failed.
- THIS COMMIT makes the helper return and log WAN recovery status, then merges it into both radio restart accumulators without replacing an earlier guest-isolation error. The GNU89 policy test covers full-restart/reconnect failures and sticky first-error behavior; the Python source contract pins both bands and production callers.
- The SCUT crontab tri-state source change and follow-up checker/sentinel fixes are complete. Exact-head Process lifecycle, Process inventory and AI docs checks PASS on parent `5878c9d11404d1bd4a329b59430a7c1279025dd4`; no local compilation was performed for this wireless slice in the GitHub-only workspace.
- The PPP ordinary-path fail-closed source change is committed and its GNU89 policy/source checks passed in Actions before the stale inline contracts; WAN wrapper usage and PPP error aggregation are now pinned by the updated checker. APCLI radio restarts now preserve the WAN recovery status in both bands; its checker pins the helper and accumulator flow. The current commit repairs the remaining APCLI callsite and PPP demand-fallback assertions. No production source changes are part of this test-only slice.

## Exact first action for the next GPT

1. Query live `main` and exact-head Wireless radio, Process lifecycle and AI docs Actions for THIS checker correction. The prior exact WAN route checker run `44625039862` PASS already confirms the PPP demand-fallback contract. Inspect any fresh failure logs before calling the APCLI contract closed.
2. If host contracts pass, obtain a current-source XY-C1 Build/TRX/artifact run: the checker-only `b50ce7d` XY workflow skipped those three L3 steps. Do not mistake host-only validation for firmware validation.
3. Continue the P1 cross-userland startup/teardown queue after validation; do not repeat the closed SCUT crontab, OpenVPN, DNSMasq resolver, `create_file()` close-status, USB modem notifier or WAN monitor-pause audits without new evidence.

## Verification boundary

- L1: PPP ordinary startup failures use the fail-closed physical-WAN recovery wrapper (`5878c9d`); `check_apcli_wan()` exposes `full_restart_wan()` and `try_wan_reconnect(1, 0)` failures to both wireless-band restart results (`de73223`).
- L2: WAN route contract `44625039862` and AI docs `44625039727` PASS on `ccf419d`; HTTPD security `44624238578` PASS on `b50ce7d`. Wireless `44625039991` and Process `44625039798` fail only at a stale negative callsite assertion after the actual `wan_status` assignment has already passed; THIS COMMIT changes it to assert exactly one captured call. Exact reruns pending; no local full build is claimed.
- L3: XY-C1 run `44624238619` on exact checker SHA `b50ce7d` passed host contracts but skipped Build, TRX verification and artifact upload because firmware inputs were unchanged. L3 remains NOT VERIFIED for the PPP/APCLI product-source changes until those three steps pass on one exact source SHA.
- L4: physical APCLI/WISP reconnection and WAN recovery on a router remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint. Compare live main, source baseline, exact-head Actions, module status and this first action. Update this file only when those resumable facts materially change; do not create a SHA-refresh or no-op commit.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; use `force=false` with the exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.
