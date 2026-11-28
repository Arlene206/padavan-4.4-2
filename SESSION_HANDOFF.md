# Session Handoff — 2026-11-28

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main at this write's parent: `1acc4300c58e9f5324cf7f4c4d75a9a3a191ff94`; this validation slice changes CI coverage only, not firmware production source.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14`, recorded HEAD `72f4f157a084f8184b9597ca5bf334b8080e5807`. Its warning file forbids mutation/deletion. PR #1 remains CLOSED/unmerged; its substantive unmerged work was already compared, with no missing production/test/build change identified.
- Startup recovery was completed once against the bootstrap, development protocol, handoff, roadmap and log indexes. Do not repeat completed audits.
- Root `dev_log.md` remains below the documented 384 KiB target and 120-record rotation threshold. Keep 20–119 records. The roadmap is close to its 128 KiB hard limit; avoid routine expansion.
- No local source checkout/build environment is available. Host regressions and firmware outcomes below are GitHub Actions results, not local claims.

## Active task

Latest production-source slice is `58700dc70b54d931aabdb6701f0ec88c14b9d1b4` (`fix(rc): reject invalid IPTV proxy ports`).

- Confirmed defect: `start_udpxy()` and `start_xupnpd()` treated any selector below 1024 as the legacy disabled value before checking the lower bound. Negative/corrupt NVRAM values such as `-1` were silently treated as a disabled service.
- New `iptv_proxy_enable_port_status()` preserves 0–1023 as disabled, accepts 1024–65535 as enabled and rejects negative or >65535 values with `ERANGE`. Both launchers apply it before the disabled return. GNU89/Werror/ASan/UBSan boundary coverage includes INT_MIN, negative values, disabled/enabled edges, max and overflow.
- Process fail-closed run #46116630850 initially exposed two stale-test issues: the new IPTV port test lacked `<stddef.h>` for `NULL`, then its rerun still searched for the removed literal `if (xport < 1024)`. Commit `173fae1685922a5d94d90cb32f370e212eea137e` fixed the test include; `6a1b28a152bed4341c912ae797945ee83f852d1c` updated the embedded LAN-startup contract to require `iptv_proxy_enable_port_status()` before `if (!xport_enabled)` and process preflight for both xupnpd and udpxy. Exact Process fail-closed run #46117034795 on 6a1b completed SUCCESS, including the IPTV port regression and LAN startup contract. AI docs #46117034802 also PASS. Neither correction changed firmware production source.
- On product SHA 58700, Process inventory #46116630819, HTTPD #46116630813, DNSMasq #46116630771, WAN DHCP callback #46116630786, WAN route #46116630800, firewall #46116630795, USB storage #46116630890, NVRAM #46116630796 and AI docs #46116630784 all PASS. XY-C1 #46116630792 is now PASS: Build, TRX verification and artifact upload all succeeded; artifact `XY-C1-firmware-58700dc70b54d931aabdb6701f0ec88c14b9d1b4`, ID `15252233106`, 12,838,305 bytes.
- Follow-up commits `9e69966fa931849a012b5b38ee17ac8f968b80df`, `173fae1685922a5d94d90cb32f370e212eea137e`, `6a1b28a152bed4341c912ae797945ee83f852d1c` and `a747994f8892363f94938655b6db68195d44c9dd` changed only handoff/log, one host-test include and CI source-contract assertions. Compare `58700...` → `a747...` confirms no production firmware path changed. Therefore the successful exact 58700 XY-C1 result validates the current product source on main.

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
| `173fae1685922a5d94d90cb32f370e212eea137e` | Process fail-closed #46116838163 | FAIL — test include fixed, then stale xupnpd port-gate assertion; no product source failure |
| `6a1b28a152bed4341c912ae797945ee83f852d1c` | Process fail-closed #46117034795 | PASS — IPTV port GNU89/Werror/ASan/UBSan regression and xupnpd/udpxy LAN-startup source contracts |
| `6a1b28a152bed4341c912ae797945ee83f852d1c` | Process fail-closed #46117034795; AI docs #46117034802 | PASS |
| `58700dc70b54d931aabdb6701f0ec88c14b9d1b4` | USB storage #46116630890; NVRAM #46116630796 | PASS |
| `58700dc70b54d931aabdb6701f0ec88c14b9d1b4` | XY-C1 #46116630792 | PASS — Build, TRX and artifact upload; artifact ID `15252233106` (12,838,305 bytes) |
| `a747994f8892363f94938655b6db68195d44c9dd` | AI docs #46117399122 | PASS |

- L3 is now VERIFIED for the product source at `58700dc70b54d931aabdb6701f0ec88c14b9d1b4`: exact XY-C1 #46116630792 passed Build, TRX verification and artifact upload. Compare 58700 → current main `a747...` shows no production firmware paths changed, so the same L3 claim applies to current main's product source. L4 physical-router behavior remains NOT VERIFIED.
- L4 physical-router flash/boot, wireless/network and WAN behavior are NOT VERIFIED.
- Do not conflate host ASan/UBSan with MIPS build or hardware validation.

## Exact first action for the next GPT

1. Check the exact-main push run for the new `iappd-event-log-xy-c1-target-compile` job. It downloads the published `mipsel-linux-musl` compiler with a pinned SHA-256 and syntax-compiles both IAPP translation units using `IAPP_EVENT_LOG` + `IAPP_OS_LINUX` and XY-C1 MIPS32r2/1004kc flags. The job is newly wired; its exact-head result is still PENDING at this checkpoint. Diagnose a failure from the job log before closing this task.
2. This targeted MIPS compile is not a full firmware image/L3 result; default production configuration remains unchanged and does not enable `IAPP_EVENT_LOG`. The last full XY-C1 Build/TRX/artifact PASS is still run #46116630792 for product SHA `58700dc70b54d931aabdb6701f0ec88c14b9d1b4`; no firmware production source has changed since.
3. Once the targeted compile passes, resume the next highest-priority unresolved stability/security slice from the roadmap. Keep unrelated independent work moving while Actions are pending.
4. Avoid repeating completed watchdog, Process inventory, NVRAM boundary, detector, MiniUPnP, HTTPD, IPTV port and base IAPP safety audits.

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
