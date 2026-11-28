# Session Handoff — 2026-11-28

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Live `main` at handoff preparation: `58700dc70b54d931aabdb6701f0ec88c14b9d1b4`. All development stays on `main`; do not create a feature/checkpoint branch or PR.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14`, recorded HEAD `72f4f157a084f8184b9597ca5bf334b8080e5807`. The branch warning file says never mutate/delete it. PR #1 remains CLOSED/unmerged; its substantive unmerged changes were already compared, with no valid missing production/test/build change identified. Do not reopen or port it.
- Startup recovery was performed once against `AI_BOOTSTRAP_PROMPT.md`, `AI_DEVELOPMENT_PROTOCOL.md`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and the root/archived dev log index. Do not repeat completed audits.
- The root `dev_log.md` had 104 entries and about 101 KB before this checkpoint; retain the 20–119 entry window and 384 KiB target. No archive rotation is due. `REFACTOR_ROADMAP.md` is close to its 128 KiB limit; do not expand it for routine updates.
- No local source checkout/build environment is available in this session. Host regressions/build results below are GitHub Actions results, not local claims.

## Active task

The current product-source slice is the watchdog's time-triggered reboot/update scheduler NVRAM snapshot.

1. `04f55a0098f9328c7bd563fccbbafd21013e03b3` added checked, single-transaction reads of the reboot/SS enable selectors, `ntp_ready` and schedules. It copies transient cache pointers before the next read, preserves missing-key defaults, rejects overlong values, wipes partial snapshots on failure, propagates transaction-close failure and defers both actions on a read error.
2. `f5bcffd73428bab5b4818e44465f3d25f8b7563a` optimized away inactive schedule-string reads. Its exact watchdog run #46115995003 FAILED: selectors were assigned only after the gating checks, so enabled schedule values were skipped. This is a real failure and is not a passing baseline.
3. `9e154cc4eb761f2d876aeb9012a8a4264c3fe8d5` corrected selector assignment order before conditional schedule reads and adjusted the three-read missing/default behavior. Its exact watchdog, AI docs, Process inventory and NVRAM checks all PASS. Exact XY-C1 run #46116069855 was still IN_PROGRESS at Build XY-C1 when this handoff was prepared; TRX and artifact steps had not started. Do not claim L3 for this SHA until Build, TRX and upload all pass.
4. Current main `6599ea954a69c70b4f811f66519e01b00671c634` is a test/checker/dev-log-only descendant of `9e154...`. It adds an explicit test case proving both schedule keys are not fetched when NTP is not ready, even when both actions are enabled. Its exact watchdog #46116463184, AI docs #46116463180, Process inventory #46116463183 and NVRAM boundary #46116463221 are all PASS. No XY-C1 run was triggered because this commit changes no firmware production input.

The watchdog snapshot regression covers cache-pointer invalidation, missing keys, backend read failure/atomic clearing, overlong values, strict selector tokens, disabled actions, inactive schedule errors, and the pre-NTP-ready path. The source checker pins transaction begin/read/end, selector-before-schedule ordering, removal of the legacy unchecked getters and test presence.

## Exact validation state

| SHA | Workflow / run | Result |
|---|---|---|
| `04f55a0098f9328c7bd563fccbbafd21013e03b3` | Watchdog #46115631687 | PASS |
| `04f55a0098f9328c7bd563fccbbafd21013e03b3` | Process inventory #46115631548 / fail-closed #46115631613 | PASS |
| `04f55a0098f9328c7bd563fccbbafd21013e03b3` | NVRAM boundary #46115631554 / AI docs #46115631633 | PASS |
| `04f55a0098f9328c7bd563fccbbafd21013e03b3` | XY-C1 #46115631593 | PASS — Build, TRX verification and artifact upload; artifact `XY-C1-firmware-04f55a0098f9328c7bd563fccbbafd21013e03b3`, ID `15251593325`, 12,838,666 bytes |
| `f5bcffd73428bab5b4818e44465f3d25f8b7563a` | Watchdog #46115995003 | FAIL — selector assignment order; corrected in `9e154...` |
| `9e154cc4eb761f2d876aeb9012a8a4264c3fe8d5` | Watchdog #46116069843 | PASS |
| `9e154cc4eb761f2d876aeb9012a8a4264c3fe8d5` | Process inventory #46116070166 | PASS |
| `9e154cc4eb761f2d876aeb9012a8a4264c3fe8d5` | NVRAM boundary #46116069885 / AI docs #46116069828 | PASS |
| `9e154cc4eb761f2d876aeb9012a8a4264c3fe8d5` | XY-C1 #46116069855 | IN PROGRESS at Build; TRX/artifact pending at last poll |
| `6599ea954a69c70b4f811f66519e01b00671c634` | Watchdog #46116463184 | PASS — includes explicit pre-NTP-ready test |
| `6599ea954a69c70b4f811f66519e01b00671c634` | Process inventory #46116463183 / AI docs #46116463180 | PASS |
| `6599ea954a69c70b4f811f66519e01b00671c634` | NVRAM boundary #46116463221 | PASS |

- Exact earlier XY-C1 #46115245898 passed Build/TRX/artifact on `2e57ff8e98ba55aac56fc2a8d01d1b6ff8c6d19c`; it verifies only that earlier source SHA and is superseded by the later exact `04f55...` firmware pass.
- L3 is verified for `04f55...` only. L3 for the current watchdog production code requires the pending exact `9e154...` XY-C1 Build/TRX/artifact run. Since current `6599...` changes no production firmware input, a successful exact `9e154...` build covers the same product source currently on main.
- L4 physical-router flash/boot, live radio/network behavior and WAN behavior are NOT VERIFIED.
- Host ASan/UBSan/GNU89 results must not be described as MIPS target validation.

## Exact first action for the next GPT

1. Poll XY-C1 run #46116069855 for source SHA `9e154cc4eb761f2d876aeb9012a8a4264c3fe8d5`. Require Build XY-C1, TRX verification and artifact upload all SUCCESS before recording L3 for the current production source.
2. If that run fails, inspect its failed step/log, make a bounded correction with regression coverage, publish one complete tree/commit and check the new exact SHA. Do not rely on the older `04f55...` firmware result for the scheduler changes.
3. Continue with a distinct, evidence-based P1 reliability/security defect from the roadmap. Do not repeat watchdog, Process inventory, NVRAM boundary, detector, MiniUPnP or HTTPD audits already completed. Keep roadmap edits minimal because it is near the size limit.
4. Do not wait idle for a pending build; continue independent work and poll later.

## Verification boundary

- Repository source state is from GitHub fetches and commits; host regressions and firmware validation are from the exact Actions runs listed above.
- No local checkout/build/test is claimed.
- XY-C1 is a firmware build/TRX/artifact gate, not a physical router test.
- Never claim firmware flashing, boot, wireless, WAN, or live network verification without an actual hardware run.

## Mandatory handoff checkpoint

Every session MUST inspect the live repository state and compare it with this handoff before ending. The checkpoint is mandatory; a document edit is conditional. Confirm meaningful product-source baselines, active task, exact-head Actions, validation boundary, blockers and exact first action. If all facts remain accurate, leave this file untouched and do not create a no-op commit.

# Cross-session execution mandate

- Only `main` is an authorized development branch. Keep the frozen backup immutable; create no feature/checkpoint branch and no pull request.
- For each multi-file engineering slice, create all blobs and one complete Git tree, then publish one atomic main commit with the latest main as sole parent; advance main with `force:false` and `expected_sha` equal to that parent. Never use sequential per-file commits, force-push, amend or rewrite published history.
- Re-fetch main immediately before every write. If compare-and-swap rejects because main moved, inspect the new head, rebuild/retest on top of it and retry without force.
- Inspect push-triggered Actions by exact `head_sha`. PASS permits the next slice; FAIL must be diagnosed/fixed; PENDING does not block independent work.
- Keep root `dev_log.md` append-only and within the documented 20–119 record / 384 KiB limits; follow `docs/dev-log/INDEX.md` when a batch archive is actually required. Do not create routine handoff or roadmap snapshot archives.
- Before context exhaustion, finish and publish a safe bounded slice, record exact source/test/CI status, and leave the next GPT a precise first action. Do not ask the user what to do next.
