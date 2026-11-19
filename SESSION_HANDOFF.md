# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup branch `backup/pre-plugin-overhaul-2026-11-14` is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Parent main before this slice: `f2e814729e9152102811b4be4b069efe4924bd4f` (`test(wifi): verify captured APCLI recovery result`).
- APCLI WAN recovery production fix is `de73223da74d29a93236f29d8214fe66221b498c`; PPP physical-WAN fail-closed recovery is `5878c9d11404d1bd4a329b59430a7c1279025dd4`. Checker-only follow-ups `ccf419d` and THIS parent correct the PPP and captured-APCLI caller contracts.
- Exact-head WAN route `44625039862` and AI docs `44625039727` PASS on `ccf419d1dcff6aff70080d37c90be707dec9fe88`. Wireless `44625039991` and Process `44625039798` failed only because a negative assertion matched the correct captured `wan_status = check_apcli_wan(...)` call; THIS parent changes the contract to require exactly one captured invocation. Those reruns remain to be checked.
- XY-C1 `44624238619` on checker-only SHA `b50ce7d` passed host contracts but skipped Build/TRX/artifact because no production input changed. It is not L3 evidence for the PPP/APCLI source fixes. This storage slice changes firmware input `hotplug_stor.c`, so its exact-head XY-C1 workflow should perform a build.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is archival only. PR #1 remains closed/unmerged.
- This slice changes SD/MMC mass-storage notification handling, adds a GNU89 completion policy/regression, extends the storage stability workflow and XY-C1 input filter, and adds a storage source contract. Local policy regression PASS; exact-head Storage/XY-C1 Actions must be checked after commit.
- The GitHub connector's push-run lookup is incomplete: `fetch_commit_workflow_runs` returns PR-triggered runs and combined commit status can return an empty list for push-only commits. Never treat an empty result as PASS; inspect the repository's push Actions directly when available.
- Root `dev_log.md` remains within its 20–119 dated-record window; no archive rotation is needed for this slice.
- No physical router result is available. Host regression/Actions are L2; XY-C1 Build/TRX/artifact on one exact product SHA is L3; device behavior is L4 NOT VERIFIED.

## Active task

- Confirmed mass-storage event-publication defect in `trunk/user/rc/src/hotplug_stor.c`: `mdev_sd_main()` and `mdev_mmc_main()` used the legacy void `notify_rc()` for whole-device removal and successful automount. The shared implementation can fail to create one or both secure PID 1 queue markers, but the caller discarded the failure and could return success while cleanup/startup was never scheduled.
- This slice switches those paths to `notify_rc_status()`, logs and preserves queue errno, includes notification status in both applet return codes, and retains the existing swapoff-before-notify and notify-after-successful-mount ordering. A GNU89/Werror/ASan/UBSan policy regression plus semantic source/workflow contracts cover both SD and MMC paths.
- Local policy binary PASS. Full storage source checker and exact-head Actions are not yet observed; do not claim Actions or XY-C1 L3 until their exact-head results are inspected.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head `storage-hotplug-stability`, XY-C1 firmware validation, and AI handoff documentation Actions for this commit. Also inspect exact-head Wireless/Process reruns on `f2e8147`. Check Build, TRX verification and artifact upload on the same product-source SHA.
2. On any FAIL, inspect the exact job log. Preserve the rule that a queued marker failure returns mdev failure; a successful queue followed by a failed PID 1 wakeup is logged by `notify_rc_status()` but remains queued and is not misreported as an unqueued event.
3. Continue the P1 cross-userland result-propagation queue after verification. Do not reopen closed SCUT crontab, OpenVPN, WAN firewall, APCLI radio or USB modem notifier paths without new evidence. Keep L1/L2/L3/L4 claims distinct.

## Verification boundary

- L1: SD/MMC add and whole-device removal now use checked PID 1 notification queue status; failed queue creation is reflected in mdev exit status.
- L2: local GNU89/Werror/ASan/UBSan notification completion policy regression PASS. Storage workflow source contracts and exact-head Action outcomes require post-push validation. The APCLI contract correction at f2e awaits exact-head Wireless/Process reruns.
- L3: XY-C1 Build, TRX verification and artifact upload for the exact source SHA are PENDING; this commit changes a firmware production input, so the build must not be skipped as a checker-only event.
- L4: physical USB/SD/MMC hotplug, PID 1 queue failure recovery and service transition behavior on hardware NOT VERIFIED. Physical PPP/APCLI router behavior is also unverified.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint. Compare live main, source baseline, exact-head Actions, module status and this first action. Update this file only when those resumable facts materially change; do not create a SHA-refresh or no-op commit.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; use `force=false` with the exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.
