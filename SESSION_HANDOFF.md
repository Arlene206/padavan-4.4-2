# Session Handoff — 2026-11-28

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; this task's parent `main` SHA: `ac29ab8c94b96873b7591a77b88df0f10e6c79fc`.
- Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify it. PR #1 remains closed/unmerged.
- Exact `ac29...` Actions: iperf3 #46122179509 PASS; Process lifecycle #46122179365 PASS; Process inventory #46122179375 PASS; AI handoff docs #46122179388 PASS. XY-C1 #46122179390 remains in progress at MIPS Build step 38; TRX step 39 and artifact upload step 40 are pending. Do not claim L3 for `ac29` until all three pass.
- Earlier product SHA `ef122034b4524ec1ce6768dd169ebee62a7da2ee` passed XY-C1 #46120731426: Build, TRX and artifact upload; artifact SHA-256 `c9e4fff453dd08361893447a1aa985e5699ad4ee0c13919b0a1e0f168373c9f5`. L3 applies to `ef122` only; physical hardware L4 remains NOT VERIFIED.
- No local firmware checkout/build environment is available. Host regressions and firmware results are GitHub Actions outputs, not local claims. Bootstrap recovery has already been completed once; do not repeat it. The roadmap is close to its hard size cap.

## Active task

1. **iperf3 executable-bound lifecycle:** exact `ac29...` iperf3 regression #46122179509 PASS; Process lifecycle #46122179365 and AI docs #46122179388 PASS. It verifies executable/argv/start_time and PID-scoped BusyBox stop.
2. **Current MTD rstats hardening:** parent `ac29...`. Replace `comm=rstats` + `killall` with exact `/sbin/rstats` executable identity and PID/start_time revalidation; test a same-comm process whose executable differs.
3. **Outstanding firmware gate:** XY-C1 #46122179390 is still at MIPS Build step 38, TRX and artifact steps pending. No L3 claim for `ac29` yet.

## Watchdog scheduled-action NVRAM snapshot baseline

- `04f55a0098f9328c7bd563fccbbafd21013e03b3` added checked single-transaction reads for reboot/SS selectors, `ntp_ready` and schedules. Exact watchdog, Process, NVRAM and AI docs checks passed; XY-C1 #46115631593 passed Build, TRX and artifact upload (artifact ID `15251593325`, 12,838,666 bytes). L3 applies to that exact SHA only.
- `f5bcffd73428bab5b4818e44465f3d25f8b7563a` optimized inactive schedule reads; exact watchdog #46115995003 FAILED because selector fields were evaluated before assignment.
- `9e154cc4eb761f2d876aeb9012a8a4264c3fe8d5` fixed selector assignment order. Watchdog #46116069843, Process #46116070166, NVRAM #46116069885 and AI docs #46116069828 PASS. XY-C1 #46116069855 remained in progress at Build; later 58700 changes production source again, so that result cannot validate the IPTV code.
- `6599ea954a69c70b4f811f66519e01b00671c634` added test-only pre-NTP schedule-read coverage. Watchdog #46116463184, Process #46116463183, NVRAM #46116463221 and AI docs #46116463180 PASS.

## Exact validation state

| Source SHA | Workflow / run | Result |
|---|---|---|
| `ac29ab8c94b96873b7591a77b88df0f10e6c79fc` | iperf3 #46122179509 | PASS |
| `ac29ab8c94b96873b7591a77b88df0f10e6c79fc` | Process lifecycle #46122179365; inventory #46122179375; AI docs #46122179388 | PASS |
| `ac29ab8c94b96873b7591a77b88df0f10e6c79fc` | XY-C1 #46122179390 | IN PROGRESS at MIPS Build step 38; TRX and artifact pending |
| `ef122034b4524ec1ce6768dd169ebee62a7da2ee` | XY-C1 #46120731426 | L3 PASS — Build, TRX, artifact upload |

## Exact first action for the next GPT

1. Inspect the exact-head MTD rstats workflow triggered by this change. Both host and BusyBox-ash restore-transaction variants must pass; on failure, inspect the first causal log and preserve save-barrier/rollback invariants.
2. Re-poll XY-C1 #46122179390 on `ac29...`; L3 requires Build, TRX and artifact upload all PASS. `ef122...` already has L3 PASS; hardware L4 is not verified.
3. Keep the frozen backup untouched and continue one independent userland stability/security defect after this slice.

## Verification boundary

- Source content and Git state are from GitHub; tests/firmware outcomes are the exact Actions runs above.
- No local checkout/build/test is claimed.
- XY-C1 Build/TRX/artifact is L3, not physical-router L4.
- Hardware flashing, boot, radio, WAN and live network validation are NOT VERIFIED.

## Mandatory handoff checkpoint

Every session MUST inspect the live repository state and compare it with this handoff before ending. The checkpoint is mandatory; a document edit is conditional. Confirm meaningful product-source baselines, active task, exact-head Actions, validation boundary, blockers and exact first action. If all facts remain accurate, leave this file untouched and do not create a no-op commit.

# Cross-session execution mandate

- Only `main` is authorized. Keep the frozen backup immutable; create no feature/checkpoint branches or PRs.
- For each multi-file slice, create all blobs and one complete Git tree, then publish one atomic main commit with the latest main as sole parent; advance with `force:false` and `expected_sha` equal to that parent. Never use per-file commits, force-push, amend or rewrite published history.
- Re-fetch main immediately before every write. If compare-and-swap rejects, inspect the new head, rebuild/retest and retry without force.
- Inspect push-triggered Actions by exact `head_sha`. PASS permits the next slice; FAIL must be diagnosed/fixed; PENDING does not block independent work.
- Keep root `dev_log.md` append-only and inside the 20–119 record / 384 KiB limits; batch-rotate only when required. Do not create routine handoff/roadmap archives.
- Before context exhaustion, finish a safe bounded slice, record exact source/test/CI state and leave the next GPT a precise first action. Do not ask the user what to do next.
