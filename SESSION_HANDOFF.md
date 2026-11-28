# Session Handoff — 2026-11-28

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main at this write's parent: `173fae1685922a5d94d90cb32f370e212eea137e`; this workflow-contract correction and handoff refresh are one atomic main commit.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14`, recorded HEAD `72f4f157a084f8184b9597ca5bf334b8080e5807`. Its warning file forbids mutation/deletion. PR #1 remains CLOSED/unmerged; its substantive unmerged work was already compared, with no missing production/test/build change identified.
- Startup recovery was completed once against the bootstrap, development protocol, handoff, roadmap and log indexes. Do not repeat completed audits.
- Root `dev_log.md` remains below the documented 384 KiB target and 120-record rotation threshold. Keep 20–119 records. The roadmap is close to its 128 KiB hard limit; avoid routine expansion.
- No local source checkout/build environment is available. Host regressions and firmware outcomes below are GitHub Actions results, not local claims.

## Active task

Latest production-source slice is `58700dc70b54d931aabdb6701f0ec88c14b9d1b4` (`fix(rc): reject invalid IPTV proxy ports`).

- Confirmed defect: `start_udpxy()` and `start_xupnpd()` treated any selector below 1024 as the legacy disabled value before checking the lower bound. Negative/corrupt NVRAM values such as `-1` were silently treated as a disabled service.
- New `iptv_proxy_enable_port_status()` preserves 0–1023 as disabled, accepts 1024–65535 as enabled and rejects negative or >65535 values with `ERANGE`. Both launchers apply it before the disabled return. GNU89/Werror/ASan/UBSan boundary coverage includes INT_MIN, negative values, disabled/enabled edges, max and overflow.
- Exact Process fail-closed #46116630850 first failed test compilation because `NULL` was undeclared; `173fae...` added `<stddef.h>`, and its test compile then passed. The same run failed later in the embedded LAN-startup source contract: it still searched for the old literal `if (xport < 1024)` although the new production helper correctly uses `iptv_proxy_enable_port_status()` followed by `if (!xport_enabled)`. This commit updates the stale contract for both xupnpd and udpxy to require the helper call before the disabled gate and process preflight. Product source is unchanged; a new exact-head Process run must pass.
- At 58700, Process lifecycle inventory #46116630819, HTTPD #46116630813, DNSMasq #46116630771, WAN DHCP callback #46116630786, WAN route #46116630800, firewall #46116630795 and AI docs #46116630784 PASS. USB storage #46116630890, NVRAM #46116630796 and XY-C1 #46116630792 were in progress at last poll.
- Docs-only `9e69966fa931849a012b5b38ee17ac8f968b80df` (parent 58700) passed AI docs #46116691062. This handoff/test correction follows it.

## Watchdog scheduled-action NVRAM snapshot baseline

- `04f55a0098f9328c7bd563fccbbafd21013e03b3` added checked single-transaction reads for reboot/SS selectors, `ntp_ready` and schedules. Exact watchdog, Process, NVRAM and AI docs checks passed; XY-C1 #46115631593 passed Build, TRX and artifact upload (artifact ID `15251593325`, 12,838,666 bytes). L3 applies to that exact SHA only.
- `f5bcffd73428bab5b4818e44465f3d25f8b7563a` optimized inactive schedule reads; exact watchdog #46115995003 FAILED because selector fields were evaluated before assignment.
- `9e154cc4eb761f2d876aeb9012a8a4264c3fe8d5` fixed selector assignment order. Watchdog #46116069843, Process #46116070166, NVRAM #46116069885 and AI docs #46116069828 PASS. XY-C1 #46116069855 remained in progress at Build; later 58700 changes production source again, so that result cannot validate the IPTV code.
- `6599ea954a69c70b4f811f66519e01b00671c634` added test-only pre-NTP schedule-read coverage. Watchdog #46116463184, Process #46116463183, NVRAM #46116463221 and AI docs #46116463180 PASS.

## Exact validation state

| Source SHA | Workflow / run | Result |
|---|---|---|
| `04f55a0098f9328c7bd563fccbbafd21013e03b3` | XY-C1 #46115631593 | PASS — Build, TRX, artifact upload |
| `04f55a0098f9328c7bd563fccbbafd21013e03b3` | Watchdog #46115631687; Process #46115631548/#46115631613; NVRAM #46115631554; AI docs #46115631633 | PASS |
| `f5bcffd73428bab5b4818e44465f3d25f8b7563a` | Watchdog #46115995003 | FAIL — selector ordering; corrected by 9e154 |
| `9e154cc4eb761f2d876aeb9012a8a4264c3fe8d5` | Watchdog #46116069843; Process #46116070166; NVRAM #46116069885; AI docs #46116069828 | PASS |
| `9e154cc4eb761f2d876aeb9012a8a4264c3fe8d5` | XY-C1 #46116069855 | IN PROGRESS at Build; TRX/artifact pending at last poll |
| `6599ea954a69c70b4f811f66519e01b00671c634` | Watchdog #46116463184; Process #46116463183; NVRAM #46116463221; AI docs #46116463180 | PASS |
| `58700dc70b54d931aabdb6701f0ec88c14b9d1b4` | Process inventory #46116630819; HTTPD #46116630813; DNSMasq #46116630771; WAN DHCP #46116630786; WAN route #46116630800; firewall #46116630795; AI docs #46116630784 | PASS |
| `58700dc70b54d931aabdb6701f0ec88c14b9d1b4` | Process fail-closed #46116630850 | FAIL — test compile (`NULL` undeclared) fixed in 173fa; subsequent embedded checker failed on stale `xport < 1024` contract |
| `58700dc70b54d931aabdb6701f0ec88c14b9d1b4` | USB storage #46116630890; NVRAM #46116630796; XY-C1 #46116630792 | IN PROGRESS at last poll |

- L3 applies to 04f55 only. Current production source includes later watchdog and IPTV changes; require exact 58700 XY-C1 Build/TRX/artifact success before claiming L3 for that source. This follow-up changes only a host-test include and docs, not firmware production input.
- L4 physical-router flash/boot, wireless/network and WAN behavior are NOT VERIFIED.
- Do not conflate host ASan/UBSan with MIPS build or hardware validation.

## Exact first action for the next GPT

1. Poll this commit's exact-head Process fail-closed run. Confirm the IPTV port regression compiles/runs under GNU89/Werror/ASan/UBSan and the embedded LAN-startup contract passes with the new range-policy helper callsites.
2. Poll exact product-source runs #46116630890 (USB storage), #46116630796 (NVRAM) and #46116630792 (XY-C1). For L3 require the same 58700 product SHA to complete Build, TRX and artifact upload; this commit only changes a host-test include and docs.
3. Diagnose any failed job before closing its task. Do not label the known compile failure as a product-source failure.
4. Continue an independent evidence-based P1 stability/security fix while Actions are pending. Avoid repeating watchdog, Process inventory, NVRAM boundary, detector, MiniUPnP and HTTPD audits already completed.

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
