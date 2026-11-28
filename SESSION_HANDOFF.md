# Session Handoff — 2026-11-28

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; live `main` parent before this slice: `76f95c69dca1325985e9f38c7dc9bb0faf40ec7b` (parent `e87be480e8f1e714a53de04f7313a444c54205bf`).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is immutable. PR #1 remains closed/unmerged.
- Exact-head runs on `76f95c69...` at last poll: Storage hotplug #46123661683 IN_PROGRESS at its first launcher-syntax step; AI handoff #46123661589 IN_PROGRESS during checkout; XY-C1 #46123661636 IN_PROGRESS during checkout, before the build stage. These results do not validate later commits.
- The last established firmware L3 is product source SHA `2d0388614f8a1f3f3a11ce2bf57f7f63a19d879c`, XY-C1 #46122904412 Build/TRX/artifact PASS. The `e87` and `76f95` follow-ups are checker/docs-only relative to that product source. This Shadowsocks slice changes firmware source, so its own exact-SHA XY-C1 Build, TRX and artifact upload are required. Hardware L4 remains NOT VERIFIED.
- Bootstrap recovery was completed once; do not repeat audits. Keep `REFACTOR_ROADMAP.md` unchanged because it is near its hard size limit. No local firmware checkout/build environment is available; report host and firmware tests only from actual Actions results.

## Active task

1. **Shadowsocks process-shutdown identity:** the previous helper used process-name `killall` and a broad `pkill -f /usr/bin/ss-monitor` after mutable name/pattern lookups. The new implementation snapshots exact executable identity (and exact monitor interpreter/argv), Linux proc start_time and PID, revalidates the original PID/start_time/executable immediately before TERM/KILL, and escalates only members of the original snapshot. A replacement process, including same-PID/same-executable reuse with a changed start_time, must not inherit KILL authorization. Incomplete/deleted executable metadata and same-comm foreign binaries fail closed. Regression coverage includes malformed metadata, deleted executable, same-comm foreign process, exact monitor argv, PID reuse, TERM/KILL, and stuck-process failure. Exact-head CI for this slice is pending after its atomic source/test/checker commit; no local runtime result is claimed.
2. **Firmware validation:** the prior L3 baseline is `2d0388614f8a1f3f3a11ce2bf57f7f63a19d879c` (#46122904412 Build/TRX/artifact PASS). The new Shadowsocks helper/test work is firmware source and must receive its own XY-C1 build/TRX/artifact PASS. Hardware L4 remains NOT VERIFIED.
3. **Concurrent prior-head checks:** Storage #46123661683, AI handoff #46123661589 and XY-C1 #46123661636 were still running on `76f95c69...` at last poll. Do not confuse those results with validation of a later source SHA.

## Watchdog scheduled-action NVRAM snapshot baseline

- `04f55a0098f9328c7bd563fccbbafd21013e03b3` added checked single-transaction reads for reboot/SS selectors, `ntp_ready` and schedules. Exact watchdog, Process, NVRAM and AI docs checks passed; XY-C1 #46115631593 passed Build, TRX and artifact upload (artifact ID `15251593325`, 12,838,666 bytes). L3 applies to that exact SHA only.
- `f5bcffd73428bab5b4818e44465f3d25f8b7563a` optimized inactive schedule reads; exact watchdog #46115995003 FAILED because selector fields were evaluated before assignment.
- `9e154cc4eb761f2d876aeb9012a8a4264c3fe8d5` fixed selector assignment order. Watchdog #46116069843, Process #46116070166, NVRAM #46116069885 and AI docs #46116069828 PASS. XY-C1 #46116069855 remained in progress at Build; later 58700 changes production source again, so that result cannot validate the IPTV code.
- `6599ea954a69c70b4f811f66519e01b00671c634` added test-only pre-NTP schedule-read coverage. Watchdog #46116463184, Process #46116463183, NVRAM #46116463221 and AI docs #46116463180 PASS.

## Exact validation state

| Source SHA | Workflow / run | Result |
|---|---|---|
| `76f95c69dca1325985e9f38c7dc9bb0faf40ec7b` | Storage hotplug #46123661683 | IN_PROGRESS at launcher-syntax step 1 |
| `76f95c69dca1325985e9f38c7dc9bb0faf40ec7b` | AI handoff #46123661589 | IN_PROGRESS during checkout |
| `76f95c69dca1325985e9f38c7dc9bb0faf40ec7b` | XY-C1 #46123661636 | IN_PROGRESS during checkout, before Build/TRX/artifact |
| `2d0388614f8a1f3f3a11ce2bf57f7f63a19d879c` | XY-C1 #46122904412 | L3 PASS — MIPS Build, TRX verification, artifact upload |
| `2d0388614f8a1f3f3a11ce2bf57f7f63a19d879c` | AI docs #46122904358; Process inventory #46122904384; GPIO #46122904392; HTTPD security #46122904374 | PASS |

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head Shadowsocks lifecycle and Process inventory/lifecycle Actions for the new executable-bound stop slice. Require shell syntax, the expanded `process_lifecycle.sh` regressions, and `check_shadowsocks_lifecycle.py` to pass. Diagnose failures from the exact job logs; do not weaken the fail-closed checks.
2. Inspect prior-head Storage #46123661683, AI handoff #46123661589 and XY-C1 #46123661636 if still useful, but keep their `76f95` SHA boundary explicit.
3. Check the new source commit's own XY-C1 run through Build, TRX verification and artifact upload before claiming L3. The established `2d038...` L3 does not automatically validate later source changes. Hardware L4 remains NOT VERIFIED.
4. Keep the backup immutable; after this slice is green, proceed to another independent userland stability/security defect rather than repeating already-completed audits.

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
